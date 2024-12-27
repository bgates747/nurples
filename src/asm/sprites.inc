; sprite_type
enemy_dead: equ 0
enemy_air: equ 1
enemy_ground: equ 2
enemy_projectile: equ 3
explosion: equ 4
player: equ 5
player_air: equ 6
player_ground: equ 7

; ###### SPRITE TABLE FIELD INDICES ######
table_record_size:         equ  64; 40 bytes ;40 bytes per sprite record
sprite_id:                 equ   0; 1 bytes ;unique spriteId, zero-based
; sprite state variables
sprite_state_variables:    equ   1; beginning of sprite state variables
sprite_x:                  equ   1; 3 bytes ;16.8 fractional x position in pixels
sprite_y:                  equ   4; 3 bytes ;16.8 fractional y position in pixels
sprite_xvel:               equ   7; 3 bytes ;x-component velocity, 16.8 fixed, pixels
sprite_yvel:               equ  10; 3 bytes ;y-component velocity, 16.8 fixed, pixels
sprite_vel:                equ  13; 3 bytes ;velocity px/frame (16.8 fixed)
sprite_heading:            equ  16; 3 bytes ;sprite movement direction deg256 16.8 fixed
sprite_orientation:        equ  19; 3 bytes ;orientation bits
; sprite definition variables
sprite_definitions:        equ  22; beginnning of sprite definition variables
sprite_type:               equ  22; 1 bytes ;type of sprite as defined in enemies.inc
sprite_base_bufferId:      equ  23; 3 bytes ;bitmap bufferId
sprite_move_program:       equ  26; 3 bytes ;address of sprite's behavior subroutine
sprite_collisions:         equ  29; 1 bytes ;see collisions.inc constants for bit definitions
sprite_dim_x:              equ  30; 1 bytes ;sprite width in pixels
sprite_dim_y:              equ  31; 1 bytes ;sprite height in pixels
sprite_num_orientations:   equ  32; 1 bytes ;number of orientations for this sprite
sprite_num_animations:     equ  33; 1 bytes ;number of animations for this sprite
sprite_animation:          equ  34; 1 bytes ;current animation index, zero-based
sprite_animation_timer:    equ  35; 1 bytes ;when hits zero, draw next animation
sprite_move_timer:         equ  36; 1 bytes ;when zero, go to next move program, or step
sprite_move_step:          equ  37; 1 bytes ;stage in a move program sequence, varies
sprite_points:             equ  38; 1 bytes ;points awarded for killing this sprite type
sprite_shield_damage:      equ  39; 1 bytes ;shield points deducted for collision
sprite_frame:              equ  40; 1 bytes ;current animation frame

; ###### SPRITE TABLE VARIABLES ######
; maximum number of sprites
table_max_records: equ 16 ; it can handle more but this is pushing it
; table_total_bytes: equ table_end-table_start_variables

; pointer to top address of current record, initialized to table_base
table_pointer: dl table_base

; flag indicating collision with screen edge
; uses orientation codes to specify which edge(s)
sprite_screen_edge: db #00 

; origin_top: equ 48
origin_top: equ 0
origin_left: equ 128
field_top: equ 0
field_bottom: equ 383-origin_top
field_left: equ 0
field_right: equ 255
sprite_top: equ field_top + 0
sprite_bottom: equ field_bottom-16
sprite_left: equ field_left
sprite_right: equ field_right-16

tile_bottom: equ 384

; ######### COLLISION SPRITE PARAMETERS ##########
; integer coordinates are all that are needed for collision calculations
collision_x: db 0x00 
collision_y: db 0x00
collision_dim_x: db 0x00
collision_dim_y: db 0x00

; scratch variables
x: db 0x00 ; 8-bit signed integer
y: db 0x00 ; 8-bit signed integer
x0: dl 0x000000 ; 16.8 signed fixed place
y0: dl 0x000000 ; 16.8 signed fixed place
incx1: dl 0x000000 ; 16.8 signed fixed place
incy1: dl 0x000000 ; 16.8 signed fixed place
incx2: dl 0x000000 ; 16.8 signed fixed place
incy2: dl 0x000000 ; 16.8 signed fixed place

; sprite_heading: dl 0x000000 ; signed fixed 16.8 
radius: dl 0x000000 ; signed fixed 16.8 (but should always be positive)
sin_sprite_heading: dl 0x000000 ; signed fixed 16.8
cos_sprite_heading: dl 0x000000 ; signed fixed 16.8

table_init:
; clear the sprite table
    xor a ; write zeroes to sprite table
    ld hl,table_base ; address to copy from
    ld bc,table_end-table_base ; number of bytes to clear
    call clear_mem
; write sprite ids
    ld ix,table_base
    xor a ; sprite id
    ld c,0 ; sprite type
    ld b,table_num_records ; loop counter
