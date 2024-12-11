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
    db 0                    ; animation_frame

activate_turret_fireball:
    ld hl,turret_fireball_defaults
; get next available sprite id
    call spawn_sprite_from_tile
    ret nc ; no carry means no free sprite slots, so we go home
; TODO: other specific initialization
    ld a,64
    ld (iy+sprite_move_timer),a ; when this hits zero,will spawn a fireball
    ret
; end activate_turret_fireball

move_turret_fireball:
    call move_active_tile 
    or a ; a will be non-zero if have scrolled off screen
    jp nz,table_deactivate_sprite
; target player
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
    add a,64 ; range is now 64-127
    ld (iy+sprite_move_timer),a ; when this hits zero,will spawn a fireball
    ret