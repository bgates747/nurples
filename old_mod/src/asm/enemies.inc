max_enemy_sprites: db 16 

; sprite_type
enemy_dead: equ 0
enemy_small: equ 1
enemy_medium: equ 2
enemy_large: equ 3
landing_pad: equ 4
laser_turret: equ 5
fireballs: equ 6
explosion: equ 7


respawn_countdown:
    ld hl,(respawn_timer)
    dec hl
    ld (respawn_timer),hl
; check hl for zero
    add hl,de
    or a
    sbc hl,de 
    ret nz
    ld b,table_max_records
@respawn_loop:
    push bc
    call enemy_init_from_landing_pad
    pop bc
    djnz @respawn_loop
    ld hl,1*60 ; 1 second
    ld (respawn_timer),hl
    ret 
respawn_timer: dl 1*60

move_enemies:
; are there any active enemies or explosions?
    ld hl,0
    ld a,(table_active_sprites)
    ld l,a
    ; call dumpRegistersHex
    and a ; will be zero if no alive enemies or explosions
    ; ret z ; so nothing to do but go back
    ; ld hl,(respawn_timer)
    ; call dumpRegistersHex
    jr nz,move_enemies_do
    call respawn_countdown
    ret
move_enemies_do:
; initialize pointers and loop counter
    ld iy,table_base ; set iy to first record in table
    ld b,table_max_records ; loop counter
move_enemies_loop:
    ld (table_pointer),iy ; update table pointer
    push bc ; backup loop counter
; check sprite_type to see if sprite is active
    ld a,(iy+sprite_type)
    and a ; if zero, sprite is dead 
    jr z,move_enemies_next_record ; ... and we skip to next record
; otherwise we prepare to move the sprite
    ld a,(iy+sprite_id) ; get spriteId
    call vdu_sprite_select ; select sprite 
    ld hl,(iy+sprite_move_program) ; load the behavior subroutine address
    jp (hl)  ; ... and jump to it
; we always jp back here from behavior subroutines
move_enemies_loop_return:
    ld iy,(table_pointer) ; get back table pointer
; now we check results of all the moves
    ld a,(iy+sprite_collisions)
    and %11110000 ; any bits set in high nibble means we died
    ld a,(iy+sprite_id) ; get spriteId for the deactivate_sprite call if needed
    jr z,move_enemies_draw_sprite ; if not dead,draw sprite
    call table_deactivate_sprite ; otherwise we ded
    xor a ; zero a so that we can ...
    ld (iy+sprite_collisions),a ; ... clear collision flags
    jr move_enemies_next_record ; and to the next record
move_enemies_draw_sprite:
; if we got here sprite will have already been activated
; so all we need to do is set its coordinates and draw it
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    call vdu_sprite_move_abs168
; fall through to next record
move_enemies_next_record:
    ld de,table_bytes_per_record
    add iy,de ; point to next record
    xor a ; clears carry flag
    ld (sprite_screen_edge),a ; clear screen edge collision flag
    pop bc ; get back our loop counter
    djnz move_enemies_loop ; loop until we've checked all the records
    ret ; and we're out

en_nav_zigzag_start:
    ld iy,(table_pointer) ; TODO: see if we can get IY to land here with the proper value
    call rand_8
    and %00111111 ; limit it to 64
    set 3,a ; make sure it's at least 8
    ld (iy+sprite_move_timer),a ; store it
    ; fall through to en_nav_zigzag
en_nav_zigzag:
    ld a,(iy+sprite_move_timer)
    dec a
    ld (iy+sprite_move_timer),a
    jr nz,en_nav_zigzag_no_switch
    ; otherwise flip direction and restart timer
    ld a,(iy+sprite_move_step)
    xor %1 ; flips bit one
    ld (iy+sprite_move_step),a ; store it
    jr nz,en_nav_zigzag_right
;otherwise zag left
    ld hl,0x00A000; southwest heading
    ld (iy+sprite_heading),hl ; save sprite heading
    jr en_nav_zigzag_start
en_nav_zigzag_right:
    ld hl,0x006000; southeast heading
    ld (iy+sprite_heading),hl ; save sprite heading
    jr en_nav_zigzag_start
en_nav_zigzag_no_switch:
    ; ld a,(sprite_orientation)
    ld hl,(iy+sprite_heading)
    jr en_nav_computevelocities

