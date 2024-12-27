    align 256
lightning_defaults:
    db tile_lightning       ; type
    dl BUF_LIGHTNING_H_0    ; base_bufferId
    dl move_lightning       ; move_program
    db %01000000            ; collisions bit 6 signals sprite alive
    db 16                   ; dim_x
    db 16                   ; dim_y
    db 1                    ; num_orientations
    db 2                    ; num_animations
    db 0                    ; animation
    db 8                    ; animation_timer
    db 0                    ; move_timer
    db 0                    ; move_step
    db 0                    ; points
    db -1                    ; shield_damage
    db 0                    ; frame

activate_lightning:
    ld hl,lightning_defaults
; get next available active tile id
    call spawn_active_tile
    ret nc ; no carry means no free active tile slots, so we go home
; other specific initialization
    ret
; end activate_lightning

move_lightning:
    call move_active_tile 
    jp z,table_deactivate_tile
    call check_enemy_collision_laser ; kills laser on collision but does not die
    call check_enemy_collision_player
    jp nz,@animate ; no collision so do animation
    call update_shields
@animate:
    dec (iy+tile_animation_timer)
    ret nz ; not time to animate
; reset the animation timer and bump animation
    ld a,8
    ld (iy+tile_animation_timer),a
    ld a,(iy+tile_frame)
    inc a
    and %00000001 ; modulo 1
    ld (iy+tile_frame),a
    jp draw_active_tile
    ret