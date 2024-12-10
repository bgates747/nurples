; ###### SPRITE TABLE FIELD INDICES ######
table_bytes_per_record:    equ  41
sprite_id:                 equ   0 ; 1 bytes ; unique spriteId, zero-based
 ; sprite state variables
sprite_state_variables:    equ   1 ; beginning of sprite state variables
sprite_x:                  equ   1 ; 3 bytes ; 16.8 fractional x position in pixels
sprite_y:                  equ   4 ; 3 bytes ; 16.8 fractional y position in pixels
sprite_xvel:               equ   7 ; 3 bytes ; x-component velocity, 16.8 fixed, pixels
sprite_yvel:               equ  10 ; 3 bytes ; y-component velocity, 16.8 fixed, pixels
sprite_vel:                equ  13 ; 3 bytes ; velocity px/frame (16.8 fixed)
sprite_heading:            equ  16 ; 3 bytes ; sprite movement direction deg256 16.8 fixed
sprite_orientation:        equ  19 ; 3 bytes ; orientation bits
 ; sprite definition variables
sprite_definitions:        equ  22 ; beginnning of sprite definition variables
sprite_type:               equ  22 ; 1 bytes ; type of sprite as defined in enemies.inc
sprite_base_bufferId:      equ  23 ; 3 bytes ; bitmap bufferId
sprite_move_program:       equ  26 ; 3 bytes ; address of sprite's behavior subroutine
sprite_collisions:         equ  29 ; 1 bytes ; see collisions.inc constants for bit definitions
sprite_dim_x:              equ  30 ; 1 bytes ; sprite width in pixels
sprite_dim_y:              equ  31 ; 1 bytes ; sprite height in pixels
sprite_num_orientations:   equ  32 ; 1 bytes ; number of orientations for this sprite
sprite_num_animations:     equ  33 ; 1 bytes ; number of animations for this sprite
sprite_animation:          equ  34 ; 1 bytes ; current animation index, zero-based
sprite_animation_timer:    equ  35 ; 1 bytes ; when hits zero, draw next animation
sprite_move_timer:         equ  36 ; 1 bytes ; when zero, go to next move program, or step
sprite_move_step:          equ  37 ; 1 bytes ; stage in a move program sequence, varies
sprite_points:             equ  38 ; 1 bytes ; points awarded for killing this sprite type
sprite_shield_damage:      equ  39 ; 1 bytes ; shield points deducted for collision
sprite_frame:              equ  40 ; 1 bytes ; current frame of animation zero-based

; ###### SPRITE TABLE VARIABLES ######
; maximum number of sprites
table_max_records: equ 16 ; it can handle more but this is pushing it
table_total_bytes: equ table_max_records*table_bytes_per_record

; #### THIS IS THE SPACE ALLOCATED TO THE SPRITE TABLE ####
    align 256
table_base: 
sprite_start_variables:  blkb table_total_bytes, 0 ; fill with zeroes
sprite_end_variables: ; in case we want to traverse the table in reverse

; pointer to top address of current record, initialized to table_base
table_pointer: dl table_base
; how many active sprites
table_active_sprites: db 0x00
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
    ld ix,table_base
    xor a ; sprite id
    ld c,0 ; sprite type
    ld b,table_max_records ; loop counter
@loop:
    ld (ix+sprite_id),a
    ld (ix+sprite_type),c
    lea ix,ix+table_bytes_per_record
    inc a
    djnz @loop
    ret

; gets the next available sprite id
; inputs; none
; returns: if new sprite available, a = sprite id, ix pointing to new sprite vars, carry set
;     otherwise, a = 0, carry flag reset, ix pointing to highest sprite vars
; destroys: a,b,c,hl,ix
; affects: bumps table_active_sprites by one
table_get_next_id:
    ld ix,table_base
    ld b,table_max_records
    ld c,0 ; sprite id counter
@loop:
    ld a,(ix+sprite_type)
    and a
    jr z,@found
    lea ix,ix+table_bytes_per_record
    inc c
    djnz @loop
@notfound:
    xor a ; a = 0 and reset carry flag indicating that we didn't find a free sprite
    ret
@found:
; bump number of active sprites
    ld hl,table_active_sprites
    inc (hl)
; return sprite id
    ld a,c
    scf ; sets carry flag indicating we found a free sprite
    ret ; done
; end table_get_next_id

; deactivate the sprite with the given id
; inputs: iy set to sprite record
; outputs: nothing
; destroys: a,hl
; affects: decrements table_active_sprites by one
table_deactivate_sprite:
    ld a,(iy+sprite_id) ; get spriteId
    call vdu_sprite_select ; select sprite 
    call vdu_sprite_hide
    xor a
    ld (iy+sprite_type),a
    ld hl,table_active_sprites
    dec (hl)
    ret
; end table_deactivate_sprite

sprites_init:
; initialize sprites
	call vdu_sprite_reset ; out of an abundance of caution (copilot: and paranoia)
	xor a
@sprite_loop:
	push af
	call vdu_sprite_select
	ld hl,BUF_CRATER ; can be anything, but why not a smoking hole in the ground?
	call vdu_sprite_add_buff
	pop af
	inc a
	cp table_max_records+2 ; tack on sprites for player and laser
	jr nz,@sprite_loop
	inc a
	call vdu_sprite_activate

; define player sprite
	ld a,(player_id)
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

; define laser sprite
	ld a,(laser_id)
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
; all done
    ret
; end sprites_init

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
    ld iy,(table_pointer) 
    ld de,(iy+sprite_vel) 
    call polar_to_cartesian
    ld iy,(table_pointer) 
    ld (iy+sprite_xvel),bc ; save x-velocity component
    ld (iy+sprite_yvel),de ; save y-velocity component 
    ret
