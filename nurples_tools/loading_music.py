"""Wire the chosen loading clock into a private assembler staging tree."""


def edit(src, filename, old, new):
    file = src / filename
    data = file.read_text()
    if data.count(old) != 1:
        raise ValueError(f"{filename}: fixture anchor changed: {old!r}")
    file.write_text(data.replace(old, new))


def configure(src, candidate, small_reads=False, whole=False, *, all_phases=False,
              combat_cues=False):
    if combat_cues and not all_phases:
        raise ValueError("The shield pre-alert uses the whole-game 50 Hz clock")
    if all_phases and not (candidate and small_reads and whole):
        raise ValueError("The campaign clock requires the qualified whole-append loader")
    if all_phases:
        edit(src, "mus_config.inc", "mus_all50: equ 0", "mus_all50: equ 1")
        # Shared graphics includes also assemble in standalone, music-free fixtures.
        # Hook only between complete commands, never in the MOS/VDU send primitive.
        wait = "    djnz @wait\n    ld a,(vdu_timing_fault_flags)"
        edit(src, "vdu.inc", wait, "    call lp_poll\n" + wait)
        edit(src, "state_game_playing.inc", "@tiles_ready:\n", "@tiles_ready:\n    call lp_poll\n")
        edit(src, "state_game_playing.inc", "    call move_tiles\n",
             "    call move_tiles\n    call lp_poll\n")
    if combat_cues:
        edit(src, "mus_config.inc", "mus_combat_cues: equ 0", "mus_combat_cues: equ 1")
    with (src / "mus_payload.inc").open("a") as f:
        f.write(f'lp_music: equ {int(candidate)}\nlp_whole: equ {int(whole)}\n'
                '    include "prt-loader.inc"\n')
    if small_reads:
        edit(src, "agnb_api.inc", "agnb_io_buffer_size:       equ 8192",
             "agnb_io_buffer_size:       equ 1024")
        filename, magic = (("MUSIC50.BIN", "LPR5") if all_phases else
                           ("LOAD1KW.BIN", "LPR4") if whole else ("LOAD1K.BIN", "LPR3"))
        edit(src, "prt-loader.inc", '"LOADPRT.BIN"', f'"{filename}"')
        edit(src, "prt-loader.inc", '"LPR1"', f'"{magic}"')
    init = "state_game_init.inc"
    timer_check = ("    ld a,(lp_owned)\n    or a\n    jr nz,@clock_ready\n"
                   "    inc a\n    ld (game_exit_requested),a\n    ret\n@clock_ready:\n"
                   if all_phases else "")
    edit(src, init, "    call load_ui_images",
         f"    call lp_begin\n{timer_check}    call load_ui_images")
    edit(src, init, "    call game_start_config_begin",
         "    call lp_finish\n    call game_start_config_begin")
    edit(src, "nurples.asm", "    call playfield_display_report",
         "    call playfield_display_report\n    call lp_save")
    if candidate:
        edit(src, "agnb_api.inc", "    call vdu_load_buffer", "    call lp_append")
        edit(src, "audio/agonAMP/libsupport.asm", "\trst.lil\t18h",
             "\tcall lp_flush ; Nurples experiment: RAM capture during preparation only")
        if not all_phases:
            edit(src, "campaign_audio.inc", "    call pt3_play\n    call ay_update",
                 "    call lp_frame")
        edit(src, init, "@agnb_load_error:\n", "@agnb_load_error:\n    push af\n"
             "    call lp_stop_timer\n    call mus_stop\n    pop af\n")
        edit(src, "nurples.asm", "main_end:\n", "main_end:\n    call lp_stop_timer\n")
