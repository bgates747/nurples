; Deterministic pure-CPU qualification for safe player spawn endpoints.

    assume adl=1
    org 040000h
    jp fixture_start

actor_record_size: equ 64
sprite_x: equ 1
sprite_y: equ 4
sprite_collisions: equ 29
sprite_dim_x: equ 30
sprite_dim_y: equ 31
tile_x: equ sprite_x
tile_y: equ sprite_y
tile_collisions: equ sprite_collisions
tile_dim_x: equ sprite_dim_x
tile_dim_y: equ sprite_dim_y
tile_shield_damage: equ 39
sprite_alive: equ 6

fixture_tile_capacity: equ 4
num_active_tiles: db 0
tile_stack: blkb fixture_tile_capacity*3,0
fixture_tile_table: blkb fixture_tile_capacity*actor_record_size,0
fixture_force_bad_stack: db 0
fixture_force_bad_record: db 0
fixture_arg_x: db 0
fixture_arg_y: db 0
fixture_arg_w: db 0
fixture_arg_h: db 0
fixture_arg_damage: db 0
fixture_arg_alive: db 0
fixture_case: db 0

    include "../../src/asm/player_spawn_resolver.inc"

FIXTURE_PASS: equ 02ah

    macro ADD_TILE x_value,y_value,w_value,h_value,damage_value,alive_value
        ld a,x_value
        ld b,y_value
        ld c,w_value
        ld d,h_value
        ld e,damage_value
        ld l,alive_value
        call fixture_add_tile
    endmacro

    macro CHECK_SUCCESS case_id,expected_x,expected_y,expected_direction
        ld a,case_id
        ld (fixture_case),a
        call player_spawn_resolve_endpoint
        jp nc,fixture_fail
        ld a,(player_spawn_resolver_result_flags)
        or a
        jp nz,fixture_fail
        ld a,(player_spawn_resolved_x)
        cp expected_x
        jp nz,fixture_fail
        ld a,(player_spawn_resolved_y)
        cp expected_y
        jp nz,fixture_fail
        ld a,(player_spawn_resolved_direction)
        cp expected_direction
        jp nz,fixture_fail
    endmacro

    macro CHECK_FAILURE case_id,expected_fault
        ld a,case_id
        ld (fixture_case),a
        call player_spawn_resolve_endpoint
        jp c,fixture_fail
        ld a,(player_spawn_resolver_result_flags)
        cp expected_fault
        jp nz,fixture_fail
    endmacro

fixture_start:
    ld sp,05ff00h

; Empty worlds use the canonical centre/lower-third endpoint.
    call fixture_clear
    CHECK_SUCCESS 1,120,224,PLAYER_SPAWN_DIRECTION_POSITIVE_X

; Scenery without collision damage cannot displace the endpoint.
    call fixture_clear
    ADD_TILE 0,112,255,128,0,1
    CHECK_SUCCESS 2,120,224,PLAYER_SPAWN_DIRECTION_POSITIVE_X

; A narrow full-height blocker on centre selects the first left candidate.
    call fixture_clear
    ADD_TILE 127,112,1,128,1,1
    CHECK_SUCCESS 3,104,224,PLAYER_SPAWN_DIRECTION_NEGATIVE_X

; Blocking centre and the first left candidate selects the first right one.
    ADD_TILE 111,112,1,128,1,1
    CHECK_SUCCESS 4,136,224,PLAYER_SPAWN_DIRECTION_POSITIVE_X

; The four Y candidates are tried before moving horizontally.
    call fixture_clear
    ADD_TILE 127,239,1,1,1,1
    CHECK_SUCCESS 5,120,208,PLAYER_SPAWN_DIRECTION_POSITIVE_X

; Structural corruption aborts rather than accepting a weakly checked point.
    call fixture_clear
    ld a,1
    ld (fixture_force_bad_stack),a
    ADD_TILE 0,0,1,1,0,1
    CHECK_FAILURE 6,1<<player_spawn_fault_bad_stack

    call fixture_clear
    ld a,1
    ld (fixture_force_bad_record),a
    ADD_TILE 0,0,1,1,0,1
    CHECK_FAILURE 7,1<<player_spawn_fault_bad_record

    call fixture_clear
    ADD_TILE 0,0,1,1,0,0
    CHECK_FAILURE 8,1<<player_spawn_fault_inactive_record

