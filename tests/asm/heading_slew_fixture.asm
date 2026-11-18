; Deterministic pure-CPU qualification for wrapped rate-limited headings.

    assume adl=1
    org 040000h
    jp fixture_start

    include "../../src/asm/heading_slew.inc"

FIXTURE_PASS: equ 02ah

    macro CHECK_SLEW case_id,current_value,desired_value,step_value,expected_value,expected_carry
        ld a,case_id
        ld (fixture_case),a
        ld hl,current_value
        ld de,desired_value
        ld bc,step_value
        call heading_slew_168
        ld a,0
        adc a,0
        cp expected_carry
        jp nz,fixture_fail
        ld de,expected_value
        or a
        sbc hl,de
        jp nz,fixture_fail
    endmacro

fixture_start:
    ld sp,05ff00h

; Equal, sub-step and exact-step destinations snap and report reached.
    CHECK_SLEW 1,01000h,01000h,00400h,01000h,1
    CHECK_SLEW 2,01000h,01200h,00400h,01200h,1
    CHECK_SLEW 3,01200h,01000h,00400h,01000h,1
    CHECK_SLEW 4,01000h,01400h,00400h,01400h,1

; Longer paths advance exactly one step in each direction.
    CHECK_SLEW 5,01000h,02000h,00400h,01400h,0
    CHECK_SLEW 6,02000h,01000h,00400h,01c00h,0

; Both directions choose the short route across the 0/256 wrap.
    CHECK_SLEW 7,0fe00h,00200h,00100h,0ff00h,0
    CHECK_SLEW 8,00200h,0fe00h,00100h,00100h,0
    CHECK_SLEW 9,0ff80h,00080h,00100h,00080h,1

; Fractional steps retain all eight fractional bits.
    CHECK_SLEW 10,0ff80h,00080h,00040h,0ffc0h,0
    CHECK_SLEW 11,00080h,0ff80h,00040h,00040h,0

; The half-turn tie advances in the documented increasing direction.
    CHECK_SLEW 12,00000h,08000h,00100h,00100h,0
    CHECK_SLEW 13,08000h,00000h,00100h,08100h,0

; A zero step is stable but not falsely reported as reached.
    CHECK_SLEW 14,01000h,02000h,00000h,01000h,0

; Poisoned ADL upper bytes are discarded for all three inputs.
    CHECK_SLEW 15,0aa1000h,0552000h,0cc0400h,01400h,0

    ld a,FIXTURE_PASS
    out (0),a

fixture_fail:
    ld a,(fixture_case)
    or a
    jp nz,@emit
    ld a,0feh
@emit:
    out (0),a

fixture_case: db 0