; contains the logic for how to move the enemy
; and then does the moving
; inputs: a fully-populated active sprite table
;         player position variables
; destroys: everything except index registers
; outputs: moving enemies
en_nav:
; set velocity and orientation by player's relative location
; move enemies y-axis
; where is player relative to us?
    call orientation_to_player ; uh.l angle to player, ub.c, ud.e = dx, dy
; is player above or below us?
    ld (ude),de ; dy
    ld a,(ude+2) ; deu
    rla ; shift sign bit into carry
    jr nc,en_nav_zigzag ; player is below,evade
; player is even or above,so home in on current heading
    ld (iy+sprite_heading),hl ; save sprite heading

; we land here from zig-zag program so as not to 
; redundantly save orientation and heading
en_nav_computevelocities:
; set x/y component velocities based on bearing to player
    ld iy,(table_pointer) ; TODO: see if we can get IY to land here with the proper value
    push hl ; we need it back to set rotation frame
    ld de,(iy+sprite_vel) 
    call polar_to_cartesian
    ld iy,(table_pointer) ; TODO: see if we can get IY to land here with the proper value
    ld (iy+sprite_xvel),bc ; save x-velocity component
    ld (iy+sprite_yvel),de ; save y-velocity component 
; change the animation frame to match heading
; by dividng the heading by 8
    pop hl ; get back Heading
    ld a,h
    srl a
    srl a
    srl a
    call vdu_sprite_select_frame

move_enemy_sprite:
    ld iy,(table_pointer) ; TODO: see if we can get IY to land here with the proper value
; x-axis movement
    ld hl,(iy+sprite_x)
    ld de,(iy+sprite_xvel)
    add hl,de
    ld (iy+sprite_x),hl
; y-axis movement
    ld hl,(iy+sprite_y)
    ld de,(iy+sprite_yvel)
    add hl,de
    ld (iy+sprite_y),hl
; detect screen edge collisions
    xor a  ; reset screen edge collision flag and clear carry
    ld (sprite_screen_edge),a
; y-axis first since we already have it in hl
    ld de,-sprite_top*256
    adc hl,de
    jp m,@top
    sbc hl,de ; hl back to original value
    ld de,-sprite_bottom*256
    adc hl,de
    jp p,@bottom
@xaxis:
    ld hl,(iy+sprite_x)
    ld de,-sprite_left*256
    adc hl,de
    jp m,@left
    sbc hl,de ; hl back to original value
    ld de,-sprite_right*256
    adc hl,de
    jp p,@right
; no edge collisions so go home
    ret
@top:
    ld a,collide_top
    ld hl,sprite_top*256
    jp @collide_y
@bottom:
    ld a,collide_bottom
    ld hl,sprite_bottom*256
@collide_y:
    ld (sprite_screen_edge),a
    ld (iy+sprite_y),hl
; reverse y-axis velocity
    ld hl,(iy+sprite_yvel)
    call neg_hlu
    ld (iy+sprite_yvel),hl
; go check for x-axis collisions
    jp @xaxis
@left:
    ld a,(sprite_screen_edge)
    or a,collide_left
    ld hl,sprite_left*256
    jp @collide_x
@right:
    ld a,(sprite_screen_edge)
    or a,collide_right
    ld hl,sprite_right*256
@collide_x:
    ld (sprite_screen_edge),a
    ld (iy+sprite_x),hl
; reverse x-axis velocity
    ld hl,(iy+sprite_xvel)
    call neg_hlu
    ld (iy+sprite_xvel),hl
; final collision flag to a and return
    ld a,(sprite_screen_edge)
    ret

; ; ######### SPRITE BEHAVIOR ROUTINES #########
; ; each sprite in the table must have one of these defined
; ; but they need not be unique to a particular sprite
; ; these are jumped to from move_enemies_do_program,but could come from other places
; ; and have the option but not obligation to go back to move_enemies_loop_return
; ; but they can call anything they want between those two endpoints
; move_programs: ; bookmark in case we want to know the first address of the first subroutine

; move_nop: ; does nothing but burn a few cycles changing the PC
;     jp move_enemies_loop_return

; move_explosion:
;     call animate_explosion 
;     jp move_enemies_loop_return

move_enemy_small:
    call en_nav
    call check_collisions
    jp move_enemies_loop_return

; move_enemy_medium:
;     call en_nav
;     call check_collisions
;     jp move_enemies_loop_return

