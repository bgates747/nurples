laser_defaults:
    db player_air        ; type
    dl BUF_LASER_A	    ; base_bufferId
    dl move_player_laser	; move_program
    db %01000000            ; collisions bit 6 signals sprite alive
    db 5	                ; dim_x
    db 13                   ; dim_y
    db 1                    ; num_orientations
    db 2                    ; num_animations
    db 0	                ; animation
    db laser_animation_timer_reset ; animation_timer
    db 0	                ; move_timer
    db 0	                ; move_step
    db 0	                ; points
    db 0	                ; shield_damage
    db 0                    ; frame


player_lasers_max: equ 4
laser_speed: equ 4*256 ; 6 pixels per frame 16.8 fixed
tmr_laser: ds 6 ; rate timer for firing laser
tmr_laser_reset: dl 120/10
laser_animation_timer_reset: equ 2 ; animate every two frames

laser_power: db 64 ; integer
laser_power_last: db 0 ; integer
laser_power_max: db 64 ; integer
laser_power_per_shot: db 4 ; integer
charge_laser_timer: db 0
charge_laser_timer_max: db 6
bar_laser:
    dl 0 ; value
    dl 8 ; x0
    dl 304 ; y0
    dl 8+7 ; x1
    dl 304+63 ; y1
    dl 304-64 ; y-plot coordinate

init_bar_laser:
    ld a,(charge_laser_timer_max)
    ld (charge_laser_timer),a
    ld ix,bar_laser
    ld a,(laser_power_max)
    ld (ix+bar_val),a
    call bar_update_v
    ret
; end init_bar_laser

init_player_laser:
; set pointer to laser sprite table
    ld iy,player_weapons_begin
    ld a,(iy+sprite_id) ; sprite id
    ld e,player_lasers_max ; sprite loop counter
; define laser sprites
@sprites_loop:
    push af
    push de
	call vdu_sprite_select
	call vdu_sprite_clear_frames
	ld hl,BUF_LASER_A
	ld b,2 ; two bitmaps for laser
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
; set laser firing timer to zero
    ld iy,tmr_laser
    ld hl,0
    call timestamp_tmr_set
    ret
; end init_player_laser

player_move_weapons:
; initialize pointers and loop counter
    ld iy,player_weapons_begin ; set iy to first record in table
    ld b,player_lasers_max ; loop counter
    call move_sprite_group
    ret
; end player_move_weapons

player_spawn_laser:
 ; check laser spawn timer
    ld iy,tmr_laser
    call timestamp_tmr_get
    ret p ; not time to fire
; check laser power
    ld a,(laser_power)
    ld hl,laser_power_per_shot
    sub (hl) ; units of power per laser shot
    ret c ; not enough power to fire
; update power to reflect firing
    ld (laser_power),a
; reset timer
    ld hl,(tmr_laser_reset)
    call timestamp_tmr_set
; fire the laser
    ld ix,player_weapons_begin
    ld b,player_lasers_max
    ld iy,player_begin
    ld (table_pointer),iy
    ld hl,laser_defaults
    call spawn_sprite_from_sprite
    ret nc ; no carry means no free sprite slots, so we go home
; other specific initialization
; ------------------------------------------------
    ld iy,player_begin
; x-coordinate
    ld hl,(iy+sprite_x)
    ld de,5*256 ; horizontal center with player sprite
    add hl,de
    ld (ix+sprite_x),hl
; y-coordinate
    ld hl,(iy+sprite_y)
    ld de,-6*256 ; set laser y a few pixels above player
    add hl,de
    ld (ix+sprite_y),hl
; set laser velocity
    ld hl,-laser_speed
    ld (ix+sprite_yvel),hl
    ret
; end player_spawn_laser

move_player_laser:
; run the movement program for the laser
    call move_sprite
    ld a,(sprite_screen_edge)
    and a,collide_top ; check collision with top of screen
    jp z,@F ; still on screen
    jp table_deactivate_sprite
@@: ; check animation timer
    dec (iy+sprite_animation_timer)
    jp nz,@F
    ld a,laser_animation_timer_reset
    ld (iy+sprite_animation_timer),a
    ld a,(iy+sprite_id)
    call vdu_sprite_select
    call vdu_sprite_next_frame
@@: ; all done
    ret
; end move_player_laser

charge_laser:
; check laser timer
    ld hl,charge_laser_timer
    dec (hl)
    ret nz
    ld a,(charge_laser_timer_max)
    ld (hl),a
    ld hl,laser_power
    inc (hl)
    ld a,64
    cp (hl)
    ret nc
    ld (hl),a
    ret
; end charge_laser
draw_laser:
    call charge_laser
    ld hl,laser_power_last
    ld a,(laser_power)
    cp (hl)
    ret z ; no change to update
    ld (hl),a
    ld ix,bar_laser
    ld (ix+bar_val),a ; TODO: this is redundant
    call bar_update_v
    ret