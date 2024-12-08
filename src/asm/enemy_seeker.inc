speed_seeker: equ 0x000280 ; 2.5 pixels per frame

seeker_defaults:
    db 0x00	                ; id
    db enemy_seeker	        ; type
    dl BUF_SEEKER_000	    ; base_bufferId
    dl move_enemy_seeker	; move_program
    db 0	                ; collisions
    db 16	                ; dim_x
    db 16                   ; dim_y
    dl 0x000000	            ; x
    dl 0x000000	            ; y
    dl 0x000000	            ; xvel
    dl 0x000000	            ; yvel
    dl speed_seeker	        ; vel
    dl 0x008000	            ; heading
    dl 0x008000	            ; orientation
    db 0x00	                ; animation
    db 0x00	                ; animation_timer
    db 0x01	                ; move_timer
    db 0x00	                ; move_step
    db 10	                ; points
    db 0x02	                ; shield_damage

enemy_spawn_seeker:
    ld iy,seeker_defaults
    call spawn_sprite_from_sprite
    ret nc ; no carry means no free sprite slots, so we go home
; ix comes back with the pointer to the new sprite variables
; a comes back with the spriteId of the new sprite
; initialize the new sprite
    ld (iy+sprite_id),a
    call sprite_load_frames
; copy coordinates of active sprite to new sprite
    ld iy,(table_pointer) ; TODO: see if we can get IY to land here with the proper value
    and %00000001 ; 50/50 chance of moving left or right on spanw
    ld (iy+sprite_move_step),a 
; set collision flags
    set collides_with_laser,(iy+sprite_collisions)
    set collides_with_player,(iy+sprite_collisions)
; update the new sprite's position and make it visible
    call vdu_sprite_move_abs168
    call vdu_sprite_show
    ret
; end enemy_spawn_seeker

move_enemy_seeker:
; run the movement program for the seeker enemy
    call en_nav_seek_zag
    jp check_enemy_player_collisions
; end move_enemy_seeker

; contains the logic for how to move the enemy
; and then does the moving
; inputs: a fully-populated active sprite table
;         player position variables
; destroys: everything except index registers
; outputs: moving enemies
en_nav_seek_zag:
; set velocity and orientation by player's relative location
; move enemies y-axis
; where is player relative to us?
    call orientation_to_player ; uh.l angle to player, ub.c, ud.e = dx, dy
; is player above or below us?
    ld (ude),de ; dy
    ld a,(ude+2) ; deu
    rla ; shift sign bit into carry
    jp nc,@zig_zag ; player is below,evade
; player is even or above,so home in on current heading
    ld (iy+sprite_heading),hl ; save sprite heading
; fall through to @compute_velocities
@compute_velocities:
    call sprite_compute_velocities
; change the animation frame to match heading
    ld a,(iy+sprite_heading+1) ; we want the integer portion
    srl a ; divide integer portion by 8
    srl a
    srl a
    call vdu_sprite_select_frame
    jp move_sprite ; will return to caller from there
@zig_zag_start:
    ld iy,(table_pointer) ; TODO: see if we can get IY to land here with the proper value
    call rand_8
    and %00111111 ; limit it to 64
    set 3,a ; make sure it's at least 8
    ld (iy+sprite_move_timer),a ; store it
    ; fall through to @zig_zag
@zig_zag:
    ld a,(iy+sprite_move_timer)
    dec a
    ld (iy+sprite_move_timer),a
    jp nz,@zig_zag_no_switch
    ; otherwise flip direction and restart timer
    ld a,(iy+sprite_move_step)
    xor %1 ; flips bit one
    ld (iy+sprite_move_step),a ; store it
    jp nz,@F ; zag right
;otherwise zag left
    ld hl,160*256; southwest heading
    ld (iy+sprite_heading),hl ; save sprite heading
    jp @zig_zag_start
@@: ; zag right
    ld hl,96*256; southeast heading
    ld (iy+sprite_heading),hl ; save sprite heading
    jp @zig_zag_start
@zig_zag_no_switch:
    ; ld a,(sprite_orientation)
    ld hl,(iy+sprite_heading)
    jp @compute_velocities
; end en_nav_seek_zag