; move_enemy_large:
;     call en_nav
;     call check_collisions
;     jp move_enemies_loop_return

; move_landing_pad:
;     call move_active_tiles
;     call check_collisions
; ; is it time to launch an enemy?
;     ld hl,sprite_move_timer
;     dec (hl)
;     jp nz,move_enemies_loop_return
;     call enemy_init_from_landing_pad
;     ; reset move timer so can spawn again if player doesn't take us out
;     call rand_8     ; snag a random number
;     and %00011111   ; keep only 5 lowest bits (max 31)
;     add a,64 ; range is now 64-127
;     ld (sprite_move_timer),a ; when this hits zero,will spawn an enemy
;     jp move_enemies_loop_return

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
; finally, make the new sprite visible
    call vdu_sprite_show
    ret
@id:               db     0x00 ; 1 bytes unique spriteId, zero-based
@type:             db enemy_small ; 1 bytes type of sprite as defined in enemies.inc
@base_bufferId:    dl BUF_SEEKER_000 ; 3 bytes bitmap bufferId
@move_program:     dl move_enemy_small ; 3 bytes address of sprite's behavior subroutine
@collisions:       db %00000011 ; 3 bytes collides with enemy and laser
@dim_x:            db     0x10 ; 1 bytes sprite width in pixels
@dim_y:            db     0x10 ; 1 bytes sprite height in pixels
@x:                dl 0x000000 ; 1 bytes 16.8 fractional x position in pixels
@y:                dl 0x000000 ; 3 bytes 16.8 fractional y position in pixels
@xvel:             dl 0x000000 ; 3 bytes x-component velocity, 16.8 fixed, pixels
@yvel:             dl 0x000000 ; 3 bytes y-component velocity, 16.8 fixed, pixels
@vel:              dl speed_seeker ; 3 bytes velocity, 16.8 fixed, pixels 
@heading:          dl 0x008000 ; 3 bytes sprite movement direction deg256 16.8 fixed
@orientation:      dl 0x008000 ; 3 bytes orientation bits
@animation:        db     0x00 ; 1 bytes current animation index, zero-based
@animation_timer:  db     0x00 ; 1 bytes when hits zero, draw next animation
@move_timer:       db     0x01 ; 1 bytes when zero, go to next move program, or step
@move_step:        db     0x00 ; 1 bytes stage in a move program sequence, varies
@points:           db     0x20 ; 1 bytes points awarded for killing this sprite type, BCD
@shield_damage:    db     0x02 ; 1 bytes shield points deducted for collision, binary

; move_laser_turret:
; ; compute orientation to player
;     call orientation_to_player
; ; h.l 8.8 fixed angle256 to player
; ; bc and de as signed 16-bit integers
; ; representing delta-x/y *to* target respectively
;     ld (Bearing_t),hl
;     ld hl,0x0400
;     ld (Vp),hl
;     call targeting_computer
;     ld (sprite_heading),hl ; store bearing to player
; ; is it time to launch a fireball?
;     ld hl,sprite_move_timer
;     dec (hl)
;     jp nz,move_laser_turret_boilerplate
;     call fireballs_init
;     ; reset move timer so can fire again if player doesn't take us out
;     call rand_8     ; snag a random number
;     and %00011111   ; keep only 5 lowest bits (max 31)
;     add a,64 ; range is now 64-127
;     ld (sprite_move_timer),a ; when this hits zero,will spawn a fireball
; move_laser_turret_boilerplate:
;     call move_active_tiles
;     call check_collisions
;     jp move_enemies_loop_return

; fireballs_init:
;     call sprite_variables_to_stack

;     ld hl,fireballs
;     ld (sprite_base_bufferId),hl

;     ld hl,move_fireballs
;     ld (sprite_move_program),hl 

;     ld a,%11 ; collides with laser and player
;     ; ld a,%10 ; collides with laser DEBUG
;     ld (iy+sprite_collisions),a

;     ld hl,(Vp)
;     ld (sprite_vel),hl
;     ld hl,(Vp_x)
;     ld (sprite_xvel),hl
;     ld hl,(Vp_y)
;     inc h ; account for ground movement
;     ld (sprite_yvel),hl

;     xor a ; zero a
;     ld (sprite_animation),a
;     ld (sprite_move_step),a
;     ld (sprite_move_timer),a

;     ld a,6 ; 1/10th of a second timer
;     ld (sprite_animation_timer),a

