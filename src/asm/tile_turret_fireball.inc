turret_fireball_custom_begin: equ actor_record_custom
turret_fireball_custom_end: equ turret_fireball_custom_begin
    .if turret_fireball_custom_end-turret_fireball_custom_begin
        dl turret_fireball_custom_span_mismatch
    .endif

turret_fireball_defaults:
    db tile_turret_fireball ; type
    dl BUF_TURRET_000       ; base_bufferId
    dl move_turret_fireball ; move_program
    db %01000000            ; collisions bit 6 signals sprite alive
    db 16                   ; dim_x
    db 16                   ; dim_y
    db 32                   ; num_orientations
    db 1                    ; num_animations
    db 0                    ; animation
    db 0                    ; animation_timer
    db 0                    ; move_timer
    db 0                    ; move_step
    db 5                    ; points
    db 0                    ; shield_damage
    db 0                    ; frame

activate_turret_fireball:
    ld hl,turret_fireball_defaults
; get next available active tile id
    call spawn_active_tile
    ret nc ; no carry means no free active tile slots, so we go home
; other specific initialization
    call turret_tracking_init
    call rand_8     ; snag a random number
    and %00011111 ; keep only 5 lowest bits (max 31)
    ld (ix+tile_move_timer),a ; when this hits zero,will spawn a fireball
    ret
; end activate_turret_fireball

move_turret_fireball:
    call move_active_tile 
    jp z,table_deactivate_tile
    call check_enemy_collision_laser
    jp z,kill_turret_fireball
; Every non-live player phase uses the same harmless rate-limited search.
; Search owns no reload state and cannot fire.
    ld hl,(player_state)
    ld de,player_alive
    or a
    sbc hl,de
    jp nz,turret_fireball_search
; target player
    call tile_orientation_to_player
    ld hl,speed_fireball
    ld (Vp),hl ; param for targeting_computer
    call targeting_computer
    call turret_track_desired_heading
    call tile_frame_heading
    call draw_active_tile
; check whether it is time to spawn a fireball      
    call active_tile_launch_timer_tick
    ret nz
; Fire from the physical barrel, never the ideal lead solution.
; Restore the turret record before rearming, regardless of allocation result.
    push iy
    call turret_prepare_physical_shot
    call enemy_spawn_fireball
    pop iy
    ; fall through to reset move timer
turret_fireball_reset_move_timer:
    call rand_8     ; snag a random number
    and %00011111   ; keep only 5 lowest bits (max 31)
    add a,24 ; range is now 24-55
    ld (iy+tile_move_timer),a ; when this hits zero,will spawn a fireball
    ret

turret_fireball_search:
    call turret_search_update
    call tile_frame_heading
    call draw_active_tile
    ret

kill_turret_fireball:
    call update_score
    jp transform_active_tile_to_crater
; end move_turret_fireball
