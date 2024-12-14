    align 256
turret_fireball_defaults:
    db enemy_ground         ; type
    dl BUF_TURRET_000       ; base_bufferId
    dl move_turret_fireball ; move_program
    db collides_with_laser  ; collisions
    db 16                   ; dim_x
    db 16                   ; dim_y
    db 32                   ; num_orientations
    db 1                    ; num_animations
    db 0                    ; animation
    db 0                    ; animation_timer
    db 24                   ; move_timer
    db 0                    ; move_step
    db 5                    ; points
    db 0                    ; shield_damage
    db 0                    ; frame

activate_turret_fireball:
    ld hl,turret_fireball_defaults
; get next available sprite id
    call spawn_active_tile
    ret nc ; no carry means no free sprite slots, so we go home
; other specific initialization
    ld a,64
    ld (iy+sprite_move_timer),a ; when this hits zero,will spawn a seeker
    ret
; end activate_turret_fireball

move_turret_fireball:
    call move_active_tile 
    or a ; a will be non-zero if have scrolled off screen
    jp nz,table_deactivate_tile
    call check_enemy_collisions
    ret m ; was collision so nothing more to do
; target player
    ld hl,speed_fireball
    ld (Vp),hl ; param for targeting_computer
    call orientation_to_player
    call targeting_computer
    ld (iy+sprite_heading),hl ; save sprite heading
    call sprite_frame_heading
; check whether it is time to spawn a fireball
    dec (iy+sprite_move_timer)
    ret nz
; spwan a fireball    
    call enemy_spawn_fireball
    ; fall through to reset move timer
turret_fireball_reset_move_timer:
    call rand_8     ; snag a random number
    and %00011111   ; keep only 5 lowest bits (max 31)
    add a,24 ; range is now 24-55
    ld (iy+sprite_move_timer),a ; when this hits zero,will spawn a fireball
    ret