;     ld a,0x00 ; BCD
;     ld (sprite_points),a
;     ld a,1 ; binary
;     ld (sprite_shield_damage),a

;     call table_add_record ; plops that on the sprite stack for later
;     call sprite_variables_from_stack ; come back to where we started
;     ret

; move_fireballs:
;     call move_enemy_sprite ; move sprite 
;     ld a,(sprite_screen_edge) ; check for collision with screen edge
;     and a ; if zero we're still within screen bounds
;     jr z,move_fireballs_alive
; ; otherwise kill sprite
;     ld a,%10000000 ; any bit set in high nibble means sprite will die
;     ld (iy+sprite_collisions),a
;     jp move_enemies_loop_return
; move_fireballs_alive:
;     ld a,(sprite_animation_timer)
;     dec a
;     ld (sprite_animation_timer),a
;     jr nz,move_fireballs_draw
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
;     jp move_enemies_loop_return

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


; targeting_computer scratch variables
Bearing_t: dw #0000 ; 8.8 fixed
Heading_t: dw #0000 ; 8.8 fixed
Vp: dw #0000 ; 8.8 fixed
Vp_x: dw #0000 ; 8.8 fixed
Vp_y: dw #0000 ; 8.8 fixed
Vt: dw #0000 ; 8.8 fixed
Vt_x: dw #0000 ; 8.8 fixed
Vt_y: dw #0000 ; 8.8 fixed


; ; Inputs:   see scratch variables
; ; Note:     a call to orientation_to_player provides these inputs
; ; Outputs:  h.l is the 16.8 fixed firing angle256
; ;           b.c and d.e are the 16.8 fixed x,y component projectile velocities
; ; https://old.robowiki.net/cgi-bin/robowiki?LinearTargeting
; targeting_computer:
; ; compute target velocity from x,y component velocities
;     ld bc,(player_xvel) 
;     ld de,(player_yvel)
;     dec d ; account for vertical ground movement: b.c=player_xvel,d.e=player_yvel-1

;     call cartesian_to_polar ; b.c=Heading_t, d.e=Vt
;     ld (Heading_t),bc
;     ld (Vt),de

; ; compute Heading_t-Bearing_t
;     ld h,b
;     ld l,c
;     ld bc,(Bearing_t)
;     and a ; clear carry
;     sbc hl,bc ; h.l=Heading_t-Bearing_t

; ; compute sin(Heading_t-Bearing_t)
;     ld b,h
;     ld c,l
;     call sin_bc ; h.l=sin(Heading_t-Bearing_t)

; ; compute (Vt*sin(Heading_t-Bearing_t))
;     ex de,hl
;     ld bc,(Vt)
;     call BC_Mul_DE_88 ; h.l=(Vt*sin(Heading_t-Bearing_t))

; ; compute (Vt * sin(Heading_t-Bearing_t)) / Vp
;     ld b,h
;     ld c,l
;     ld de,(Vp)
;     call div_88 ; h.l=(Vt*sin(Heading_t-Bearing_t)) / Vp
; ; answer is in radians, convert to degrees256
;     ex de,hl
;     ld bc,#28BE ; 40.74=57.29578*256/360
;     call BC_Mul_DE_88 

; ; add lead angle to target bearing
;     ld de,(Bearing_t)
;     add hl,de ; h.l=lead angle+target bearing
;     push hl

; ; compute component projectile velocities
;     ld b,h
;     ld c,l
;     ld de,(Vp)
;     call polar_to_cartesian ; b.c=Vp_x, d.e=Vp_y

;     ld (Vp_x),bc
;     ld (Vp_y),de
;     pop hl ; h.l=lead angle+target bearing
;     ret

; this routine vanquishes the enemy sprite
; and replaces it with an animated explosion
; we jump here instead of call because
; we want to return to differing locations in the loop
; depending on whether we're still sploding
; destroys: everything except index registers
; returns: an incandescent ball of debris and gas
kill_nurple:
; ; tally up points
;     ld bc,0
;     ld a,(sprite_points)
;     ld e,a
;     ld d,0
;     ld hl,add_bcd_arg2
;     call set_bcd
;     ld hl,player_score
;     ld de,add_bcd_arg2
;     ld a,3 ; number of bytes to add
;     call add_bcd
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
;     jr z, done_explosion ; if wraparound to top of screen, kill explosion
;     ld hl,sprite_animation_timer
;     dec (hl) ; if timer is zero,we do next animation
;     jr z,next_explosion
;     ;otherwise we fall through to draw the current one
;     call vdu_bmp_select ; TODO: convert to vdu_buff_select
;     call vdu_bmp_draw ; convert to vdu_bmp_plot
;     ret ; now we go back to caller
; done_explosion:
    ld a,%10000000 ; high bit set is non-specific kill-me flag
    ld iy,(table_pointer); TODO: see if we can get IY to land here with the proper value
    ld (iy+sprite_collisions),a
    ret ; now we go back to caller

