seekers_count: db 0
seekers_max: equ 230
speed_seeker: equ 0x000280 ; 2.5 pixels per frame

seeker_defaults:
    db enemy_air            ; type
    dl BUF_SEEKER_000       ; base_bufferId
    dl move_enemy_seeker_random    ; move_program
    db %01000000            ; collisions bit 6 signals sprite alive
    db 16                   ; dim_x
    db 16                   ; dim_y
    db 32                   ; num_orientations
    db 1                    ; num_animations
    db 0                    ; animation
    db 0                    ; animation_timer
    db 1                    ; move_timer
    db 0                    ; move_step
    db 10                   ; points
    db -4                   ; shield_damage
    db 0                    ; frame

; DEBUG
; text coordinates for seeker display
seeker_x: equ 39
seeker_y: equ 2
num_seekers: db 0
; inputs: none
print_seeker:
; set text colors
    ld a,c_black+128
    call vdu_colour_text
    ld a,c_green
    call vdu_colour_text
; print the seeker
    ld c,seeker_x
    ld b,seeker_y
    call vdu_move_cursor
    ld a,(num_seekers)
    call printHexA
    ret
; end print_seeker
; END DEBUG

enemy_spawn_seeker:
    ld hl,seeker_defaults
    ld ix,seekers_base
    ld b,seekers_max
    call spawn_sprite_from_tile
    ret nc ; no carry means no free sprite slots, so we go home
; other specific initialization
    ld hl,speed_seeker
    ld (ix+sprite_vel),hl
; DEBUG
    ld hl,num_seekers
    inc (hl)
    call print_seeker
    call @rand
    ld (ix+sprite_xvel),hl
    call @rand
    ld (ix+sprite_yvel),hl
    ret
@rand:
    ld hl,0
    call rand_8
    ld b,a
    and %00000001
    ld h,a
    ld l,b
    bit 7,b
    ret z
    call neg_hlu
    ret
; END DEBUG
    ret
; end enemy_spawn_seeker

; DEBUG
move_enemy_seeker_random:
    call move_sprite
    ret
; END DEBUG

move_enemy_seeker_orbit:
; compute x coordinate of orbit origin based on sprite id
    ld a,seeker_min_sprite_id
    sub (iy+sprite_id)
    ld l,a
    ld h,85 ; 1/3 the playing field width
    mlt hl
    ld a,64 ; 1/4 of the playing field width
    add a,l ; a = x coordinate of orbit origin
    ld (iy+sprite_move_step),a ; as good a place as any to store it
; initialize orbit angle
    xor a
    ld (iy+sprite_orientation),a
; ; initialize move timer
;     inc a ; set to 1
;     ld (iy+sprite_move_timer),a
; init done, so update move program
    ld hl,@stage1 ; update move program pointer
    ld (iy+sprite_move_program),hl
; fall through to @stage1
@stage1:
; ; check whether time to update orbit angle
;     dec (iy+sprite_move_timer)
;     jp nz,@F ; not time to change so simply movethe sprite on the current heading
;     inc (iy+sprite_move_timer) ; reset timer to 1
; update the target coordinates based on orbit angle and radius
    inc (iy+sprite_orientation) ; increment angle
    ld hl,0 ; clear hlu and l
    ld h,(iy+sprite_orientation) ; uh.l = angle
    ld de,128*256 ; ud.e = radius
    call polar_to_cartesian ; u.bc, u.de = x,y offsets
    ld a,(iy+sprite_move_step) ; x coordinate of orbit origin
    add a,b ; add x offset
    ld bc,0 ; bcu and c
    ld b,a ; ub.c = x
    ld hl,336/2*256 
    add hl,de
    ex de,hl ; ud.e = y
    call bearing_to_coord ; uh.l = angle to coordinate
    ld (iy+sprite_heading),hl ; save angle as heading
; compute component velocities, save them in sprite record, and move the sprite
    call sprite_compute_velocities
@@:
    call move_sprite ; move the sprite
    jp move_enemy_seeker
; end move_enemy_seeker_orbit

move_enemy_seeker:
    ld ix,player_begin
    bit sprite_alive,(ix+sprite_collisions)
    jp z,@F ; player is dead so don't check for collisions
    call check_enemy_collision_player
    jp z,@player
@@:
    call check_enemy_collision_laser
    jp z,@laser
; change the animation frame to match heading
    ld a,(iy+sprite_id)
    call vdu_sprite_select
    call sprite_frame_heading ; a, sprite_frame set to heading-based frame
    call vdu_sprite_select_frame
    ret
@laser:
    call update_score
    jp spawn_explosion
@player:
    call update_shields
    jp spawn_explosion
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
    call move_sprite ; move the sprite
    or a ; a will be non-zero if have collided with screen edge
    jp nz,@zag_switch ; switch direction
    jp move_enemy_seeker ; didn't hit an edge so check collisions and move sprite
@zig_zag_start:
    ld iy,(table_pointer) 
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
@zag_switch:
    ; otherwise flip direction and restart timer
    ld a,(iy+sprite_move_step)
    xor %1 ; flips bit zero
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


init_enemy_seekers:
; set pointer to seeker sprite table
    ld iy,seekers_base
    ld a,(iy+sprite_id) ; sprite id
    ld e,seekers_max ; sprite loop counter
; define seeker sprites
@sprites_loop:
    push af
    push de
	call vdu_sprite_select
	call vdu_sprite_clear_frames
	ld hl,BUF_SEEKER_000
	ld b,32 ; 32 bitmaps for seeker
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
; end init_enemy_seekers