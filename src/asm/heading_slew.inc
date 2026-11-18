; Wrapped, rate-limited angle256 heading step.
;
; Angles use the game's unsigned 16.8 representation: the low sixteen bits are
; one complete turn, so arithmetic wraps naturally at 0/256 degrees256. The
; upper ADL byte is explicitly discarded at entry and after wrapped arithmetic.
;
; inputs:  HL = current heading
;          DE = desired heading
;          BC = nonnegative maximum step for this update
; outputs: HL = desired heading when it is within the step, otherwise current
;               advanced by exactly BC along the shortest wrapped path
;          carry set when the desired heading was reached; carry clear when
;               motion remains (including a zero maximum step)
; tie:     an exact half-turn advances in the positive/increasing direction
; destroys: AF, BC, DE; preserves IX, IY

heading_slew_168:
; Normalize every nominal u16 value. The dec/inc.s idiom restores the original
; low word while forcing the accidentally supplied ADL upper byte to zero.
    dec hl
    inc.s hl
    dec de
    inc.s de
    dec bc
    inc.s bc
    ld (heading_slew_current),hl
    ld (heading_slew_desired),de
    ld (heading_slew_step),bc

; Unsigned modulo-u16 delta = desired-current. Normalize the 24-bit subtraction
; back to its low word so a negative ADL result becomes the intended wrap.
    push de
    pop hl
    ld de,(heading_slew_current)
    or a
    sbc hl,de
    dec hl
    inc.s hl
    ld a,h
    or l
    jp z,@reached

; Delta <= 0x8000 uses increasing motion. The exact half-turn is the frozen
; deterministic tie-break; larger deltas use decreasing motion.
    push hl
    ld de,08000h
    or a
    sbc hl,de
    pop hl
    jp c,@increasing
    jp z,@increasing

; Convert the negative-direction delta to its positive magnitude.
    push hl
    pop de
    ld hl,0
    or a
    sbc hl,de
    dec hl
    inc.s hl
    call heading_slew_within_step
    jp c,@reached
    ld hl,(heading_slew_current)
    ld de,(heading_slew_step)
    or a
    sbc hl,de
    dec hl
    inc.s hl
    or a
    ret

@increasing:
    call heading_slew_within_step
    jp c,@reached
    ld hl,(heading_slew_current)
    ld de,(heading_slew_step)
    add hl,de
    dec hl
    inc.s hl
    or a
    ret

@reached:
    ld hl,(heading_slew_desired)
    scf
    ret
; end heading_slew_168

; inputs: HL = positive shortest-path magnitude
; outputs: carry set when magnitude <= configured maximum step
; preserves: HL
heading_slew_within_step:
    push hl
    ld de,(heading_slew_step)
    or a
    sbc hl,de
    jp c,@yes
    jp z,@yes
    pop hl
    or a
    ret
@yes:
    pop hl
    scf
    ret
; end heading_slew_within_step

heading_slew_current: dl 0
heading_slew_desired: dl 0
heading_slew_step: dl 0
