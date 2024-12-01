max_enemy_sprites: db 1 

; sprite_type
enemy_dead: equ 0
enemy_seeker: equ 1
enemy_medium: equ 2
enemy_large: equ 3
landing_pad: equ 4
laser_turret: equ 5
fireballs: equ 6
explosion: equ 7

move_enemies:
; are there any active enemies or explosions?
    ld a,(table_active_sprites)
    and a ; will be zero if no alive enemies or explosions
    jp nz,@they_live
    ld hl,enemies_dead
    ld (enemies_state),hl
    ret
@they_live:
; initialize pointers and loop counter
    ld iy,table_base ; set iy to first record in table
    ld b,table_max_records ; loop counter
@move_loop:
    ld (table_pointer),iy ; update table pointer
    push bc ; backup loop counter
; check sprite_type to see if sprite is active
    ld a,(iy+sprite_type)
    and a ; if zero, sprite is dead 
    jp z,@next_record ; ... and we skip to next record
; otherwise we prepare to move the sprite
    ld a,(iy+sprite_id) ; get spriteId
    call vdu_sprite_select ; select sprite 
    ld hl,(iy+sprite_move_program) ; load the behavior subroutine address
    callHL
; move_enemies_loop_return: return from behavior subroutines
    ld iy,(table_pointer) ; get back table pointer
; now we check results of all the moves
    ld a,(iy+sprite_collisions)
    and sprite_just_died
    ld a,(iy+sprite_id) ; get spriteId for the deactivate_sprite call if needed
    jp z,@draw_sprite ; if not dead,draw sprite
    call table_deactivate_sprite ; otherwise we ded
    res sprite_just_died,(iy+sprite_collisions) ; clear just died flag
    jp @next_record ; and to the next record
    
@draw_sprite:
; if we got here sprite will have already been activated
; so all we need to do is set its coordinates and draw it
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    call vdu_sprite_move_abs168
; fall through to next record
@next_record:
    ld de,table_bytes_per_record
    add iy,de ; point to next record
    xor a ; clears carry flag
    ld (sprite_screen_edge),a ; clear screen edge collision flag
    pop bc ; get back our loop counter
    djnz @move_loop ; loop until we've checked all the records
    ret ; and we're out

; ; ######### SPRITE BEHAVIOR ROUTINES #########
; ; each sprite in the table must have one of these defined
; ; but they need not be unique to a particular sprite
; ; these are jumped to from move_enemies_do_program,but could come from other places
; ; and have the option but not obligation to go back to move_enemies_loop_return
; ; but they can call anything they want between those two endpoints
; move_programs: ; bookmark in case we want to know the first address of the first subroutine

; move_nop: ; does nothing but burn a few cycles changing the PC
;     ret

; move_explosion:
;     call animate_explosion 
;     ret

move_enemy_seeker:
    call en_nav_seek_zag
    ld ix,(laser_start_variables)
    call check_collision
    jp p,@F ; no collision
    jp enemy_kill
@@:
    ld ix,(player_start_variables)
    call check_collision
    ret p ; no collision
    jp enemy_kill

speed_seeker: equ 0x000280 ; 2.5 pixels per frame

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
    push hl ; we need it back to set rotation frame
; fall through to @compute_velocities
@compute_velocities:
    call sprite_compute_velocities
; change the animation frame to match heading
    pop af ; get back heading
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

enemy_init_from_landing_pad:
; get next available spriteId
    call table_get_next_id
    ret nc ; no carry means no free sprite slots, so we go home
; ix comes back with the pointer to the new sprite variables
    push ix ; de picks it up when we're ready for the copy to the table
; a comes back with the spriteId of the new sprite
    ld (@id),a
; initialize the new sprite
    call vdu_sprite_select
    call vdu_sprite_clear_frames
    ld hl,BUF_SEEKER_000
    ld b,32
@load_frames:
    push bc
    push hl
    call vdu_sprite_add_buff
    pop hl
    inc hl
    pop bc
    djnz @load_frames
; copy coordinates of active sprite to new sprite
    ld iy,(table_pointer) ; TODO: see if we can get IY to land here with the proper value
    ; ld hl,(iy+sprite_x)
    ; ld hl,0x008000 ; debug
    
    call rand_8
    ld hl,0
    ld h,a

    ld (@x),hl
    ; ld hl,(iy+sprite_y)
    ; ld hl,0x002000 ; debug

    call rand_8
    ld hl,0
    ld h,a

    ld (@y),hl
    call rand_8
    and %00000001 ; 50/50 chance of moving left or right on spanw
    ld (@move_step),a 
; now copy to the table
    ld hl,@id ; address to copy from
    pop de ; address to copy to (was ix)
    ld bc,table_bytes_per_record ; number of bytes to copy
    ldir ; copy the records from local scratch to sprite table
; set collision flags
    set collides_with_laser,(iy+sprite_collisions)
    set collides_with_player,(iy+sprite_collisions)
; update the new sprite's position and make it visible
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    call vdu_sprite_move_abs168
    call vdu_sprite_show
    ret
