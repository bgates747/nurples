; Deterministic native fixture for the bounded start-flow choice controller.

    assume adl=1
    org 040000h
    jp fixture_start

    include "../../src/asm/input_gate.inc"
    include "../../src/asm/start_choice_controller.inc"

FIXTURE_PASS: equ 02ah
fixture_case: db 0

fixture_start:
    ld sp,05ff00h
    call start_choice_enter

; A held attract key cannot select YES even if its normalized value happens to
; match. A release only arms the controller.
    ld a,1
    ld c,START_CHOICE_YES
    call start_choice_update
    jp c,fixture_fail
    xor a
    ld c,START_CHOICE_NONE
    call start_choice_update
    jp c,fixture_fail

; A fresh unrelated key is consumed but not accepted. Continued hold cannot
; turn into a choice; another full release is required.
    ld a,1
    ld c,START_CHOICE_NONE
    call start_choice_update
    jp c,fixture_fail
    ld a,1
    ld c,START_CHOICE_NO
    call start_choice_update
    jp c,fixture_fail
    xor a
    ld c,START_CHOICE_NONE
    call start_choice_update
    jp c,fixture_fail

; The next fresh NO is accepted exactly once and persisted for the caller.
    ld a,1
    ld c,START_CHOICE_NO
    call start_choice_update
    jp nc,fixture_fail
    cp START_CHOICE_NO
    jp nz,fixture_fail
    ld a,(start_choice_result)
    cp START_CHOICE_NO
    jp nz,fixture_fail
    ld a,1
    ld c,START_CHOICE_NO
    call start_choice_update
    jp c,fixture_fail

; Exit clears ownership. A new entry again requires release, then accepts YES.
    call start_choice_exit
    ld a,(start_choice_result)
    or a
    jp nz,fixture_fail
    call start_choice_enter
    xor a
    ld c,START_CHOICE_NONE
    call start_choice_update
    jp c,fixture_fail
    ld a,1
    ld c,START_CHOICE_YES
    call start_choice_update
    jp nc,fixture_fail
    cp START_CHOICE_YES
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
