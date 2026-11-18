; Deterministic pure-CPU qualification for actor-owned turret tracking.

    assume adl=1
    org 040000h
    jp fixture_start

actor_record_size: equ 64
actor_record_custom: equ 42
tile_heading: equ 16
speed_fireball: equ 4*256

Heading_p: dl 0
Vp_x: dl 0
Vp_y: dl 0
fixture_turret: blkb actor_record_size,0
fixture_case: db 0
fixture_random_index: db 0
fixture_random_values: db 1,7,0,31

    include "../../src/asm/heading_slew.inc"
    include "../../src/asm/turret_tracking.inc"

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
    ld iy,fixture_turret

; Initialization owns desired heading, direction and bounded duration.
    ld a,1
    ld (fixture_case),a
    ld hl,01234h
    ld (iy+tile_heading),hl
    xor a
    ld (fixture_random_index),a
    push iy
    pop ix
    call turret_tracking_init
    CHECK24 fixture_turret+turret_desired_heading,01234h
    ld a,(iy+turret_search_direction)
    cp TURRET_SEARCH_NEGATIVE
    jp nz,fixture_fail
    ld a,(iy+turret_search_timer)
    cp 31
    jp nz,fixture_fail

; Live targets take the shortest wrapped path at the fixed physical rate.
    ld a,2
    ld (fixture_case),a
    ld hl,0fe00h
    ld (iy+tile_heading),hl
    ld hl,00200h
    call turret_track_desired_heading
    jp c,fixture_fail
    CHECK24 fixture_turret+tile_heading,00000h
    CHECK24 fixture_turret+turret_desired_heading,00200h

    ld a,3
    ld (fixture_case),a
    ld hl,00200h
    ld (iy+tile_heading),hl
    ld hl,0fe00h
    call turret_track_desired_heading
    jp c,fixture_fail
    CHECK24 fixture_turret+tile_heading,00000h

; Equality reports reached without moving.
    ld a,4
    ld (fixture_case),a
    ld hl,02000h
    ld (iy+tile_heading),hl
    call turret_track_desired_heading
    jp nc,fixture_fail
    CHECK24 fixture_turret+tile_heading,02000h

; Search holds one direction for its actor-owned duration, then flips.
    ld a,5
    ld (fixture_case),a
    ld hl,0
    ld (iy+tile_heading),hl
    ld a,TURRET_SEARCH_POSITIVE
    ld (iy+turret_search_direction),a
    ld a,2
    ld (iy+turret_search_timer),a
    call turret_search_update
    CHECK24 fixture_turret+tile_heading,00200h
    ld a,(iy+turret_search_timer)
    cp 1
    jp nz,fixture_fail
    call turret_search_update
    CHECK24 fixture_turret+tile_heading,00400h
    ld a,(iy+turret_search_timer)
    or a
    jp nz,fixture_fail
    ld a,2
    ld (fixture_random_index),a ; next value is zero: new duration 24
    call turret_search_update
    CHECK24 fixture_turret+tile_heading,00200h
    ld a,(iy+turret_search_direction)
    cp TURRET_SEARCH_NEGATIVE
    jp nz,fixture_fail
    ld a,(iy+turret_search_timer)
    cp 23
    jp nz,fixture_fail

; Shot preparation publishes the physical heading and returned components.
    ld a,6
    ld (fixture_case),a
    ld hl,05580h
    ld (iy+tile_heading),hl
    call turret_prepare_physical_shot
    CHECK24 Heading_p,05580h
    CHECK24 Vp_x,00111h
    CHECK24 Vp_y,00222h

    ld a,FIXTURE_PASS
    out (0),a

fixture_fail:
    ld a,(fixture_case)
    or a
    jp nz,@emit
    ld a,0feh
@emit:
    out (0),a

rand_8:
    ld hl,fixture_random_values
    ld a,(fixture_random_index)
    ld e,a
    ld d,0
    add hl,de
    inc a
    ld (fixture_random_index),a
    ld a,(hl)
    ret

polar_to_cartesian:
    ld bc,00111h
    ld de,00222h
    ret
