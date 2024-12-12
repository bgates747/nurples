    align 256
speed_fireball: equ 4*256 ; 6 pixels per frame 16.8 fixed

fireball_defaults:
    db enemy_projectile        ; type
    dl BUF_FIREBALL_A	    ; base_bufferId
    dl move_enemy_fireball	; move_program
    db collides_with_laser | collides_with_player ; collisions
    db 7	                ; dim_x
    db 7                    ; dim_y
    db 1                    ; num_orientations
    db 1                    ; num_animations
    db 0	                ; animation
    db 0	                ; animation_timer
    db 0	                ; move_timer
    db 0	                ; move_step
    db 20	                ; points
    db 5	                ; shield_damage
    db 0                    ; animation_frame

enemy_spawn_fireball:
    ld hl,fireball_defaults
    call spawn_sprite_from_sprite
    ret nc ; no carry means no free sprite slots, so we go home
; other specific initialization
; ------------------------------------------------
; center the fireball on the turret
    ld de,3*128 ; half the fireball size
    ld hl,(iy+sprite_x)
    add hl,de
    ld (ix+sprite_x),hl
    ld hl,(iy+sprite_y)
    add hl,de
    ld (ix+sprite_y),hl
; copy component velocities from targeting_computer
    ld hl,(Vp_x)
    ld (ix+sprite_xvel),hl
    ld hl,(Vp_y)
    ld (ix+sprite_yvel),hl
; move projectile one frame's distance so it isn't drawn right over turret
    ld hl,(ix+sprite_x)
    ld de,(ix+sprite_xvel)
    add hl,de
    ld (ix+sprite_x),hl
    ld hl,(ix+sprite_y)
    ld de,(ix+sprite_yvel)
    add hl,de
    ld (ix+sprite_y),hl
    ret
; end enemy_spawn_fireball

move_enemy_fireball:
; run the movement program for the fireball enemy
    call move_sprite
    ld a,(sprite_screen_edge)
    or a ; a will be non-zero if have scrolled off screen
    jp nz,table_deactivate_sprite
    jp check_enemy_collisions
; end move_enemy_fireball