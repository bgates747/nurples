    align 256
explosion_defaults:
    db explosion            ; type
    dl BUF_EXPLOSION_A	    ; base_bufferId
    dl move_explosion	    ; move_program
    db 0                    ; collisions
    db 7	                ; dim_x
    db 7                    ; dim_y
    db 1                    ; num_orientations
    db 5                    ; num_animations
    db 5	                ; animation
    db 8	                ; animation_timer
    db 0	                ; move_timer
    db 0	                ; move_step
    db 0	                ; points
    db 5	                ; shield_damage
    db 0                    ; frame

spawn_explosion:
    ld hl,explosion_defaults
    call alter_sprite
; other specific initialization
    ld hl,0
    ld (iy+sprite_xvel),hl
    ld hl,1*256
    ld (iy+sprite_yvel),hl
    ret
; end spawn_explosion

move_explosion:
; run the movement program for the explosion
    call move_sprite
    or a ; a will be non-zero if have scrolled off screen
    jp nz,@make_crater
; do the animations
    dec (iy+sprite_animation_timer)
    ret nz ; not time to bump animation frame
    inc (iy+sprite_frame)
    dec (iy+sprite_animation)
    jp z,@make_crater ; reached the end of the animations
    ld a,8 ; reset animation timer
    ld (iy+sprite_animation_timer),a
    ret
@make_crater: ; deactivate sprite and draw a crater in its place
    ld hl,BUF_CRATER
    call vdu_buff_select
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    call vdu_plot_bmp168
    jp table_deactivate_sprite
; end move_explosion