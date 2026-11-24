"""Instrument the private assembler staging tree, never the ordinary game."""


def instrument(src):
    def edit(name, old, new):
        path = src / name
        text = path.read_text()
        if text.count(old) != 1:
            raise ValueError(f"missing or ambiguous scanout hook: {name}: {old}")
        path.write_text(text.replace(old, new))

    # Zero only incoming damage. Contact audio, projectile destruction and
    # ground/air collision responses still go through their ordinary paths.
    edit("player_shields.inc", "    ld l,(iy+sprite_shield_damage)", "    ld l,0")
    # Replace only the setup-time actor marking call pair. The ordinary game
    # still marks every actor as hardware; frame submission is identical.
    edit("sprites.inc", "    call vdu_sprite_select\n    call vdu_sprite_mark_hardware",
         "    call scanout_mark_actor")
    edit("nurples.asm", "main:\n", '    include "scanout_fixture.inc"\nmain:\n'
         "    xor a\n    ld (scanout_off),a\n    ld a,7\n    ld (scanout_previous),a\n")
    edit("game_pause.inc", "    call lp_sample_m", "    call scanout_sample\n    call lp_sample_m")
    # Ignore diagnostic keys at title/Game Over; other keys in those same
    # bitmap bytes must still work. C holds the Y/N result and stays intact.
    edit("presentation_input.inc", """    ld b,music_key_byte
@low:
    or (hl)
    inc hl
    djnz @low
    inc hl ; skip byte 12, already accumulated with M removed
    ld b,15-music_key_byte""", """    ld b,a
    ld a,(ix+2)
    and 0fdh
    or b
    ld b,a
    ld a,(ix+6)
    and 0fch
    or b
""" + "".join(f"    or (ix+{i})\n" for i in range(13) if i not in (2, 6, 12))
         + "    lea hl,ix+13\n    ld b,3")

    # Skip the output loop only. Row admission/activation below it reloads
    # the source row pointer independently of the visual cache.
    edit("tiles.inc", "tiles_plot_visual_ready:\n", "tiles_plot_visual_ready:\n"
         "    ld a,(scanout_off)\n    bit 1,a\n    jp nz,scanout_row_done\n")
    edit("tiles.inc", "    djnz tiles_plot_loop", "    djnz tiles_plot_loop\nscanout_row_done:")

    # Each background renderer has already advanced its phase and selected
    # its next strip before this point. Skip both buffer selection and plot.
    guard = "    ld a,(scanout_off)\n    bit 0,a\n    ret nz\n"
    for name, anchor in (("tiles.inc", "@@:\n    call vdu_buff_select\n    ld bc,0"),
                         ("earth_opening.inc", "@plot:\n    ld hl,(bg_bufferId)"),
                         ("starfield_warp.inc", "    call vdu_set_gfx_viewport\n    ld hl,(bg_bufferId)"),
                         ("levels.inc", "; draw initial background tiles \n    ld hl,BUF_BG_STARS_0")):
        first, rest = anchor.split("\n", 1)
        edit(name, anchor, first + "\n" + guard + rest)

    # A carries the original viewport extent. The caller has already updated
    # the logical travel distance; don't suppress actors or projectile motion.
    edit("vdu.inc", "vdu_scroll_down:\n", "vdu_scroll_down:\n"
         "    push af\n    ld a,(scanout_off)\n    bit 2,a\n    jr z,@emit\n"
         "    pop af\n    ret\n@emit:\n    pop af\n")
