; Verify the actual terrain path against a one-row, one-column level.
    assume adl=1
    org 040000h
    jp fixture_start

    include "../../src/asm/macros.inc"

origin_left: equ 128
origin_top: equ 48
field_bottom: equ 335
BUF_BG_STARS_0: equ 1024
tile_activation_routines_count: equ 1
tile_activation_routines: dl fixture_fail
fixture_last_row: db 0
fixture_scrolls: dl 0
fixture_tile_plots: db 0
fixture_allow_travel: db 1

    include "../../src/asm/tiles.inc"

fixture_start:
    ld sp,05ff00h
    ld hl,0
    ld (tiles_y_plot),hl ; last row's final scanline at Y=0
    ld hl,512
    ld (cur_baseBufferId),hl
    ld hl,fixture_last_row
    ld (tiles_row_defs),hl
    ld a,1
    ld (tiles_cols),a
    ld (tiles_row),a
    call tiles_plot
    or a
    jp nz,fixture_fail
    ld hl,(tiles_drain_remaining)
    ld de,336
    or a
    sbc hl,de
    jp nz,fixture_fail

; A player phase that pauses scrolling must not spend the drain budget.
    xor a
    ld (fixture_allow_travel),a
    call tiles_plot
    ld hl,(fixture_scrolls)
    dec hl
    ld a,h
    or l
    jp nz,fixture_fail
    ld a,1
    ld (fixture_allow_travel),a

    ld bc,335
@drain:
    push bc
    call tiles_plot
    pop bc
    or a
    jp nz,fixture_fail
    dec bc
    ld a,b
    or c
    jp nz,@drain

; The last terrain pixel leaves on precisely the next scroll.
    call tiles_plot
    cp 1
    jp nz,fixture_fail
    ld hl,(fixture_scrolls)
    ld de,337 ; final admission plus 336 drain scrolls
    or a
    sbc hl,de
    jp nz,fixture_fail
    ld hl,(tiles_row_defs)
    ld de,fixture_last_row+1
    or a
    sbc hl,de
    jp nz,fixture_fail
    ld a,(fixture_tile_plots)
    cp 1
    jp nz,fixture_fail
    ld a,02ah
    out (0),a

fixture_fail:
    ld a,0feh
    out (0),a

player_world_travel_enabled:
    ld a,(fixture_allow_travel)
    or a
    ret z
    scf
    ret

vdu_scroll_down:
    ld hl,fixture_scrolls
    inc (hl)
    ret nz
    inc hl
    inc (hl)
    ret

vdu_buff_select:
    ld a,h
    cp 2 ; terrain buffer base is 512; starfield and its variants start at 1024
    ret nz
    ld hl,fixture_tile_plots
    inc (hl)
    ret

rand_8:
    xor a
    ret

vdu_plot_bmp:
vdu_set_gfx_origin:
set_gfx_playing_field:
vdu_set_gfx_viewport:
    ret