; game_over:
;     jp new_game

; it's presumed we've already checked that laser is alive
collision_enemy_with_laser:
    ld ix,(laser_x)
    ld iy,(laser_y)
    ld a,(laser_dim_x)
    sra a ; divide by 2
    push af ; we need this later
    jp collision_enemy

; it's presumed we've already checked that player is alive
collision_enemy_with_player:
    ld ix,(player_x)
    ld iy,(player_y)
    ld a,(player_dim_x)
    sra a ; divide by 2
    push af ; we need this later
    ; fall through to collision_enemy

; compute the distance between the two sprites' centers
; inputs: bc and de as y0,x0 and y1,x1 respectively
collision_enemy:
; back up iy because we need it as the sprite table pointer
    push iy
    ld iy,(table_pointer)
    ld hl,(iy+sprite_x)
    ld a,(iy+sprite_dim_x)
    sra a
    push af ; we need this later
    push hl
    pop bc ; bc = x0
    ld hl,(iy+sprite_y)
    ld a,(iy+sprite_dim_y)
    ex de,hl ; de = y0
    pop af ; TODO: srsly, this is the best way to do this?
    pop iy
    push af 
    call distance168

; ; subtract sum of radii from distance between centers
;     ld de,0
;     pop af ; radius of enemy sprite
;     ld e,a
;     pop af ; radius of player or laser sprite
;     add a,e
;     ld e,a
;     and a ; clear carry
;     sbc hl,de
;     jr c,collision_enemy_is
;     xor a
;     ret
; temp fix TODO: remove this
    pop af
    pop af
    ld de,16*256
    and a
    sbc hl,de
    jr c,collision_enemy_is
    xor a
    ret
collision_enemy_is:
    xor a
    inc a
    ret

; ; looks up what enemy sprite collides with
; ; detects collisions
; ; and sets things to sploding accordingly
; check_collisions:
;     ld a,(iy+sprite_collisions) ; snag what we collide with
;     and a ; if this is zero,
;     ret z ; there's nothing to do
;     and %01 ; do we collide with player?
;     jr z,move_enemies_laser ; if not,check laser collision
;     call collision_enemy_with_player ; otherwise see if we hit player
;     and a ; was there a collision?
;     jr z,move_enemies_laser ; if not,see if laser smacked us
; ; yes collision with player
;     ; deduct shield damage
;     ld hl,sprite_shield_damage 
;     ld a,(player_shields) 
;     sub (hl)
;     ld (player_shields),a 
; ; if shields >= 0,player survives
;     jp p,check_collisions_kill_nurple
; ; otherwise update player status so it will die
;     ld a,(player_collisions)
;     or %10 ; sets bit 1,meaning player just died
;     ld (player_collisions),a
;     ; fall through
; check_collisions_kill_nurple:
; ; kill enemy and replace with explosion
;     call kill_nurple
;     ret ; and out

check_collisions:
    call collision_enemy_with_player ; did we hit the player?
    and a ; was there a collision?
    ret z ; if not,we're done
    call kill_nurple ; otherwise kill enemy
    ret

; did we hit the laser?
move_enemies_laser:
    ld a,(iy+sprite_collisions) ; snag what we collide with again
    and %10 ; do we even collide with laser?
    ret z ; if not,we're out
    ld a,(laser_collisions) ; is laser alive?
    and %1 ; if bit 0 is not set laser is dead
    ret z ; so we're out
    call collision_enemy_with_laser ; otherwise check for collision
    and a ; was there a collision?
    ret z ; if not,we're done
; otherwise we mark laser for termination and kill enemy
; update laser status so it will die
    ld a,(laser_collisions)
    or %10 ; bit 1 set means laser just died
    ld (laser_collisions),a
    call kill_nurple ; yes there was a collision,so kill enemy
    ret ; we're outta' here