pose_before_behavior:
    ; HL is the validated move program used by the following CALL_HL.
    push hl
    ld de,move_earth_air
    or a
    sbc hl,de
    pop hl
    ld a,0
    jr nz,@ordinary
    inc a
    ld (pose_eligible),a
    ret
@ordinary:
    ld (pose_eligible),a
    jp pose_barrier
