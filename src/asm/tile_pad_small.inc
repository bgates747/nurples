pad_small_defaults:
    db 0x00                 ; id
    db tile_pad_small       ; type
    dl BUF_PAD              ; base_bufferId
    dl move_pad_small       ; move_program
    db collides_with_laser  ; collisions
    db 16                   ; dim_x
    db 16                   ; dim_y
    dl 0x000000             ; x
    dl 0x000000             ; y
    dl 0*256                ; xvel
    dl 1*256                ; yvel
    dl 1*256                ; vel
    dl 0                    ; heading
    dl 0                    ; orientation
    db 0                    ; animation
    db 0                    ; animation_timer
    db active_tile_timer    ; move_timer
    db 0                    ; move_step
    db 5                    ; points
    db 0                    ; shield_damage

activate_pad_small:
; set number of animation frames
    ld b,1
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