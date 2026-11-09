seeker_state_seek: equ 0
seeker_state_orbit: equ 1
seeker_custom_begin: equ actor_record_custom
seeker_state: equ seeker_custom_begin
seeker_orbit_origin_x: equ seeker_state+1
seeker_orbit_angle: equ seeker_orbit_origin_x+1
seeker_custom_end: equ seeker_orbit_angle+1
    .if seeker_custom_end-seeker_custom_begin-3
        dl seeker_custom_span_mismatch
    .endif

seekers_count: db 0
seekers_max: equ 3
speed_seeker: equ 0x000280 ; 2.5 pixels per frame

seeker_defaults:
    db enemy_air            ; type
    dl BUF_SEEKER_000       ; base_bufferId
    dl en_nav_seek_zag    ; move_program
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

enemy_spawn_seeker:
    ld hl,seeker_defaults
    ld ix,seekers_base
    ld b,seekers_max
    call spawn_sprite_from_tile
    ret nc ; no carry means no free sprite slots, so we go home
; other specific initialization
    ld hl,speed_seeker
    ld (ix+sprite_vel),hl
    ld a,seeker_state_seek
    ld (ix+seeker_state),a
    ret
; end enemy_spawn_seeker

; Enter orbit through an actor-owned initializer. Publish the legal callback
; only after every field owned by the target state is ready.
; inputs: iy = validated live seeker record
; destroys: a,de,hl
seeker_enter_orbit:
; compute x coordinate of orbit origin based on sprite id
    ld a,seeker_min_sprite_id
    sub (iy+sprite_id)
    ld l,a
    ld h,85 ; 1/3 the playing field width
    mlt hl
    ld a,64 ; 1/4 of the playing field width
    add a,l ; a = x coordinate of orbit origin
    ld (iy+seeker_orbit_origin_x),a
; initialize orbit angle and local state
    xor a
    ld (iy+seeker_orbit_angle),a
    ld a,seeker_state_orbit
    ld (iy+seeker_state),a
; publish the initialized target behavior last
    ld hl,move_enemy_seeker_orbit_stage1
    ld (iy+sprite_move_program),hl
    scf
    ret
; end seeker_enter_orbit

; Compatibility entry for reviewed legacy callers. Stored callbacks use only
; the persistent stage below.
move_enemy_seeker_orbit:
    call seeker_enter_orbit
; fall through to the persistent orbit behavior
move_enemy_seeker_orbit_stage1:
; update the target coordinates based on orbit angle and radius
    inc (iy+seeker_orbit_angle)
    ld hl,0 ; clear hlu and l
    ld h,(iy+seeker_orbit_angle) ; uh.l = angle
    ld de,128*256 ; ud.e = radius
    call polar_to_cartesian ; u.bc, u.de = x,y offsets
    ld a,(iy+seeker_orbit_origin_x)
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

; Transition every live seeker without accepting an arbitrary callback or
; caller-supplied pool range.
seeker_group_enter_orbit:
    ld iy,seekers_base
    ld b,seekers_max
@loop:
    push bc
    call sprite_table_record_pointer_valid
    jp nc,@bad_record
    bit sprite_alive,(iy+sprite_collisions)
    jp z,@next
    ld a,(iy+sprite_type)
    and actor_type_mask
    cp enemy_air
    jp nz,@bad_record
    call seeker_enter_orbit
@next:
    lea iy,iy+table_record_size
    pop bc
    djnz @loop
    scf
    ret
@bad_record:
    ld hl,sprite_fault_flags
    set sprite_fault_bad_record,(hl)
    pop bc
    xor a
    ret
; end seeker_group_enter_orbit

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
    jp nz,@edge_switch ; switch direction, but never move twice in one frame
    jp move_enemy_seeker ; didn't hit an edge so check collisions and move sprite
@reset_zig_zag_timer:
    ld iy,(table_pointer) 
    call rand_8
    and %00111111 ; limit it to 64
    set 3,a ; make sure it's at least 8
    ld (iy+sprite_move_timer),a ; store it
    ret
@zig_zag:
    ld a,(iy+sprite_move_timer)
    dec a
    ld (iy+sprite_move_timer),a
    jp nz,@zig_zag_no_switch
@zag_switch:
    call @switch_zag_heading
    call @reset_zig_zag_timer
    jp @zig_zag_no_switch
@edge_switch:
; The generic mover has already clamped/bounced this seeker. Choose the next
; zag and return through collision handling; moving again here can loop forever
; at the bottom edge because both zag headings point downward.
    call @switch_zag_heading
    call @reset_zig_zag_timer
    jp move_enemy_seeker
@switch_zag_heading:
    ld a,(iy+sprite_move_step)
    xor %1 ; flips bit zero
    ld (iy+sprite_move_step),a ; store it
    jp nz,@right ; zag right
;otherwise zag left
    ld hl,160*256; southwest heading
    ld (iy+sprite_heading),hl ; save sprite heading
    ret
@right:
    ld hl,96*256; southeast heading
    ld (iy+sprite_heading),hl ; save sprite heading
    ret
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