@id:                    db 0x00 ; 1 bytes unique spriteId, zero-based
@type:                  db enemy_seeker ; 1 bytes type of sprite as defined in enemies.inc
@base_bufferId:         dl BUF_SEEKER_000 ; 3 bytes bitmap bufferId
@move_program:          dl move_enemy_seeker ; 3 bytes address of sprite's behavior subroutine
@collisions:            db 0 ; 3 bytes, set by spawing routine
@dim_x:                 db 16 ; 1 bytes sprite width in pixels
@dim_y:                 db 16 ; 1 bytes sprite height in pixels
@x:                     dl 0x000000 ; 1 bytes 16.8 fractional x position in pixels
@y:                     dl 0x000000 ; 3 bytes 16.8 fractional y position in pixels
@xvel:                  dl 0x000000 ; 3 bytes x-component velocity, 16.8 fixed, pixels
@yvel:                  dl 0x000000 ; 3 bytes y-component velocity, 16.8 fixed, pixels
@vel:                   dl speed_seeker ; 3 bytes velocity, 16.8 fixed, pixels 
@heading:               dl 0x008000 ; 3 bytes sprite movement direction deg256 16.8 fixed
@orientation:           dl 0x008000 ; 3 bytes orientation bits
@animation:             db 0x00 ; 1 bytes current animation index, zero-based
@animation_timer:       db 0x00 ; 1 bytes when hits zero, draw next animation
@move_timer:            db 0x01 ; 1 bytes when zero, go to next move program, or step
@move_step:             db 0x00 ; 1 bytes stage in a move program sequence, varies
@points:                db 10 ; 1 bytes points awarded for killing this sprite type
@shield_damage:         db 0x02 ; 1 bytes shield points deducted for collision


; move_fireballs:
;     call move_sprite ; move sprite 
;     ld a,(sprite_screen_edge) ; check for collision with screen edge
;     and a ; if zero we're still within screen bounds
;     jp z,move_fireballs_alive
; ; otherwise kill sprite
;     ld a,%10000000 ; any bit set in high nibble means sprite will die
;     ld (iy+sprite_collisions),a
;     ret
; move_fireballs_alive:
;     ld a,(sprite_animation_timer)
;     dec a
;     ld (sprite_animation_timer),a
;     jp nz,move_fireballs_draw
;     ld a,(sprite_animation)
;     xor %1
;     ld (sprite_animation),a
;     ld a,6 ; 1/10th of a second timer
;     ld (sprite_animation_timer),a
;     ; fall through

; move_fireballs_draw:
;     call vdu_bmp_select ; TODO: convert to vdu_buff_select
;     call vdu_bmp_draw ; convert to vdu_bmp_plot
;     call check_collisions
;     ret

; compute orientation to player 
; based on relative positions
; returns: h.l 16.8 fixed angle256 to player
;    ub.c and ud.e as 16.8 signed fixed point numbers
;    representing delta-x/y *to* target respectively
orientation_to_player:
    ld iy,(table_pointer) ; TODO: see if we can get IY to land here with the proper value
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    ld ix,(player_x)
    ld iy,(player_y)
    call dxy168
    push bc
    push de
    call atan2_168fast
    pop de
    pop bc
    ld iy,(table_pointer) ; TODO: see if we can get IY to land here with the proper value
    ret

; this routine vanquishes the enemy sprite
; and replaces it with an animated explosion
; we jump here instead of call because
; we want to return to differing locations in the loop
; depending on whether we're still sploding
; destroys: everything except index registers
; returns: an incandescent ball of debris and gas
enemy_kill:
    ld iy,(table_pointer); TODO: see if we can get IY to land here with the proper value
; tally up points
    ld hl,(player_score)
    ld bc,0 ; clear bcu
    ld c,(iy+sprite_points)
    add hl,bc
    ld (player_score),hl
; ; initialize explosion
; init_explosion:
;     ld hl,explosion
;     ld (sprite_base_bufferId),hl
;     ld hl,move_explosion
;     ld (sprite_move_program),hl
;     ld a,%00000000 ; collides with nothing
;     ld (iy+sprite_collisions),a
;     ld hl,0 ; north
;     ld (sprite_heading),hl
;     ld a,0x04 ; will decrement to 03
;     ld (sprite_animation),a
;     ld a,0x07 ; 7/60th of a second timer
;     ld (sprite_animation_timer),a
;     xor a
;     ld (sprite_move_timer),a
;     call vdu_bmp_select ; TODO: convert to vdu_buff_select
; ; fall through to next_explosion
; next_explosion:
;     ld a,(sprite_animation)
;     dec a ; if rolled negative from zero,we're done sploding
;     jp m,done_explosion
;     ld (sprite_animation),a
;     ld a,0x7 ; 7/60th of a second timer
;     ld (sprite_animation_timer),a
; ; fall through to animate_explosion
; animate_explosion:
;     ld hl,sprite_y+1
;     inc (hl) ; move explosion down 1 pixel
;     jp z, done_explosion ; if wraparound to top of screen, kill explosion
;     ld hl,sprite_animation_timer
;     dec (hl) ; if timer is zero,we do next animation
;     jp z,next_explosion
;     ;otherwise we fall through to draw the current one
;     call vdu_bmp_select ; TODO: convert to vdu_buff_select
;     call vdu_bmp_draw ; convert to vdu_bmp_plot
;     ret ; now we go back to caller
; done_explosion:
    set sprite_just_died,(iy+sprite_collisions)
; TODO: remove this once the generic logic for this has been reimplemented
    ld hl,(ix+sprite_id)
    call vdu_sprite_select
    call vdu_sprite_hide
; END TODO
    ret ; now we go back to caller

; game_over:
;     jp game_initialize
