; Deterministic pure-CPU qualification for the level-transition schedule.

    assume adl=1
    org 040000h
    jp fixture_start

fixture_case: db 0
fixture_frames: dl 0
fixture_speed_counts: blkb 17*3,0

    include "../../src/asm/level_transition_controller.inc"

FIXTURE_PASS: equ 02ah

fixture_start:
    ld sp,05ff00h
    call fixture_clear_counts
    call level_transition_init

@frame:
    call level_transition_step
    jp c,@complete
    ld b,a
    or a
    jp z,fixture_fail
    cp 17
    jp nc,fixture_fail
    ld hl,(fixture_frames)
    inc hl
    ld (fixture_frames),hl
    ld hl,0
    ld l,b
    ld h,3
    mlt hl
    ld de,fixture_speed_counts
    add hl,de
    ld de,(hl)
    inc de
    ld (hl),de
    jp @frame

@complete:
    ld a,1
    ld (fixture_case),a
    ld hl,(fixture_frames)
    ld de,level_transition_total_frames
    or a
    sbc hl,de
    jp nz,fixture_fail
    ld a,(level_transition_phase)
    cp LEVEL_TRANSITION_COMPLETE
    jp nz,fixture_fail
    ld a,(level_transition_speed)
    cp 1
    jp nz,fixture_fail
    ld a,(level_transition_error)
    or a
    jp nz,fixture_fail

; Expected frame histogram: 1=106; 2/3/4/6/8/12=16 each; 16=98.
    ld a,2
    ld (fixture_case),a
    ld hl,(fixture_speed_counts+[1*3])
    ld de,106
    or a
    sbc hl,de
    jp nz,fixture_fail
    ld b,6
    ld hl,fixture_expected_mid_speeds
@mid_loop:
    ld a,(hl)
    inc hl
    push hl
    ld l,a
    ld h,3
    mlt hl
    ld de,fixture_speed_counts
    add hl,de
    ld de,(hl)
    ld hl,16
    or a
    sbc hl,de
    pop hl
    jp nz,fixture_fail
    djnz @mid_loop
    ld hl,(fixture_speed_counts+[16*3])
    ld de,98
    or a
    sbc hl,de
    jp nz,fixture_fail

; A corrupt phase fails closed on the first step.
    ld a,3
    ld (fixture_case),a
    ld a,99
    ld (level_transition_phase),a
    call level_transition_step
    jp nc,fixture_fail
    ld a,(level_transition_error)
    cp 1
    jp nz,fixture_fail

; Opening reuses the controller. Qualify every emitted speed in sequence,
; including the one-pixel frames on both sides of the fast hold.
    ld a,4
    ld (fixture_case),a
    call level_transition_init_entrance
    ld ix,fixture_expected_entrance
@entrance_run:
    ld c,(ix+0)
    ld b,(ix+1)
    ld a,b
    or a
    jp z,@entrance_end
@entrance_frame:
    call level_transition_step
    jp c,fixture_fail
    cp c
    jp nz,fixture_fail
    djnz @entrance_frame
    lea ix,ix+2
    jp @entrance_run
@entrance_end:
    call level_transition_step
    jp nc,fixture_fail
    ld a,(level_transition_error)
    or a
    jp nz,fixture_fail

; A later ordinary transition must restore the full timing, not inherit the
; short opening parameters or the completed phase.
    ld a,5
    ld (fixture_case),a
    call level_transition_init
    ld b,98
@restored_opening:
    call level_transition_step
    jp c,fixture_fail
    cp 1
    jp nz,fixture_fail
    djnz @restored_opening
    call level_transition_step
    jp c,fixture_fail
    cp 2
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

fixture_expected_mid_speeds: db 2,3,4,6,8,12
fixture_expected_entrance:
    db 1,16, 2,4, 3,4, 4,4, 6,4, 8,4, 12,4, 16,44
    db 12,4, 8,4, 6,4, 4,4, 3,4, 2,4, 1,4, 0,0

fixture_clear_counts:
    xor a
    ld (fixture_frames),a
    ld (fixture_frames+1),a
    ld (fixture_frames+2),a
    ld hl,fixture_speed_counts
    ld b,17*3
@loop:
    ld (hl),a
    inc hl
    djnz @loop
    ret
