"""Wire the chosen loading clock into a private assembler staging tree."""


def edit(src, filename, old, new):
    file = src / filename
    data = file.read_text()
    if data.count(old) != 1:
        raise ValueError(f"{filename}: fixture anchor changed: {old!r}")
    file.write_text(data.replace(old, new))


def configure(src, candidate, small_reads=False, whole=False):
    with (src / "mus_payload.inc").open("a") as f:
        f.write(f'lp_music: equ {int(candidate)}\nlp_whole: equ {int(whole)}\n'
                '    include "prt-loader.inc"\n')
    if small_reads:
        edit(src, "agnb_api.inc", "agnb_io_buffer_size:       equ 8192",
             "agnb_io_buffer_size:       equ 1024")
        edit(src, "prt-loader.inc", '"LOADPRT.BIN"', '"LOAD1KW.BIN"' if whole else '"LOAD1K.BIN"')
        edit(src, "prt-loader.inc", '"LPR1"', '"LPR4"' if whole else '"LPR3"')
    init = "state_game_init.inc"
    edit(src, init, "    call load_ui_images", "    call lp_begin\n    call load_ui_images")
    edit(src, init, "    call game_start_config_begin",
         "    call lp_finish\n    call game_start_config_begin")
    edit(src, "nurples.asm", "    call playfield_display_report",
         "    call playfield_display_report\n    call lp_save")
    if candidate:
        edit(src, "agnb_api.inc", "    call vdu_load_buffer", "    call lp_append")
        edit(src, "audio/agonAMP/libsupport.asm", "\trst.lil\t18h",
             "\tcall lp_flush ; Nurples experiment: RAM capture during preparation only")
        edit(src, "campaign_audio.inc", "mus_tick:\n    call pt3_play\n    call ay_update",
             "mus_tick:\n    call lp_frame")
        edit(src, init, "@agnb_load_error:\n", "@agnb_load_error:\n    push af\n"
             "    call lp_stop_timer\n    call mus_stop\n    pop af\n")
        edit(src, "nurples.asm", "main_end:\n", "main_end:\n    call lp_stop_timer\n")