; end sprite_compute_velocities

; create a new sprite at the same location as an existing sprite
; inputs: hl = pointer to new sprite default settings, table_pointer = existing sprite
spawn_sprite:
    push hl ; back up pointer to new sprite defaults
    call table_get_next_id ; if available: a = new sprite id, ix = pointer to new sprite vars
    pop hl ; restore pointer to new sprite defaults
    ret nc ; no carry means no free sprite slots, so we go home
; copy default settings to new sprite
    push ix ; back up pointer to new sprite record
    lea ix,ix+sprite_definitions ; bump pointer to copy to
    push ix ; pointer to default sprite settings
    pop de ; address to copy to
    ld bc,table_bytes_per_record-sprite_definitions ; number of bytes to copy
    ldir ; copies defaults to new sprite record
; return to caller
    pop ix ; restore pointer to new sprite record
    set 7,(ix+sprite_type) ; set just spawned flag
    scf ; set carry flag to indicate success
    ret

; inputs: hl = pointer to new sprite default settings, table_pointer = existing sprite
spawn_sprite_from_sprite: 
    call spawn_sprite
    ret nc ; no carry means no free sprite slots, so we go home
; copy sprite position, velocity and orientation from existing sprite
    ld iy,(table_pointer) ; get existing sprite record
    ld hl,(iy+sprite_x)
    ld (ix+sprite_x),hl
    ld hl,(iy+sprite_y)
    ld (ix+sprite_y),hl
; return to caller
    set 7,(ix+sprite_type) ; set just spawned flag
    scf ; set carry flag to indicate success
    ret

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

; compute the sprite frame based on the sprite's heading / 8
; inputs: table_pointer = sprite record
; outputs: sprite_frame updated, both h and l = frame number
; destroys: hl
sprite_frame_orientation:
    ld h,(iy+sprite_heading+1) ; we just want the integer portion
    srl h ; divide integer portion by 8
    srl h
    srl h
    ld (iy+sprite_frame),h ; modulo 256 since sprite_frame is one byte
    ld l,h ; return frame number in l for parity with sprite_frame_orientation_animation
    ret

; compute the sprite frame based on the sprite's heading / 8 and animation
; inputs: table_pointer = sprite record
; outputs: sprite_frame updated, l = frame number
; destroys: hl
sprite_frame_orientation_animation:
    ld h,(iy+sprite_heading+1) ; we just want the integer portion
    srl h ; divide integer portion by 8
    srl h
    srl h
    ld l,(iy+sprite_animation)
    inc l ; make 1-based so the mlt works
    mlt hl ; hl = heading * (animation + 1)
    ld (iy+sprite_frame),l ; modulo 256 since sprite_frame is one byte
    ret
; end sprite_frame_orientation_animation

dump_sprite_record:
    ; call vdu_home_cursor
    call printInline
    asciz "id:   "
    ld a,(ix+sprite_id)
    call printHexA

    call printInline
    asciz "\r\nx:   "
    ld hl,(ix+sprite_x)
    call print_s168_hl

    call printInline
    asciz "\r\ny:   "
    ld hl,(ix+sprite_y)
    call print_s168_hl

    call printInline
    asciz "\r\nxv:  "
    ld hl,(ix+sprite_xvel)
    call print_s168_hl

    call printInline
    asciz "\r\nyv:  "
    ld hl,(ix+sprite_yvel)
    call print_s168_hl

    call printInline
    asciz "\r\nv:   "
    ld hl,(ix+sprite_vel)
    call print_s168_hl

    call printInline
    asciz "\r\nhdg  "
    ld hl,(ix+sprite_heading)
    call print_s168_hl

    call printInline
    asciz "\r\nori: "
    ld hl,(ix+sprite_orientation)
    call print_s168_hl
    
    call printInline
    asciz "\r\ntyp: "
    ld a,(ix+sprite_type)
    call printHexA
    
    call printInline
    asciz "\r\nBId: "
    ld hl,(ix+sprite_base_bufferId)
    call printHex24

    call printInline
    asciz "\r\nmPg: "
    ld hl,(ix+sprite_move_program)
    call printHex24

    call printInline
    asciz "\r\ncol: "
    ld a,(ix+sprite_collisions)
    call printBin8

    call printInline
    asciz "\r\nszx: "
    ld a,(ix+sprite_dim_x)
    call printHexA
    
    call printInline
    asciz "\r\nszy: "
    ld a,(ix+sprite_dim_y)
    call printHexA

    call printInline
    asciz "\r\n#ori:"
    ld a,(ix+sprite_num_orientations)
    call printHexA

    call printInline
    asciz "\r\n#ani:"
    ld a,(ix+sprite_num_animations)
    call printHexA

    call printInline
    asciz "\r\nani: "
    ld a,(ix+sprite_animation)
    call printHexA

    call printInline
    asciz "\r\nanit:"
    ld a,(ix+sprite_animation_timer)
    call printHexA

    call printInline
    asciz "\r\nmvt: "
    ld a,(ix+sprite_move_timer)
    call printHexA

    call printInline
    asciz "\r\nmvs: "
    ld a,(ix+sprite_move_step)
    call printHexA

    call printInline
    asciz "\r\npts: "
    ld a,(ix+sprite_points)
    call printHexA

    call printInline
    asciz "\r\ndmg: "
    ld a,(ix+sprite_shield_damage)
    call printHexA

    call printInline
    asciz "\r\nfrm: "
    ld a,(ix+sprite_frame)
    call printHexA

    ; call waitKeypress
    ret