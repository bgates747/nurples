    align 256
pad_small_defaults:
    db tile_pad_small       ; type
    dl BUF_PAD              ; base_bufferId
    dl move_pad_small       ; move_program
    db collides_with_laser  ; collisions
    db 16                   ; dim_x
    db 16                   ; dim_y
    db 1                    ; num_orientations
    db 1                    ; num_animations
    db 0                    ; animation
    db 0                    ; animation_timer
    db 24                   ; move_timer
    db 0                    ; move_step
    db 5                    ; points
    db 0                    ; shield_damage

activate_pad_small:
    ld hl,pad_small_defaults
; get next available sprite id
    call spawn_sprite_from_tile
    ret nc ; no carry means no free sprite slots, so we go home
; TODO: other specific initialization
    ret
; end activate_pad_small

move_pad_small:
    call move_active_tile 
    or a ; a will be non-zero if have scrolled off screen
    jp nz,table_deactivate_sprite
; check collisions
    call check_enemy_collisions
    or a ; a will be negative if have collided with player or laser
    ret m ; we're dead so nothing more to do
; check whether it is time to spawn a seeker
    dec (iy+sprite_move_timer)
    ret nz
    ; ld a,24 ; TODO: make this a random number
    ; ld (iy+sprite_move_timer),a
    ; ld iy,pad_small_defaults
    ; call enemy_spawn_seeker
    ret