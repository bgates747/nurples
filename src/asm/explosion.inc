explosions_max: equ 8
explosion_defaults:
    db explosion            ; type
    dl BUF_EXPLOSION_A	    ; base_bufferId
    dl move_explosion	    ; move_program
    db %01000000            ; collisions bit 6 signals sprite alive
    db 7	                ; dim_x
    db 7                    ; dim_y
    db 1                    ; num_orientations
    db 5                    ; num_animations
    db 5	                ; animation
    db 8	                ; animation_timer
    db 0	                ; move_timer
    db 0	                ; move_step
    db 0	                ; points
    db 0	                ; shield_damage
    db 0                    ; frame

spawn_explosion:
    ld hl,explosion_defaults
    ld ix,explosions_base
    ld b,explosions_max
    call spawn_sprite_from_sprite
    jp nc,table_deactivate_sprite ; no carry means no free sprite slots, so we deactivate parent sprite and go home
; other specific initialization
    ld hl,0
    ld (ix+sprite_xvel),hl
    ld hl,1*256
    ld (ix+sprite_yvel),hl
; deactivate parent sprite
    jp table_deactivate_sprite
; end spawn_explosion

spawn_explosion_player:
    ld iy,player_begin
    ld (table_pointer),iy
    ld hl,explosion_defaults
    ld ix,explosions_base
    ld b,explosions_max
    call spawn_sprite_from_sprite
    jp nc,table_deactivate_sprite ; no carry means no free sprite slots, so we deactivate parent sprite and go home
; other specific initialization
    ret
; end spawn_explosion_player

tile_spawn_explosion:
    ld hl,explosion_defaults
    ld ix,explosions_base
    ld b,explosions_max
    call spawn_sprite_from_tile
    jp nc,table_deactivate_tile ; no carry means no free sprite slots, so we deactivate parent tile and go home
; other specific initialization
    ld hl,0
    ld (ix+sprite_xvel),hl
    ld hl,1*256
    ld (ix+sprite_yvel),hl
; deactivate parent sprite
    jp table_deactivate_tile
; end tile_spawn_explosion

move_explosion:
; run the movement program for the explosion
    call move_sprite
    or a ; a will be non-zero if have scrolled off screen
    jp nz,table_deactivate_sprite
; do the animations
    dec (iy+sprite_animation_timer)
    ret nz ; not time to bump animation frame
    inc (iy+sprite_frame)
    dec (iy+sprite_animation)
    jp z,table_deactivate_sprite ; reached the end of the animations
    ld a,8 ; reset animation timer
    ld (iy+sprite_animation_timer),a
    ret
; end move_explosion

move_explosions:
; initialize pointers and loop counter
    ld iy,explosions_base ; set iy to first record in table
    ld b,explosions_max ; loop counter
    call move_sprite_group
    ret
; end move_explosions

init_explosions:
; set pointer to explosion sprite table
    ld iy,explosions_base
    ld a,(iy+sprite_id) ; sprite id
    ld e,explosions_max ; sprite loop counter
; define explosion sprites
@sprites_loop:
    push af
    push de
	call vdu_sprite_select
	call vdu_sprite_clear_frames
	ld hl,BUF_EXPLOSION_A
	ld b,5 ; 5 bitmaps for explosion
@frames_loop:
	push bc
	push hl
	call vdu_sprite_add_buff
	pop hl
	inc hl
	pop bc
	djnz @frames_loop
    pop de ; sprite loop counter
    pop af ; sprite id
    inc a
    dec e
    jp nz,@sprites_loop
    ret
; end init_explosions