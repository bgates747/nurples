; Deterministic pure-CPU qualification for seeker pad identity and retreat.

    assume adl=1
    org 040000h
    jp fixture_start

actor_record_size: equ 64
actor_record_generation: equ 41
actor_record_custom: equ 42
actor_type_mask: equ %01111111
tile_table_record_size: equ 64

sprite_id: equ 0
sprite_x: equ 1
sprite_y: equ 4
sprite_xvel: equ 7
sprite_yvel: equ 10
sprite_vel: equ 13
sprite_heading: equ 16
sprite_type: equ 22
sprite_move_program: equ 26
sprite_collisions: equ 29
sprite_frame: equ 40
tile_id: equ sprite_id
tile_x: equ sprite_x
tile_y: equ sprite_y
tile_type: equ sprite_type
tile_collisions: equ sprite_collisions
sprite_alive: equ 6

seeker_state: equ actor_record_custom
seeker_orbit_origin_x: equ seeker_state+1
seeker_orbit_angle: equ seeker_orbit_origin_x+1
seeker_custom_end: equ seeker_orbit_angle+1

tile_pad_small: equ 1
enemy_air: equ 1
field_bottom: equ 335
collide_left: equ %00000010
collide_right: equ %00000001

fixture_seeker: blkb actor_record_size,0
tile_table_base:
fixture_pads: blkb 2*tile_table_record_size,0
tile_table_end:
table_pointer: dl fixture_seeker
sprite_screen_edge: db 0
fixture_force_bad_record: db 0
fixture_collision: db 0
fixture_deactivated: db 0
fixture_case: db 0

    include "../../src/asm/seeker_retreat.inc"

FIXTURE_PASS: equ 02ah

    macro CHECK8 address,expected
        ld a,(address)
        cp expected
        jp nz,fixture_fail
    endmacro

    macro CHECK24 address,expected
        ld hl,(address)
        ld de,expected
        or a
        sbc hl,de
        jp nz,fixture_fail
    endmacro

fixture_start:
    ld sp,05ff00h
    call fixture_clear
    call fixture_prepare_pad_one

; Capture records the exact physical id and generation.
    ld a,1
    ld (fixture_case),a
    ld ix,fixture_seeker
    ld iy,fixture_pads+tile_table_record_size
    call seeker_capture_spawn_pad
    jp nc,fixture_fail
    CHECK8 fixture_seeker+seeker_spawn_pad_id,1
    CHECK8 fixture_seeker+seeker_spawn_pad_generation,7
    CHECK8 fixture_seeker+seeker_completed_orbits,0

; The unchanged visible lifetime resolves back to the exact record.
    ld a,2
    ld (fixture_case),a
    ld ix,fixture_seeker
    call seeker_resolve_spawn_pad
    jp nc,fixture_fail
    push iy
    pop hl
    ld de,fixture_pads+tile_table_record_size
    or a
    sbc hl,de
    jp nz,fixture_fail

; Generation reuse, wrong type, inactivity and off-screen position all fail.
    ld a,3
    ld (fixture_case),a
    inc (iy+actor_record_generation)
    ld ix,fixture_seeker
    call seeker_resolve_spawn_pad
    jp c,fixture_fail
    call fixture_prepare_pad_one

    ld a,4
    ld (fixture_case),a
    ld a,9
    ld (fixture_pads+tile_table_record_size+tile_type),a
    call seeker_resolve_spawn_pad
    jp c,fixture_fail
    call fixture_prepare_pad_one

    ld a,5
    ld (fixture_case),a
    res sprite_alive,(iy+tile_collisions)
    call seeker_resolve_spawn_pad
    jp c,fixture_fail
    call fixture_prepare_pad_one

    ld a,6
    ld (fixture_case),a
    ld hl,field_bottom*256
    ld (fixture_pads+tile_table_record_size+tile_y),hl
    call seeker_resolve_spawn_pad
    jp c,fixture_fail
    call fixture_prepare_pad_one

; A valid pad publishes pad-retreat only after state is initialized.
    ld a,7
    ld (fixture_case),a
    ld iy,fixture_seeker
    call seeker_enter_retreat
    jp nc,fixture_fail
    CHECK8 fixture_seeker+seeker_state,seeker_state_retreat_pad
    CHECK24 fixture_seeker+sprite_move_program,move_enemy_seeker_retreat_pad

