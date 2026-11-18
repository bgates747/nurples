; Deterministic pure-CPU qualification for the automated victory flight.

    assume adl=1
    org 040000h
    jp fixture_start

actor_record_size: equ 64
actor_record_custom: equ 42
; Match the production player schema: victory state owns the final two bytes
; after the fourteen-byte death/entry/grace block.
player_custom_end: equ 56
sprite_x: equ 1
sprite_y: equ 4
sprite_xvel: equ 7
sprite_yvel: equ 10
sprite_vel: equ 13
sprite_heading: equ 16
sprite_collisions: equ 29
sprite_frame: equ 40
sprite_alive: equ 6

player_begin: blkb actor_record_size,0
table_pointer: dl player_begin
fixture_case: db 0
fixture_draws: dl 0

    include "../../src/asm/player_victory.inc"

FIXTURE_PASS: equ 02ah

    macro CHECK24 address,expected
        ld hl,(address)
        ld de,expected
        or a
        sbc hl,de
        jp nz,fixture_fail
    endmacro

fixture_start:
    ld sp,05ff00h
    ld iy,player_begin
    set sprite_alive,(iy+sprite_collisions)
    ld hl,20*256
    ld (iy+sprite_x),hl
    ld hl,300*256
    ld (iy+sprite_y),hl
    call player_victory_init
    bit sprite_alive,(iy+sprite_collisions)
    jp nz,fixture_fail

; First gather update moves through the stub; second recognizes exact arrival.
    ld a,1
    ld (fixture_case),a
    call player_victory_update
    jp c,fixture_fail
    call player_victory_update
    jp c,fixture_fail
    ld a,(player_begin+player_victory_phase)
    cp PLAYER_VICTORY_LOOP
    jp nz,fixture_fail

; Exactly 64 bounded loop updates enter CENTER and normalize heading to start.
    ld a,2
    ld (fixture_case),a
    ld b,player_victory_loop_frames
@loop:
    push bc
    call player_victory_update
    jp c,fixture_fail
    pop bc
    djnz @loop
    ld a,(player_begin+player_victory_phase)
    cp PLAYER_VICTORY_CENTER
    jp nz,fixture_fail
    CHECK24 player_begin+sprite_heading,player_victory_loop_heading

; Centre approach likewise moves once, then enters HOLD on exact arrival.
    ld a,3
    ld (fixture_case),a
    call player_victory_update
    jp c,fixture_fail
    call player_victory_update
    jp nc,fixture_fail
    ld a,(player_begin+player_victory_phase)
    cp PLAYER_VICTORY_HOLD
    jp nz,fixture_fail
    CHECK24 player_begin+sprite_x,player_victory_center_x*256
    CHECK24 player_begin+sprite_y,player_victory_center_y*256
    CHECK24 player_begin+sprite_xvel,0
    CHECK24 player_begin+sprite_yvel,0
    ld a,(player_begin+sprite_frame)
    cp 1
    jp nz,fixture_fail
    ld a,(player_victory_error)
    or a
    jp nz,fixture_fail

; Corrupt actor phase fails closed at the same centred HOLD.
    ld a,4
    ld (fixture_case),a
    ld a,99
    ld (player_begin+player_victory_phase),a
    call player_victory_update
    jp nc,fixture_fail
    ld a,(player_victory_error)
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

neg_hlu:
    push hl
    pop de
    ld hl,0
    or a
    sbc hl,de
    ret

bearing_to_coord:
    ld hl,0
    ret

sprite_compute_velocities:
    ret

; Approach phases reach the requested target after one visible movement call;
; loop motion is intentionally irrelevant to the phase/timing fixture.
move_sprite:
    ld a,(player_begin+player_victory_phase)
    cp PLAYER_VICTORY_LOOP
    ret z
    ld hl,(player_victory_target_x)
    ld (player_begin+sprite_x),hl
    ld hl,(player_victory_target_y)
    ld (player_begin+sprite_y),hl
    ret

player_draw_record:
    ld hl,(fixture_draws)
    inc hl
    ld (fixture_draws),hl
    ret
