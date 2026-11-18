; Bounded TITLE/ATTRACT lifecycle.
;
; Entry assumes resident assets, mode-20 playfield art and the ordinary 20-slot
; software-sprite prefix already exist. Each update advances only the blank
; starfield by one scanline, then updates fixed hardware overlay sprites. The
; outer scheduler remains the sole VBlank owner.

attract_lifecycle_active: db 0
attract_dismiss_requested: db 0

attract_lifecycle_process_init:
    xor a
    ld (attract_lifecycle_active),a
    ld (attract_dismiss_requested),a
    ret
; end attract_lifecycle_process_init

; outputs: carry set on active entry; carry clear on renderer/setup failure
attract_lifecycle_enter:
    ld a,(attract_lifecycle_active)
    or a
    jp z,@acquire
    scf
    ret
@acquire:
    ld a,PRESENTATION_ATTRACT
    call presentation_overlay_enter
    ret nc
    call presentation_renderer_enter
    jp c,@active
    call presentation_overlay_exit
    xor a
    ret
@active:
    xor a
    ld (attract_dismiss_requested),a
    inc a
    ld (attract_lifecycle_active),a
    scf
    ret
; end attract_lifecycle_enter

; Advance exactly one outer frame.
; outputs: carry set when a release-gated fresh key requests start flow
attract_lifecycle_update:
    ld a,(attract_lifecycle_active)
    or a
    jp nz,@frame
    xor a
    ret
@frame:
; Speed one is the ordinary background pace, but this blank-starfield path has
; no terrain-row admission and cannot mutate any actor or active-tile record.
    ld a,1
    call starfield_warp_scroll
    call presentation_input_poll
    call presentation_overlay_update
    ld a,0
    adc a,0
    ld (attract_dismiss_requested),a
    call presentation_renderer_update
    ld a,(attract_dismiss_requested)
    or a
    jp z,@continue
    scf
    ret
@continue:
    xor a
    ret
; end attract_lifecycle_update

attract_lifecycle_exit:
    ld a,(attract_lifecycle_active)
    or a
    ret z
    call presentation_renderer_exit
    call presentation_overlay_exit
    xor a
    ld (attract_lifecycle_active),a
    ld (attract_dismiss_requested),a
    scf
    ret
; end attract_lifecycle_exit
