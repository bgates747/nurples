; Pure-CPU regression: execute the real input/patch routines against emulated
; GPIO levels and a fixture keymap. MOS/VDP calls below are observation stubs.
; This is not a MOS application and must never be deployed to physical hardware.
    assume adl=1
    org 040000h
    jp fixture_start

    macro MOSCALL function
        ld ix,fixture_keymap
    endmacro
mos_getkbmap: equ 01eh
player_speed: equ 3*256
sprite_id: equ 0
sprite_x: equ 1
sprite_y: equ 4
sprite_xvel: equ 7
sprite_yvel: equ 10
    include "../../src/asm/player_input.inc"

    macro CHECK_INPUT expected_x,expected_y,expected_frame,expected_fire
        xor a
        ld (fixture_fired),a
        call player_input
        ld hl,(player_begin+sprite_xvel)
        ld de,expected_x
        or a
        sbc hl,de
        jp nz,fixture_fail
        ld hl,(player_begin+sprite_yvel)
        ld de,expected_y
        or a
        sbc hl,de
        jp nz,fixture_fail
        ld a,(fixture_frame)
        cp expected_frame
        jp nz,fixture_fail
        ld a,(fixture_fired)
        cp expected_fire
        jp nz,fixture_fail
        ld hl,fixture_case
        inc (hl)
    endmacro

fixture_start:
    ld sp,07fff0h
; Output mode is only a headless stimulus: it supplies controlled pin levels.
    xor a
    out0 (09fh),a
    out0 (0a3h),a
    ld a,0ffh
    out0 (0a2h),a
; Default NOPs retain joystick behavior: PC3 low cancels keyboard Up.
    ld a,0f7h
    out0 (09eh),a
    ld a,2
    ld (fixture_keymap+7),a
    CHECK_INPUT 0,0,1,0
; The same pin/key combination must move Up with joystick disabled.
    call player_joystick_disable
    CHECK_INPUT 0,-player_speed,1,0
; Even all-low GPIO must produce neither idle movement nor phantom fire.
    xor a
    ld (fixture_keymap+7),a
    out0 (09eh),a
    out0 (0a2h),a
    CHECK_INPUT 0,0,1,0
; Keyboard diagonals preserve A's left/right sprite-frame selection.
    ld a,2
    ld (fixture_keymap+3),a
    ld (fixture_keymap+7),a
    CHECK_INPUT -player_speed,-player_speed,0,0
    xor a
    ld (fixture_keymap+3),a
    ld (fixture_keymap+7),a
    ld a,2
    ld (fixture_keymap+15),a
    ld (fixture_keymap+5),a
    CHECK_INPUT player_speed,player_speed,2,0
    xor a
    ld (fixture_keymap+15),a
    ld (fixture_keymap+5),a
; Keyboard fire remains available while the GPIO fire poll is skipped.
    ld a,4
    ld (fixture_keymap+12),a
    CHECK_INPUT 0,0,1,1
    xor a
    ld (fixture_keymap+12),a
; A later Y restores both NOP pairs, including their displacement bytes.
    call player_joystick_enable
    ld a,0ffh
    out0 (09eh),a
    out0 (0a2h),a
    CHECK_INPUT 0,0,1,0
    ld a,0fdh
    out0 (09eh),a
    ld a,0dfh
    out0 (0a2h),a
    CHECK_INPUT 0,-player_speed,1,1
; A second N must once again suppress both live joystick inputs.
    call player_joystick_disable
    CHECK_INPUT 0,0,1,0
    ld a,02ah
    out (0),a
    halt
fixture_fail:
    ld a,(fixture_case)
    out (0),a
    halt

vdu_sprite_select_frame:
    ld (fixture_frame),a
vdu_sprite_select:
vdu_sprite_move_abs168:
move_sprite:
    ret
player_spawn_laser:
    ld a,1
    ld (fixture_fired),a
    ret
table_pointer: dl player_begin
player_begin: blkb 13,0
fixture_keymap: blkb 16,0
fixture_frame: db 0
fixture_fired: db 0
fixture_case: db 1
