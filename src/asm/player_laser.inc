    align 256
speed_laser: equ 4*256 ; 6 pixels per frame 16.8 fixed
tmr_laser: ds 6 ; rate timer for firing laser
tmr_laser_reset: dl 120/3
laser_animation_timer_reset: equ 2 ; animate every two frames


laser_defaults:
    db player_air        ; type
    dl BUF_LASER_A	    ; base_bufferId
    dl move_player_laser	; move_program
    db 0 ; collisions
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

laser_control:

move_weapons:
; initialize pointers and loop counter
    ld iy,player_weapons_begin ; set iy to first record in table
    ; PUSH_ALL
    ; ; call vdu_home_cursor
    ; ; call dumpRegistersHex
    ; CALL DEBUG_PRINT_TABLE
    ; ; CALL DEBUG_WAITKEYPRESS
    ; POP_ALL

    ld b,player_weapons_max ; loop counter
@move_loop:
    ld (table_pointer),iy ; update table pointer
    push bc ; backup loop counter
; check top bit of sprite_type to see if sprite is just spawned
    ld a,(iy+sprite_type)
    bit 7,a
    jp nz,@next_record ; if just spawned, skip to next record
; check sprite_type to see if sprite is active
    and a ; if zero, sprite is dead 
    jp z,@next_record ; ... and we skip to next record
; otherwise we prepare to move the sprite
    ld hl,(iy+sprite_move_program) ; load the behavior subroutine address
    callHL
; move_weapons_loop_return: return from behavior subroutines
    ld iy,(table_pointer) ; get back table pointer
; ; now we check results of all the moves
;     bit sprite_just_died,(iy+sprite_collisions)
;     jp z,@draw_sprite ; if not dead, draw sprite
;     call table_deactivate_sprite 
;     jp @next_record ; and to the next record
@draw_sprite:
    ld a,(iy+sprite_id) ; get spriteId
    call vdu_sprite_select ; select sprite 
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    call vdu_sprite_move_abs168
; fall through to @next_record
@next_record:
    lea iy,iy+table_record_size
    xor a ; clears carry flag
    ld (sprite_screen_edge),a ; clear screen edge collision flag
    pop bc ; get back our loop counter
    djnz @move_loop ; loop until we've checked all the records
; reset all just spawned flags
    ld iy,player_weapons_begin
    ld b,player_weapons_max ; loop counter
@reset_just_spawned:
    push bc ; back up loop counter
    bit 7,(iy+sprite_type) ; check if just spawned
    jp z,@F
    res 7,(iy+sprite_type) ; clear just spawned flag
    ld a,(iy+sprite_id)
    call vdu_sprite_select
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    call vdu_sprite_move_abs168
    call vdu_sprite_show
@@:
    lea iy,iy+table_record_size
    pop bc ; get back loop counter
    djnz @reset_just_spawned
    ret
; end laser_control

player_spawn_laser:
; check laser spawn timer
    ld iy,tmr_laser
    call timestamp_tmr_get
    ret p ; not time to fire
; check if at max weapons active
    ld a,(player_weapons_count)
    cp player_weapons_max
    ret z ; at max weapons active
    inc a
    ld (player_weapons_count),a
; reset timer
    ld hl,(tmr_laser_reset)
    call timestamp_tmr_set
; fire the laser
    ld ix,player_weapons_begin
    ld b,player_weapons_max
    ld iy,player_begin
    ld (table_pointer),iy
    ld hl,laser_defaults
    call spawn_sprite_from_sprite_new
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
    ld hl,-speed_laser
    ld (ix+sprite_yvel),hl
    ret
; end player_spawn_laser

move_player_laser:
; run the movement program for the laser player
    call move_sprite
    ld a,(sprite_screen_edge)
    or a ; a will be non-zero if have scrolled off screen
    jp z,@F ; still on screen
    ld hl,player_weapons_count
    dec (hl)
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

init_player_laser:
; set pointer to laser sprite table
    ld iy,player_weapons_begin
    ld a,(iy+sprite_id) ; loop counter / sprite id
; define laser sprites
@loop:
    push af
	call vdu_sprite_select
	call vdu_sprite_clear_frames
	ld hl,BUF_LASER_A
	ld b,2 ; two bitmaps for laser
@sprite_laser_loop:
	push bc
	push hl
	call vdu_sprite_add_buff
	pop hl
	inc hl
	pop bc
	djnz @sprite_laser_loop
    pop af ; restore loop counter / sprite id
    inc a
    cp player_weapons_max
    jp nz,@loop
; set laser firing timer to zero
    ld iy,tmr_laser
    ld hl,0
    call timestamp_tmr_set
    ret