@loop:
    ld (ix+sprite_id),a
    ld (ix+sprite_type),c
    lea ix,ix+table_record_size
    inc a
    djnz @loop
    ret
; end table_init

; deactivate the sprite with the given id
; inputs: iy set to sprite record
; outputs: nothing
; destroys: a,hl
table_deactivate_sprite:
    ld a,(iy+sprite_id) ; get spriteId
    call vdu_sprite_select ; select sprite 
    call vdu_sprite_hide
    res sprite_alive,(iy+sprite_collisions)
    res sprite_just_died,(iy+sprite_collisions)
    ret
; end table_deactivate_sprite

sprites_init:
; initialize sprites
	call vdu_sprite_reset ; out of an abundance of caution (copilot: and paranoia)
	xor a ; loop counter / sprite id
@sprite_loop:
	push af ; save loop counter / sprite id
	call vdu_sprite_select
	ld hl,BUF_CRATER ; can be anything, but why not a smoking hole in the ground?
	call vdu_sprite_add_buff
	pop af ; restore loop counter / sprite id
    inc a
    cp table_num_records
    jp nz,@sprite_loop
	call vdu_sprite_activate

; define player sprite
	ld a,(player_begin+sprite_id)
	call vdu_sprite_select
	call vdu_sprite_clear_frames
	ld hl,BUF_SHIP_0L
	ld bc,3 ; three bitmaps for player ship
@sprite_player_loop:
	push bc
	push hl
	call vdu_sprite_add_buff
	pop hl
	inc hl
	pop bc
	djnz @sprite_player_loop

; define player weapon sprites
    call init_player_laser

; define enemy sprites
    call init_enemy_seekers
    call init_enemy_fireballs

; define explosion sprites
    call init_explosions
    ret
; end sprites_init

; gets the next available sprite id for a particular sprite group
; inputs; ix = pointer to sprite group, b = number of records in sprite group
; returns: if new sprite available, a = sprite id, ix pointing to new sprite vars, carry set
;      otherwise, a = 0, carry flag reset, ix pointing to highest sprite vars
; destroys: a,b,c,hl,ix
table_get_next_id:
@loop:
    bit sprite_alive,(ix+sprite_collisions)
    jr z,@found
    lea ix,ix+table_record_size
    djnz @loop
@notfound:
    xor a ; a = 0 and reset carry flag indicating that we didn't find a free sprite
    ret
@found:
; return sprite id
    ld a,(ix+sprite_id)
    scf ; sets carry flag indicating we found a free sprite
    ret ; done
; end table_get_next_id

; create a new sprite at the prescribed coordinates
; inputs: hl = pointer to new sprite default settings, ub.c, ud.e = x,y position 
;         ix = pointer to sprite group, a = number of records in sprite group
; returns: if new sprite available, a = sprite id, ix pointing to new sprite vars, carry set
;      otherwise, a = 0, carry flag reset, ix pointing to highest sprite vars
spawn_sprite_from_coords:
    ld (@sprite_x),bc ; store x-coordinate
    ld (@sprite_y),de ; store y-coordinate
    ld b,a ; number of records in sprite group
    push hl ; back up pointer to new sprite defaults
    call table_get_next_id ; if available: a = new sprite id, ix = pointer to new sprite vars
    pop hl ; restore pointer to new sprite defaults
    ret nc ; no carry means no free sprite slots, so we go home
; copy default settings to new sprite
    push ix ; back up pointer to new sprite record
    lea ix,ix+sprite_definitions ; bump pointer to copy to
    push ix ; pointer to default sprite settings
    pop de ; address to copy to
    ld bc,table_record_size-sprite_definitions ; number of bytes to copy
    ldir ; copies defaults to new sprite record
    pop ix ; restore pointer to new sprite record
; copy sprite position from existing sprite
    ld hl,(@sprite_x)
    ld (ix+sprite_x),hl
    ld hl,(@sprite_y)
    ld (ix+sprite_y),hl
; return to caller
    set 7,(ix+sprite_type) ; set just spawned flag
    set sprite_alive,(ix+sprite_collisions) ; set sprite alive flag
    scf ; set carry flag to indicate success
    ret
@sprite_x: dl 0 ; new sprite x-coordinate
@sprite_y: dl 0 ; new sprite y-coordinate
; end spawn_sprite_from_coords

; create a new sprite at the same location as an existing sprite
; inputs: hl = pointer to new sprite default settings, table_pointer = existing sprite
;         ix = pointer to sprite group, b = number of records in sprite group
; returns: if new sprite available, a = sprite id, ix pointing to new sprite vars, carry set
;      otherwise, a = 0, carry flag reset, ix pointing to highest sprite vars
spawn_sprite_from_sprite: 
    push hl ; back up pointer to new sprite defaults
    call table_get_next_id ; if available: a = new sprite id, ix = pointer to new sprite vars
    pop hl ; restore pointer to new sprite defaults
    ret nc ; no carry means no free sprite slots, so we go home
