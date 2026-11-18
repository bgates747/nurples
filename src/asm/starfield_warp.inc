; Multi-scanline starfield-only scroller for level starts and transitions.
;
; This module must never be called by tiles_plot or generalized to terrain.
; The global game renderer may select it only while no terrain rows are being
; admitted (initial player approach, drained level transition, or hyperwarp).
; It advances the existing 256x256 background stream by 0..16 scanlines in one
; outer frame, splitting at most once when a step crosses a tile seam. Each
; chunk scrolls only the playfield and redraws exactly the newly exposed strip.

starfield_warp_max_step: equ 16
starfield_warp_remaining: db 0
starfield_warp_chunk: db 0

; inputs: A = requested scanline displacement, 0..16
; outputs: carry set on success; carry clear with no VDU output if out of range
; destroys: everything except SP
starfield_warp_scroll:
    cp starfield_warp_max_step+1
    jp nc,@invalid
    ld (starfield_warp_remaining),a
    or a
    jp z,@done
@next_chunk:
    ld a,(bg_y_plot) ; low byte is the 0..255 position within current tile
    cp 255
    jp z,@begin_new_tile
; Capacity before the seam is 255-phase. A larger request renders that tail,
; then loops once at phase 255 to select and render the new tile's head.
    ld b,a
    ld a,255
    sub b
    ld b,a ; B = existing-tile capacity
    ld a,(starfield_warp_remaining)
    cp b
    jp c,@existing_final
    jp z,@existing_final
    ld a,b
    jp @existing_chunk_ready
@existing_final:
    ld a,(starfield_warp_remaining)
@existing_chunk_ready:
    ld (starfield_warp_chunk),a
    ld b,a
    ld a,(bg_y_plot)
    add a,b
    ld (bg_y_plot),a
    call starfield_warp_render_chunk
    jp @consume_chunk

@begin_new_tile:
    call starfield_warp_select_next_tile
    ld a,(starfield_warp_remaining)
    ld (starfield_warp_chunk),a
    dec a ; phase 255 + N steps wraps to N-1 in the new tile
    ld (bg_y_plot),a
    call starfield_warp_render_chunk

@consume_chunk:
    ld a,(starfield_warp_remaining)
    ld b,a
    ld a,(starfield_warp_chunk)
    sub b
    neg ; A = remaining - chunk without relying on preserved flags/registers
    ld (starfield_warp_remaining),a
    or a
    jp nz,@next_chunk
@done:
    call set_gfx_playing_field
    scf
    ret
@invalid:
    xor a
    ret
; end starfield_warp_scroll

; Select a new 256x256 starfield tile using the same four-buffer policy as the
; gameplay background stream. Called only when phase 255 crosses to zero.
starfield_warp_select_next_tile:
    ld hl,BUF_BG_STARS_0
    call rand_8
    and %00000011
    add a,l
    ld l,a
    ld a,0
    adc a,h
    ld h,a
    ld (bg_bufferId),hl
    ret
; end starfield_warp_select_next_tile

; Scroll and redraw one nonzero chunk. The full playfield viewport is restored
; before scrolling. VDP bitmap clipping needs one guard scanline beyond the
; nominal 0..chunk-1 exposed strip here. Without it, one source scanline from
; above the playfield survives each call, is scrolled down on the next call,
; and grows into a visible cockpit-art band during the entrance warp.
starfield_warp_render_chunk:
    ld bc,origin_left
    ld de,origin_top
    call vdu_set_gfx_origin
    call set_gfx_playing_field
    ld a,2 ; current graphics viewport
    ld l,2 ; down
    ld a,(starfield_warp_chunk)
    ld h,a
    ld a,2
    call vdu_scroll_down
    ld bc,0
    ld de,0
    ld ix,255
    ld iy,0
    ld a,(starfield_warp_chunk)
    ld iyl,a
    call vdu_set_gfx_viewport
    ld hl,(bg_bufferId)
    call vdu_buff_select
    ld bc,0
    ld de,(bg_y_plot)
    inc de
    call vdu_plot_bmp
    ret
; end starfield_warp_render_chunk