; A lost pad chooses the nearest horizontal edge and physical velocity.
    ld a,8
    ld (fixture_case),a
    ld a,SEEKER_PAD_NONE
    ld (fixture_seeker+seeker_spawn_pad_id),a
    ld hl,80*256
    ld (fixture_seeker+sprite_x),hl
    ld hl,2*256
    ld (fixture_seeker+sprite_vel),hl
    ld iy,fixture_seeker
    call seeker_enter_retreat
    CHECK8 fixture_seeker+seeker_state,seeker_state_retreat_edge
    CHECK24 fixture_seeker+sprite_heading,seeker_edge_heading_left
    CHECK24 fixture_seeker+sprite_xvel,-2*256

    ld a,9
    ld (fixture_case),a
    ld hl,180*256
    ld (fixture_seeker+sprite_x),hl
    ld iy,fixture_seeker
    call seeker_enter_retreat
    CHECK24 fixture_seeker+sprite_heading,seeker_edge_heading_right
    CHECK24 fixture_seeker+sprite_xvel,2*256

; Two complete orbit wraps deterministically enter the same retreat policy.
    ld a,10
    ld (fixture_case),a
    xor a
    ld (fixture_seeker+seeker_orbit_angle),a
    ld (fixture_seeker+seeker_completed_orbits),a
    ld iy,fixture_seeker
    call seeker_orbit_note_step
    jp c,fixture_fail
    CHECK8 fixture_seeker+seeker_completed_orbits,1
    call seeker_orbit_note_step
    jp nc,fixture_fail
    CHECK8 fixture_seeker+seeker_completed_orbits,2
    CHECK8 fixture_seeker+seeker_state,seeker_state_retreat_edge

; Edge exit and pad docking both release the seeker record.
    ld a,11
    ld (fixture_case),a
    ld iy,fixture_seeker
    set sprite_alive,(iy+sprite_collisions)
    xor a
    ld (sprite_screen_edge),a
    ld (fixture_deactivated),a
    call move_enemy_seeker_retreat_edge
    CHECK8 fixture_deactivated,0
    ld a,collide_left
    ld (sprite_screen_edge),a
    call move_enemy_seeker_retreat_edge
    CHECK8 fixture_deactivated,1

    ld a,12
    ld (fixture_case),a
    call fixture_prepare_pad_one
    ld a,1
    ld (fixture_seeker+seeker_spawn_pad_id),a
    ld a,7
    ld (fixture_seeker+seeker_spawn_pad_generation),a
    ld iy,fixture_seeker
    set sprite_alive,(iy+sprite_collisions)
    ld a,1
    ld (fixture_collision),a
    xor a
    ld (fixture_deactivated),a
    call move_enemy_seeker_retreat_pad
    CHECK8 fixture_deactivated,1

    ld a,FIXTURE_PASS
    out (0),a

fixture_fail:
    ld a,(fixture_case)
    or a
    jp nz,@emit
    ld a,0feh
@emit:
    out (0),a

fixture_clear:
    ld hl,fixture_seeker
    ld bc,actor_record_size+2*tile_table_record_size
@loop:
    xor a
    ld (hl),a
    inc hl
    dec bc
    ld a,b
    or c
    jp nz,@loop
    xor a
    ld (fixture_force_bad_record),a
    ld (fixture_collision),a
    ld (fixture_deactivated),a
    ret

fixture_prepare_pad_one:
    ld iy,fixture_pads+tile_table_record_size
    ld a,1
    ld (iy+tile_id),a
    ld a,7
    ld (iy+actor_record_generation),a
    ld a,tile_pad_small
    ld (iy+tile_type),a
    set sprite_alive,(iy+tile_collisions)
    ld hl,100*256
    ld (iy+tile_x),hl
    ld hl,120*256
    ld (iy+tile_y),hl
    ret

tile_table_record_pointer_valid:
    ld a,(fixture_force_bad_record)
    or a
    jp nz,@invalid
    scf
    ret
@invalid:
    xor a
    ret

polar_to_cartesian:
    ld a,h
    cp 192
    jp z,@left
    push de
    pop bc
    ld de,0
    ret
@left:
    ld hl,0
    or a
    sbc hl,de
    push hl
    pop bc
    ld de,0
    ret

check_collision_box:
    ld a,(fixture_collision)
    or a
    jp nz,@collision
    ld a,0ffh
    or a
    ret
@collision:
    xor a
    ret

bearing_to_coord:
    ld hl,0
    ret

sprite_compute_velocities:
    ret

move_sprite:
    ld a,(sprite_screen_edge)
    ret

sprite_frame_heading:
    ret

table_deactivate_sprite:
    res sprite_alive,(iy+sprite_collisions)
    ld a,1
    ld (fixture_deactivated),a
    ret
