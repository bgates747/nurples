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

; A = slot ID from sprites_mark_actor_prefix, before activation.
scanout_mark_actor:
    push af
    call vdu_sprite_select
    pop af
    cp [air_base-table_base]/table_record_size
    jp c,vdu_sprite_mark_hardware
    cp air_end_sprite_id
    jp c,vdu_sprite_mark_software
    jp vdu_sprite_mark_hardware
