    align 256
crater_defaults:
    db tile_crater          ; type
    dl BUF_EXPLOSION_A      ; base_bufferId
    dl move_crater          ; move_program
    db %01000000            ; collisions bit 6 signals sprite alive
    db 16                   ; dim_x
    db 16                   ; dim_y
    db 1                    ; num_orientations
    db 4                    ; num_animations
    db 5	                ; animation
    db 8                    ; animation_timer
    db 0                    ; move_timer
    db 0                    ; move_step
    db 0                    ; points
    db 0                    ; shield_damage
    db 0                    ; frame

activate_crater:
    ld hl,crater_defaults
    call spawn_active_tile_from_tile
; other specific initialization
    ret
; end activate_crater

move_crater:
    call move_active_tile 
    jp z,table_deactivate_tile
; do animation
    dec (iy+tile_animation_timer)
    ret nz ; not time to animate
    dec (iy+sprite_animation) ; check whether time to transition to crater
    call z,@make_crater
    ld a,8
    ld (iy+tile_animation_timer),a
    ld a,(iy+tile_frame)
    inc a
    and %00000011 ; modulo 4
    ld (iy+tile_frame),a
    jp draw_active_tile
@make_crater:
    ld hl,BUF_CRATER_0
    ld (iy+tile_base_bufferId),hl
    ld a,-1 ; will wrap to zero on return
    ld (iy+tile_frame),a
    ret
; end move_crater