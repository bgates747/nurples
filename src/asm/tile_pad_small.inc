pad_small_defaults:
    db tile_pad_small       ; type
    dl BUF_TILE_PAD_0       ; base_bufferId
    dl move_pad_small       ; move_program
    db %01000000            ; collisions bit 6 signals sprite alive
    db 16                   ; dim_x
    db 16                   ; dim_y
    db 1                    ; num_orientations
    db 1                    ; num_animations
    db 0                    ; animation
    db 16                   ; animation_timer
    db 0                    ; move_timer
    db 0                    ; move_step
    db 5                    ; points
    db 0                    ; shield_damage
    db 0                    ; frame

activate_pad_small:
    ld hl,pad_small_defaults
; get next available active tile id
    call spawn_active_tile
    ret nc ; no carry means no free active tile slots, so we go home
; other specific initialization
    call rand_8     ; snag a random number
    and %00011111 ; keep only 5 lowest bits (max 31)
    ld (ix+tile_move_timer),a ; when this hits zero,will spawn a seeker
    ret
; end activate_pad_small

move_pad_small:
    call move_active_tile 
    jp z,table_deactivate_tile
    call check_enemy_collision_laser
    jp z,kill_pad_small
; do animation
    dec (iy+tile_animation_timer)
    jp nz,@F
    ld a,16
    ld (iy+tile_animation_timer),a
    ld a,(iy+tile_frame)
    inc a
    and %00000111 ; modulo 8
    ld (iy+tile_frame),a
    call draw_active_tile
@@: ; check whether it is time to spawn a seeker    
    dec (iy+tile_move_timer)
    ret nz
; spwan a seeker    
    call enemy_spawn_seeker
    ; fall through to reset move timer
pad_small_reset_move_timer:
    call rand_8     ; snag a random number
    and %00011111   ; keep only 5 lowest bits (max 31)
    add a,64 ; range is now 64-127
    ld (iy+tile_move_timer),a ; when this hits zero,will spawn a seeker
    ret

kill_pad_small:
    call update_score
    jp activate_crater
; end move_pad_small