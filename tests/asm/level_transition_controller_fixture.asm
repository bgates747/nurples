; Deterministic pure-CPU qualification for the level-transition schedule.

    assume adl=1
    org 040000h
    jp fixture_start

field_bottom: equ 335

fixture_case: db 0
fixture_frames: dl 0
fixture_terrain_frames: dl 0
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
    ld a,(level_transition_frame_policy)
    and LEVEL_TRANSITION_POLICY_TERRAIN
    jp z,@count_speed
    ld hl,(fixture_terrain_frames)
    inc hl
    ld (fixture_terrain_frames),hl
@count_speed:
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
    ld hl,(fixture_terrain_frames)
    ld de,level_transition_drain_frames
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

; Expected frame histogram: 1=442; 2/3/4/6/8/12=16 each; 16=98.
    ld a,2
    ld (fixture_case),a
    ld hl,(fixture_speed_counts+[1*3])
    ld de,442
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

fixture_clear_counts:
    xor a
    ld (fixture_frames),a
    ld (fixture_frames+1),a
    ld (fixture_frames+2),a
    ld (fixture_terrain_frames),a
    ld (fixture_terrain_frames+1),a
    ld (fixture_terrain_frames+2),a
    ld hl,fixture_speed_counts
    ld b,17*3
@loop:
    ld (hl),a
    inc hl
    djnz @loop
    ret
