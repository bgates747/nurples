; ###### SPRITE TABLE FIELD INDICES ######
table_bytes_per_record: equ 38 ; 38 bytes per sprite record
sprite_id:              equ 00 ; 1 bytes unique spriteId, zero-based
sprite_type:            equ 01 ; 1 bytes type of sprite as defined in enemies.inc
sprite_base_bufferId:   equ 02 ; 3 bytes bitmap bufferId
sprite_move_program:    equ 05 ; 3 bytes address of sprite's behavior subroutine
sprite_collisions:      equ 08 ; 1 bytes low/high nibble: collision details
sprite_dim_x:           equ 09 ; 1 bytes sprite width in pixels
sprite_dim_y:           equ 10 ; 1 bytes sprite height in pixels
sprite_x:               equ 11 ; 3 bytes 16.8 fractional x position in pixels
sprite_y:               equ 14 ; 3 bytes 16.8 fractional y position in pixels
sprite_xvel:            equ 17 ; 3 bytes x-component velocity, 16.8 fixed, pixels
sprite_yvel:            equ 20 ; 3 bytes y-component velocity, 16.8 fixed, pixels
sprite_vel:             equ 23 ; 3 bytes velocity px/frame (16.8 fixed)
sprite_heading:         equ 26 ; 3 bytes sprite movement direction deg256 16.8 fixed
sprite_orientation:     equ 29 ; 3 bytes orientation bits
sprite_animation:       equ 32 ; 1 bytes current animation index, zero-based
sprite_animation_timer: equ 33 ; 1 bytes when hits zero, draw next animation
sprite_move_timer:      equ 34 ; 1 bytes when zero, go to next move program, or step
sprite_move_step:       equ 35 ; 1 bytes stage in a move program sequence, varies
sprite_points:          equ 36 ; 1 bytes points awarded for killing this sprite type, BCD
sprite_shield_damage:   equ 37 ; 1 bytes shield points deducted for collision, binary

; ###### SPRITE TABLE VARIABLES ######
; maximum number of sprites
table_max_records: equ 16 ; it can handle more but this is pushing it
table_total_bytes: equ table_max_records*table_bytes_per_record

; #### THIS IS THE SPACE ALLOCATED TO THE SPRITE TABLE ####
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
; next sprite id to use
sprite_next_id: db 0

; origin_top: equ 48
origin_top: equ 0 ; DEBUG
origin_left: equ 128
field_top: equ 0
field_bottom: equ 383-origin_top
field_left: equ 0
field_right: equ 255
sprite_top: equ 0
sprite_bottom: equ field_bottom-16
sprite_left: equ field_left
sprite_right: equ field_right-16
collide_top: equ %00001000
collide_bottom: equ %00000100
collide_left: equ %00000010
collide_right: equ %00000001

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

; gets the next available sprite id
; inputs; none
; returns: if new sprite available, a = sprite id, 
;           ix pointing to new sprite vars, carry set
;      otherwise, a = 0, carry flag reset, ix pointing to highest sprite vars
; destroys: a,b,hl,ix
; affects: bumps table_active_sprites by one
table_get_next_id:
    ld ix,table_base
    ld de,table_bytes_per_record
    ld b,table_max_records
@loop:
    ld a,(ix+sprite_type)
    and a
    jr z,@found
    add ix,de
    djnz @loop
@notfound:
    xor a ; a = 0 and reset carry flag indicating that we didn't find a free sprite
    ret
@found:
; bump number of active sprites
    ld hl,table_active_sprites
    inc (hl)
; return sprite id
    ld a,table_max_records
    sub b
    ld (sprite_next_id),a
    scf ; sets carry flag indicating we found a free sprite
    ret ; done

; deactivate the sprite with the given id
; inputs: a = sprite id
; outputs: nothing
; destroys: a,ix,de
; affects: decrements table_active_sprites by one
table_deactivate_sprite:
    push af ; save sprite id bc we need it later
    call vdu_sprite_select
    call vdu_sprite_hide
    pop af ; restore sprite id
    ld de,0 ; clear deu
    ld d,a
    ld e,table_bytes_per_record
    mlt de
    ld ix,table_base
    add ix,de
    xor a
    ld (ix+sprite_type),a
    ld ix,table_active_sprites
    dec (ix)
    ret

sprites_init:
; initialize sprites
	call vdu_sprite_reset ; out of an abundance of caution (copilot: and paranoia)
	xor a
@sprite_loop:
	push af
	call vdu_sprite_select
	ld hl,BUF_0TILE_EMPTY ; can be anything, but why not blank?
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
	call vdu_sprite_show

; all done
    ret

; compute a sprite's new position based on its component x,y velocities
; with screen boundary checking and update its sprite table record accordingly
; inputs: iy pointing to sprite table record
; returns: updated x, y positions and edge collision flags in sprite table record
; destroys: everything
; note: actual rendering of sprite must be handled by caller
move_sprite:
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