; One live damaging tile can reject the complete finite search grid.
    call fixture_clear
    ADD_TILE 0,112,255,128,1,1
    CHECK_FAILURE 9,1<<player_spawn_fault_no_candidate

    ld a,FIXTURE_PASS
    out (0),a

fixture_fail:
    ld a,(fixture_case)
    or a
    jp nz,@emit
    ld a,0feh
@emit:
    out (0),a

; inputs: A=x, B=y, C=w, D=h, E=damage, L=alive
fixture_add_tile:
    ld (fixture_arg_x),a
    ld a,b
    ld (fixture_arg_y),a
    ld a,c
    ld (fixture_arg_w),a
    ld a,d
    ld (fixture_arg_h),a
    ld a,e
    ld (fixture_arg_damage),a
    ld a,l
    ld (fixture_arg_alive),a

    ld hl,0
    ld a,(num_active_tiles)
    ld l,a
    ld h,actor_record_size
    mlt hl
    ld de,fixture_tile_table
    add hl,de
    push hl
    pop iy

    ld hl,0
    ld a,(fixture_arg_x)
    ld h,a
    ld (iy+tile_x),hl
    ld hl,0
    ld a,(fixture_arg_y)
    ld h,a
    ld (iy+tile_y),hl
    ld a,(fixture_arg_w)
    ld (iy+tile_dim_x),a
    ld a,(fixture_arg_h)
    ld (iy+tile_dim_y),a
    ld a,(fixture_arg_damage)
    ld (iy+tile_shield_damage),a
    xor a
    ld (iy+tile_collisions),a
    ld a,(fixture_arg_alive)
    or a
    jp z,@publish
    set sprite_alive,(iy+tile_collisions)
@publish:
    ld hl,0
    ld a,(num_active_tiles)
    ld l,a
    ld h,3
    mlt hl
    ld de,tile_stack
    add hl,de
    ld (hl),iy
    ld hl,num_active_tiles
    inc (hl)
    ret

fixture_clear:
    xor a
    ld (num_active_tiles),a
    ld (fixture_force_bad_stack),a
    ld (fixture_force_bad_record),a
    ld hl,tile_stack
    ld bc,fixture_tile_capacity*3
    call fixture_zero
    ld hl,fixture_tile_table
    ld bc,fixture_tile_capacity*actor_record_size
    call fixture_zero
    ret

; inputs: HL=start, BC=byte count
fixture_zero:
@loop:
    xor a
    ld (hl),a
    inc hl
    dec bc
    ld a,b
    or c
    jp nz,@loop
    ret

tile_stack_pointer_valid:
    ld a,(fixture_force_bad_stack)
    or a
    jp nz,@invalid
    scf
    ret
@invalid:
    xor a
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

; Same inclusive AABB contract as production check_collision_box.
check_collision_box:
    ld hl,0
    ld l,(ix+sprite_y+1)
    ld h,(ix+sprite_y+2)
    ld de,0
    ld e,(ix+sprite_dim_y)
    add hl,de
    ld e,(iy+sprite_y+1)
    ld d,(iy+sprite_y+2)
    xor a
    sbc hl,de
    jp m,@not_collision

    ld hl,0
    ld l,(iy+sprite_y+1)
    ld h,(iy+sprite_y+2)
    ld de,0
    ld e,(iy+sprite_dim_y)
    add hl,de
    ld e,(ix+sprite_y+1)
    ld d,(ix+sprite_y+2)
    xor a
    sbc hl,de
    jp m,@not_collision

    ld hl,0
    ld l,(ix+sprite_x+1)
    ld h,(ix+sprite_x+2)
    ld de,0
    ld e,(ix+sprite_dim_x)
    add hl,de
    ld e,(iy+sprite_x+1)
    ld d,(iy+sprite_x+2)
    xor a
    sbc hl,de
    jp m,@not_collision

    ld hl,0
    ld l,(iy+sprite_x+1)
    ld h,(iy+sprite_x+2)
    ld de,0
    ld e,(iy+sprite_dim_x)
    add hl,de
    ld e,(ix+sprite_x+1)
    ld d,(ix+sprite_x+2)
    xor a
    sbc hl,de
    jp m,@not_collision
    xor a
    ret
@not_collision:
    dec a
    ret
