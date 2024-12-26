    align 256
electrode_r_defaults:
    db 11     ; type
    dl BUF_CRATER           ; base_bufferId
    dl move_electrode_r     ; move_program
    db %01000000            ; collisions bit 6 signals sprite alive
    db 16                   ; dim_x
    db 16                   ; dim_y
    db 1                    ; num_orientations
    db 1                    ; num_animations
    db 0                    ; animation
    db 0                    ; animation_timer
    db 0                    ; move_timer
    db 0                    ; move_step
    db 5                    ; points
    db 0                    ; shield_damage
    db 0                    ; frame

activate_electrode_r:
    ld hl,electrode_r_defaults
; get next available active tile id
    call spawn_active_tile
    ret nc ; no carry means no free active tile slots, so we go home
; other specific initialization

    ret
; end activate_electrode_r

move_electrode_r:
    call move_active_tile 
    jp z,table_deactivate_tile
    call check_enemy_collision_laser
    jp z,kill_electrode_r
    ret

kill_electrode_r:
    jp tile_kill
; end move_electrode_r