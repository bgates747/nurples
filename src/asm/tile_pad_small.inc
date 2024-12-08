pad_small_defaults:
    db tile_pad_small	    ; type
    dl BUF_PAD      	    ; base_bufferId
    dl move_enemy_seeker	; move_program
    db collides_with_laser  ; collisions
    db 16	                ; dim_x
    db 16                   ; dim_y
    db 1                    ; num_orientations
    db 1                    ; num_animations
    db 0	                ; animation
    db 0	                ; animation_timer
    db 1	                ; move_timer
    db 0	                ; move_step
    db 5	                ; points
    db 0	                ; shield_damage

activate_pad_small:
    ld iy,pad_small_defaults
    ld b,1 ; number of frames
; get next available sprite id
    call spawn_sprite_from_tile
    ret nc ; no carry means no free sprite slots, so we go home
; TODO: other specific initialization
    ret
; end activate_pad_small

move_pad_small:
    dec (iy+sprite_move_timer)
    ret nz
    ld a,24
    ld (iy+sprite_move_timer),a
    call enemy_spawn_seeker
    ret