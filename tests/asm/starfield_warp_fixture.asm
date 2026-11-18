; Deterministic pure-CPU/VDU-call qualification for starfield warp chunking.

    assume adl=1
    org 040000h
    jp fixture_start

origin_left: equ 128
origin_top: equ 48
BUF_BG_STARS_0: equ 0400h

bg_y_plot: dl -1
bg_bufferId: dl BUF_BG_STARS_0
fixture_case: db 0
fixture_scroll_count: db 0
fixture_scroll_chunks: blkb 4,0
fixture_plot_count: db 0
fixture_random: db 2

    include "../../src/asm/starfield_warp.inc"

FIXTURE_PASS: equ 02ah

fixture_start:
    ld sp,05ff00h

; First step from phase 255 selects a new tile and advances to phase zero.
    call fixture_reset_counts
    ld a,1
    ld (fixture_case),a
    ld hl,-1
    ld (bg_y_plot),hl
    ld a,1
    call starfield_warp_scroll
    jp nc,fixture_fail
    ld a,(bg_y_plot)
    or a
    jp nz,fixture_fail
    ld a,(fixture_scroll_count)
    cp 1
    jp nz,fixture_fail
    ld a,(fixture_scroll_chunks)
    cp 1
    jp nz,fixture_fail
    ld hl,(bg_bufferId)
    ld de,BUF_BG_STARS_0+2
    or a
    sbc hl,de
    jp nz,fixture_fail

; Crossing at phase 250 splits into five old-tile and three new-tile rows.
    call fixture_reset_counts
    ld a,2
    ld (fixture_case),a
    ld hl,-6 ; low byte 250, upper bytes retain the negative tile placement
    ld (bg_y_plot),hl
    ld a,8
    call starfield_warp_scroll
    jp nc,fixture_fail
    ld a,(bg_y_plot)
    cp 2
    jp nz,fixture_fail
    ld a,(fixture_scroll_count)
    cp 2
    jp nz,fixture_fail
    ld a,(fixture_scroll_chunks+0)
    cp 5
    jp nz,fixture_fail
    ld a,(fixture_scroll_chunks+1)
    cp 3
    jp nz,fixture_fail
    ld a,(fixture_plot_count)
    cp 2
    jp nz,fixture_fail

; A non-crossing maximum step emits one exact-width chunk.
    call fixture_reset_counts
    ld a,3
    ld (fixture_case),a
    ld hl,-256
    ld (bg_y_plot),hl ; phase zero
    ld a,16
    call starfield_warp_scroll
    jp nc,fixture_fail
    ld a,(bg_y_plot)
    cp 16
    jp nz,fixture_fail
    ld a,(fixture_scroll_count)
    cp 1
    jp nz,fixture_fail
    ld a,(fixture_scroll_chunks)
    cp 16
    jp nz,fixture_fail

; Zero is a successful no-op; out-of-range input fails before VDU output.
    call fixture_reset_counts
    ld a,4
    ld (fixture_case),a
    xor a
    call starfield_warp_scroll
    jp nc,fixture_fail
    ld a,(fixture_scroll_count)
    or a
    jp nz,fixture_fail
    ld a,17
    call starfield_warp_scroll
    jp c,fixture_fail
    ld a,(fixture_scroll_count)
    or a
    jp nz,fixture_fail

    ld a,FIXTURE_PASS
    out (0),a

fixture_fail:
    ld a,(fixture_case)
    or a
    jp nz,@emit
    ld a,0feh
@emit:
    out (0),a

fixture_reset_counts:
    xor a
    ld (fixture_scroll_count),a
    ld (fixture_plot_count),a
    ld (fixture_scroll_chunks+0),a
    ld (fixture_scroll_chunks+1),a
    ld (fixture_scroll_chunks+2),a
    ld (fixture_scroll_chunks+3),a
    ret

rand_8:
    ld a,(fixture_random)
    ret

vdu_set_gfx_origin:
    ret

set_gfx_playing_field:
    ret

vdu_scroll_down:
    ld a,(fixture_scroll_count)
    ld e,a
    ld d,0
    ld iy,fixture_scroll_chunks
    add iy,de
    ld (iy),h
    inc a
    ld (fixture_scroll_count),a
    ret

vdu_set_gfx_viewport:
    ret

vdu_buff_select:
    ret

vdu_plot_bmp:
    ld hl,fixture_plot_count
    inc (hl)
    ret
