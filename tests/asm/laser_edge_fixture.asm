; Assemble with copies of collisions.inc and laser_geometry.inc beside this file.
    assume adl=1
    org 0
    jp start
sprite_x: equ 1
sprite_y: equ 4
sprite_collisions: equ 29
sprite_dim_x: equ 30
sprite_dim_y: equ 31
field_right: equ 255
table_record_size: equ 64
player_lasers_max: equ 1
player_lasers_begin: equ 0x2000
player_begin: equ 0x2040
player_death_pending: db 0
c_red: equ 1
c_green: equ 2
dx168: dl 0
dy168: dl 0
distance168:
vdu_plot_rf:
vdu_gcol:
    ret
    include "collisions.inc"
start:
    ld sp,0xf000
    ld ix,player_lasers_begin
    ld hl,(ix+sprite_x)
    call laser_bound_muzzle_x
    ld (ix+sprite_x),hl
    ld iy,0x2040
    call check_enemy_collision_laser
    ld (0x2100),a
    ld iy,0x2080
    call check_enemy_collision_laser
    ld (0x2101),a
    ld a,1
    ld (0x2102),a
    halt