; copy default settings to new sprite
    push ix ; back up pointer to new sprite record
    lea ix,ix+sprite_definitions ; bump pointer to copy to
    push ix ; pointer to default sprite settings
    pop de ; address to copy to
    ld bc,table_record_size-sprite_definitions ; number of bytes to copy
    ldir ; copies defaults to new sprite record
    pop ix ; restore pointer to new sprite record
; copy sprite position from existing sprite
    ld iy,(table_pointer) ; get existing sprite record
    ld hl,(iy+sprite_x)
    ld (ix+sprite_x),hl
    ld hl,(iy+sprite_y)
    ld (ix+sprite_y),hl
; return to caller
    set 7,(ix+sprite_type) ; set just spawned flag
    set sprite_alive,(ix+sprite_collisions) ; set sprite alive flag
    scf ; set carry flag to indicate success
    ret
; end spawn_sprite_from_sprite

; call the behavior subroutines for a specified sprite group
; inputs: iy = pointer to sprite group, b = number of records in sprite group
move_sprite_group:
    push bc ; backup loop counter
    push iy ; backup table pointer
@move_loop:
    ld (table_pointer),iy ; update table pointer
    push bc ; backup loop counter
; check top bit of sprite_type to see if sprite is just spawned
    bit 7,(iy+sprite_type)
    jp nz,@next_record ; if just spawned, skip to next record
; check sprite_just_died bit
    bit sprite_just_died,(iy+sprite_collisions)
    jp nz,@sprite_died ; if just died, deactivate sprite
; check sprite_alive bit to see if sprite is active
    bit sprite_alive,(iy+sprite_collisions)
    jp z,@next_record ; ... skip to next record
; otherwise move the sprite
    ld hl,(iy+sprite_move_program) ; load the behavior subroutine address
    CALL_HL ; run the behavior subroutine
    ld a,(iy+sprite_id) ; get spriteId
    call vdu_sprite_select ; select sprite 
    ld a,(iy+sprite_frame)
    call vdu_sprite_select_frame
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
    pop iy ; get back table pointer
    pop bc ; get back loop counter
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
@sprite_died:
    call table_deactivate_sprite
    jp @next_record
; end move_sprite_group

; compute a sprite's new position based on its component x,y velocities
; with screen boundary checking and update its sprite table record accordingly
; inputs: table_pointer set to sprite record
; returns: updated x, y positions and edge collision flags in sprite_screen_edge
; destroys: everything
; note: actual rendering of sprite must be handled by caller
move_sprite:
    ld iy,(table_pointer) 
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
    xor a ; reset screen edge collision flag and clear carry
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
; end move_sprite

sprite_compute_velocities:
    ld hl,(iy+sprite_heading) 
    ld de,(iy+sprite_vel) 
    call polar_to_cartesian
    ld iy,(table_pointer) 
    ld (iy+sprite_xvel),bc ; save x-velocity component
    ld (iy+sprite_yvel),de ; save y-velocity component 
    ret
; end sprite_compute_velocities

; compute animation frame based on sprite_animation and sprite_heading
; inputs: table_pointer set to sprite record
; outputs: a, sprite_frame set to animation frame
; destroys: a,hl
sprite_frame_heading_animation:
    ld a,(iy+sprite_heading+1) ; we want the integer portion
    srl a ; divide integer portion by 8
    srl a
    srl a
    ld h,(iy+sprite_animation)
    ld l,32 ; 32 orientation frames per animation
    mlt hl ; hl = offset from base animation
    add a,l ; modulo256 since sprite_frame is only 1 byte
    ld (iy+sprite_frame),a
    ret
; end sprite_frame_heading_animation

; compute animation frame based on sprite_heading
; inputs: table_pointer set to sprite record
; outputs: a, sprite_frame set to animation frame
; destroys: a,hl
sprite_frame_heading:
    ld a,(iy+sprite_heading+1) ; we want the integer portion
    srl a ; divide integer portion by 8
    srl a
    srl a
    ld (iy+sprite_frame),a
    ret
; end sprite_frame_heading

; load a sprite with animation bitmaps
; inputs: a = sprite id, hl = base bufferId, bc = number of frames, sprite_select
sprite_load_frames:
; back up input parameters
    push hl ; base bufferId
    push bc ; bc = number of frames
    call vdu_sprite_clear_frames
    pop bc ; bc = number of frames
    pop hl ; = base bufferId
@load_frames:
    push bc ; back up loop counter
    push hl ; back up base bufferId
    call vdu_sprite_add_buff
    pop hl ; restore base bufferId
    inc hl ; bump it to the next frame
; decrement loop counter
    pop bc
    dec bc
    ld a,c
    or a
    jp nz,@load_frames
    ld a,b
    or a
    jp nz,@load_frames
    ret
; end sprite_load_frames