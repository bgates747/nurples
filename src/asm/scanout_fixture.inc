; Included only in the diagnostic game. MOS held-key snapshot is in IX.
; Bits set in scanout_off suppress future submissions, never repaint history.
scanout_off: db 0
scanout_previous: db 7
scanout_sample:
    ld a,(ix+2) ; top-row 3 = byte 2 bit 1
    and 2
    add a,a
    ld b,a
    ld a,(ix+6) ; top-row 1 and 2 = byte 6 bits 0 and 1
    and 3
    or b
    ld b,a
    ld a,(scanout_previous)
    cpl
    and b ; rising edges only
    ld hl,scanout_off
    xor (hl)
    ld (hl),a
    ld a,b
    ld (scanout_previous),a
    ret
