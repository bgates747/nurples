; Deterministic pure-CPU qualification for the shared release/press gate.

    assume adl=1
    org 040000h
    jp fixture_start

    include "../../src/asm/input_gate.inc"

FIXTURE_PASS: equ 02ah

fixture_gate: db 0a5h
fixture_case: db 0

; inputs: B = expected state, C = expected carry (0/1)
fixture_check:
    ld a,0
    adc a,0
    ld d,a
    ld a,(fixture_gate)
    cp b
    jp nz,fixture_fail
    ld a,d
    cp c
    jp nz,fixture_fail
    ret

; inputs: A = any-key-down sample, B = expected state, C = expected carry
fixture_update_check:
    push bc
    ld iy,fixture_gate
    call input_gate_update
    pop bc
    jp fixture_check

fixture_start:
    ld sp,05ff00h

; Reset always enters WAIT_RELEASE and clears carry.
    ld a,1
    ld (fixture_case),a
    ld iy,fixture_gate
    call input_gate_reset
    ld b,INPUT_GATE_WAIT_RELEASE
    ld c,0
    call fixture_check

; A key already held at state entry never fires and never arms.
    ld a,2
    ld (fixture_case),a
    ld a,1
    ld b,INPUT_GATE_WAIT_RELEASE
    ld c,0
    call fixture_update_check
    ld a,0ffh
    ld b,INPUT_GATE_WAIT_RELEASE
    ld c,0
    call fixture_update_check

; The first all-up sample arms the gate; repeated all-up samples are inert.
    ld a,3
    ld (fixture_case),a
    xor a
    ld b,INPUT_GATE_ARMED
    ld c,0
    call fixture_update_check
    xor a
    ld b,INPUT_GATE_ARMED
    ld c,0
    call fixture_update_check

; The next press fires once and consumes the arm immediately.
    ld a,4
    ld (fixture_case),a
    ld a,1
    ld b,INPUT_GATE_WAIT_RELEASE
    ld c,1
    call fixture_update_check
    ld a,1
    ld b,INPUT_GATE_WAIT_RELEASE
    ld c,0
    call fixture_update_check

; A second release/press cycle produces exactly one second event.
    ld a,5
    ld (fixture_case),a
    xor a
    ld b,INPUT_GATE_ARMED
    ld c,0
    call fixture_update_check
    ld a,080h
    ld b,INPUT_GATE_WAIT_RELEASE
    ld c,1
    call fixture_update_check
    ld a,080h
    ld b,INPUT_GATE_WAIT_RELEASE
    ld c,0
    call fixture_update_check

; Resetting an armed gate revokes its pending acceptance.
    ld a,6
    ld (fixture_case),a
    xor a
    ld b,INPUT_GATE_ARMED
    ld c,0
    call fixture_update_check
    ld iy,fixture_gate
    call input_gate_reset
    ld b,INPUT_GATE_WAIT_RELEASE
    ld c,0
    call fixture_check
    ld a,1
    ld b,INPUT_GATE_WAIT_RELEASE
    ld c,0
    call fixture_update_check

    ld a,FIXTURE_PASS
    out (0),a

fixture_fail:
    ld a,(fixture_case)
    or a
    jp nz,@emit
    ld a,0feh
@emit:
    out (0),a
