    align 256
crater_defaults:
    db tile_crater          ; type
    dl BUF_CRATER_0         ; base_bufferId
    dl move_crater          ; move_program
    db %01000000            ; collisions bit 6 signals sprite alive
    db 16                   ; dim_x
    db 16                   ; dim_y
    db 1                    ; num_orientations
    db 4                    ; num_animations
    db 0                    ; animation
    db 8                    ; animation_timer
    db 0                    ; move_timer
    db 0                    ; move_step
    db 0                    ; points
    db 0                    ; shield_damage
    db 0                    ; frame

activate_crater:
; get next available active tile id
    ld hl,crater_defaults
    call spawn_active_tile_from_sprite
    ret nc ; no carry means no free active tile slots, so we go home
; other specific initialization
    ret
; end activate_crater

move_crater:
    call move_active_tile 
    jp z,table_deactivate_tile
; do animation
    dec (iy+tile_animation_timer)
    ret nz ; not time to animate
    ld a,8
    ld (iy+tile_animation_timer),a
    ld a,(iy+tile_frame)
    inc a
    and %00000011 ; modulo 4
    ld (iy+tile_frame),a
    jp draw_active_tile
; end move_crater