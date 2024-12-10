    align 256
speed_fireball: equ 0x000280 ; 2.5 pixels per frame

fireball_defaults:
    db enemy_fireball        ; type
    dl BUF_FIREBALL_A	    ; base_bufferId
    dl move_enemy_fireball	; move_program
    db collides_with_laser | collides_with_player ; collisions
    db 16	                ; dim_x
    db 16                   ; dim_y
    db 32                   ; num_orientations
    db 1                    ; num_animations
    db 0	                ; animation
    db 0	                ; animation_timer
    db 1	                ; move_timer
    db 0	                ; move_step
    db 20	                ; points
    db 5	                ; shield_damage
    db 0                    ; animation_frame

enemy_spawn_fireball:
    ld hl,fireball_defaults
    call spawn_sprite_from_sprite
    ret nc ; no carry means no free sprite slots, so we go home
; other specific initialization
    ld hl,speed_fireball
    ld (ix+sprite_vel),hl
    ret
; end enemy_spawn_fireball

move_enemy_fireball:
; run the movement program for the fireball enemy
    call move_sprite
    jp check_enemy_collisions
; end move_enemy_fireball