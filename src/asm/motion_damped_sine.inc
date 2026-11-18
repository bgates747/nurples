; Reusable horizontal damped-sine position primitive.
;
; This owns no actor fields and performs no movement or drawing. Actor schemas
; retain their own phase/amplitude state, so the same primitive can drive a
; player presentation, an enemy flight pattern, or another scripted actor.
;
; inputs:
;   A  = angle256 integer phase
;   BC = positive 16.8 amplitude in pixels
;   DE = signed 16.8 centre position
; output:
;   HL = centre + sin(phase) * amplitude, signed 16.8
; preserves: IX, IY, SP
; destroys: AF, BC, DE, HL
motion_damped_sine_position:
    push de
    push bc
    ld hl,0
    ld h,a
    call sin168
    pop de
    call smul168
    pop de
    add hl,de
    ret
; end motion_damped_sine_position
