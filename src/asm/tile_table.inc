; ###### tile TABLE FIELD INDICES ######
tile_table_record_size:         equ  64; 40 bytes ;40 bytes per tile record
tile_id:                 equ   0; 1 bytes ;unique tileId, zero-based
; tile state variables
tile_state_variables:    equ   1; beginning of tile state variables
tile_x:                  equ   1; 3 bytes ;16.8 fractional x position in pixels
tile_y:                  equ   4; 3 bytes ;16.8 fractional y position in pixels
tile_xvel:               equ   7; 3 bytes ;x-component velocity, 16.8 fixed, pixels
tile_yvel:               equ  10; 3 bytes ;y-component velocity, 16.8 fixed, pixels
tile_vel:                equ  13; 3 bytes ;velocity px/frame (16.8 fixed)
tile_heading:            equ  16; 3 bytes ;tile movement direction deg256 16.8 fixed
tile_orientation:        equ  19; 3 bytes ;orientation bits
; tile definition variables
tile_definitions:        equ  22; beginnning of tile definition variables
tile_type:               equ  22; 1 bytes ;type of tile as defined in enemies.inc
tile_base_bufferId:      equ  23; 3 bytes ;bitmap bufferId
tile_move_program:       equ  26; 3 bytes ;address of tile's behavior subroutine
tile_collisions:         equ  29; 1 bytes ;see collisions.inc constants for bit definitions
tile_dim_x:              equ  30; 1 bytes ;tile width in pixels
tile_dim_y:              equ  31; 1 bytes ;tile height in pixels
tile_num_orientations:   equ  32; 1 bytes ;number of orientations for this tile
tile_num_animations:     equ  33; 1 bytes ;number of animations for this tile
tile_animation:          equ  34; 1 bytes ;current animation index, zero-based
tile_animation_timer:    equ  35; 1 bytes ;when hits zero, draw next animation
tile_move_timer:         equ  36; 1 bytes ;when zero, go to next move program, or step
tile_move_step:          equ  37; 1 bytes ;stage in a move program sequence, varies
tile_points:             equ  38; 1 bytes ;points awarded for killing this tile type
tile_shield_damage:      equ  39; 1 bytes ;shield points deducted for collision
tile_frame:              equ  40; 1 bytes ;current animation frame

; ###### tile TABLE VARIABLES ######
tile_stack: blkb 256*3,0 ; stack of pointers to tile records
    db 0 ; list terminator
tile_stack_end:
tile_stack_pointer: dl tile_stack ; pointer to current stack record, initialized to tile_stack
tile_table_pointer: dl tile_table_base ; pointer to top address of current record, initialized to tile_table_base
num_active_tiles: dl 0 ; how many active tiles
next_tile_id: db 0 ; next available tile id
new_tile_table_pointer: dl 0 ; pointer to new tile record

tile_table_init:
; clear the tile stack
    xor a ; write zeroes to tile stack
    ld hl,tile_stack
    ld bc,tile_stack_end-tile_stack
    call clear_mem
; clear the tile table
    xor a ; write zeroes to tile table
    ld hl,tile_table_base ; address to copy from
    ld bc,tile_table_end-tile_table_base ; number of bytes to clear
    call clear_mem
; write tile ids
    ld ix,tile_table_base
    xor a ; tile id
    ld b,0 ; loop counter
@loop:
    ld (ix+tile_id),a
    lea ix,ix+table_record_size
    inc a
    djnz @loop
; set pointers to top of tile stack and tile table
    ld hl,next_tile_id
    ld a,255 ; will wrap around to 0 on first call to create active tile
    ld (hl),a
    ld hl,tile_table_base
    ld (tile_table_pointer),hl
    ld hl,tile_stack
    ld (tile_stack_pointer),hl
    ret
; end tile_table_init

; gets the next available tile id
; inputs; none
; returns: if new tile available, a = tile id, ix pointing to new tile vars, carry set
;      otherwise, a = 0, carry flag reset, ix pointing to highest tile vars
; destroys: a,hl,ix
; affects: on success, bumps num_active_tiles by one, adds a tile pointer to the top of the tile stack
tile_table_get_next_id:
    ld hl,num_active_tiles
    inc (hl)
    jp nz,@F ; check rollover to > 256 active tiles
    dec (hl) ; undo increment
    xor a ; reset carry flag indicating that we didn't find a free tile
    ret
@@: ; point ix to next tile record
    ld hl,next_tile_id
    inc (hl)
    ld d,(hl) ; index into tile table
    ld e,tile_table_record_size
    mlt de ; de is now the offset to the table entry
    ld ix,tile_table_base ; point to base address of tile table
    add ix,de ; ix is now the address of the table entry
; add tile pointer to tile stack
    ld hl,tile_stack
    add hl,de ; de was already set to the correct offset
    ld (hl),ix
; return tile id
    ld a,(hl)
    scf ; sets carry flag indicating we found a free tile
    ret ; done
; end tile_table_get_next_id

; deactivate the tile with the given id
; inputs: iy pointing to tile stack record
; outputs: tile stack pointer unchanged, effectively pointing to next record
; destroys: a,hl
; affects: decrements num_active_tiles by one, removes tile pointer from the tile stack, shifts stack records above it down one position
table_deactivate_tile: ; TODO: implement
    ld hl,num_active_tiles
    dec (hl) ; decrement number of active tiles
; remove tile pointer from tile stack
    push iy ; pointer to tile stack record
    pop de ; address to copy to
    ld h,(hl) ; number of active tiles
    dec h ; zero-based index
    ld l,3 ; number of bytes per tile stack record
    mlt hl ; hl = offset from base stack record
    inc hl ; to pick up zero terminator
    or a ; clear carry
    sbc hl,de ; number of bytes to copy
    push hl
    pop bc ; number of bytes to copy
    lddr ; shift stack records down
    ret
; end table_deactivate_tile

; change an existing tile to a new type, keeping its position and velocity the same
; inputs: hl = pointer to new tile default settings, tile_table_pointer = existing tile
alter_tile:
; copy default settings to new tile
    lea iy,iy+tile_definitions ; bump pointer to copy to
    push iy ; pointer to default tile settings
    pop de ; address to copy to
    ld bc,tile_table_record_size-tile_definitions ; number of bytes to copy
    ldir ; copies new defaults existing tile record
; return to caller
    ld iy,(tile_table_pointer) ; reset tile_table_pointer
    set 7,(iy+tile_type) ; set just spawned flag
    scf ; set carry flag to indicate success
    ret
; end alter_tile

; spawn a new active tile at the prescribed location
; inputs: hl = pointer to new tile default settings, tiles_x_plot, tiles_y_plot = x,y position
; returns: if new tile available, a = tile id, ix pointing to new tile vars, carry set
;      otherwise, a = 0, carry flag reset, ix pointing to highest tile vars
; destroys: a,hl,ix
; preserves: bc,de (x,y position)
; affects: on success, bumps num_active_tiles by one, adds a tile pointer to the top of the tile stack
spawn_active_tile:
    push hl ; back up pointer to new tile defaults
    call tile_table_get_next_id ; if available: a = new tile id, ix = pointer to new tile vars
    pop hl ; restore pointer to new tile defaults
    ret nc ; no carry means no free tile slots, so we go home
; copy default settings to new tile
    ld (new_tile_table_pointer),ix ; save pointer to new tile record
    lea ix,ix+tile_definitions ; bump pointer to copy to
    push ix ; pointer to default tile settings
    pop de ; address to copy to
    ld bc,tile_table_record_size-tile_definitions ; number of bytes to copy
    ldir ; copies defaults to new tile record
; copy tile position from inputs
    ld ix,(new_tile_table_pointer) ; reset pointer to new tile record
    ld hl,(tiles_x_plot-1) ; shift up one byte to get 16.8 fixed point
    ld (ix+tile_x),hl
    ld hl,(tiles_y_plot-1) ; shift up one byte to get 16.8 fixed point
    ld (ix+tile_y),hl
; set default tile velocities
    ld hl,0
    ld (ix+tile_xvel),hl
    ld hl,1*256 ; 1 pixel per frame
    ld (ix+tile_yvel),hl
; return to caller
    set 7,(ix+tile_type) ; set just spawned flag
    scf ; set carry flag to indicate success
    ret
; end spawn_active_tile

; compute animation frame based on tile_animation and tile_heading
; inputs: iy set to tile record
; outputs: a, tile_frame set to animation frame
; destroys: a,hl
tile_frame_heading_animation:
    ld a,(iy+tile_heading+1) ; we want the integer portion
    srl a ; divide integer portion by 8
    srl a
    srl a
    ld h,(iy+tile_animation)
    ld l,32 ; 32 orientation frames per animation
    mlt hl ; hl = offset from base animation
    add a,l ; modulo256 since tile_frame is only 1 byte
    ld (iy+tile_frame),a
    ret
; end tile_frame_heading_animation

; compute animation frame based on tile_heading
; inputs: iy set to tile record
; outputs: a, tile_frame set to animation frame
; destroys: a,hl
tile_frame_heading:
    ld a,(iy+tile_heading+1) ; we want the integer portion
    srl a ; divide integer portion by 8
    srl a
    srl a
    ld (iy+tile_frame),a
    ret
; end tile_frame_heading
dump_tile_record:
    ; call vdu_home_cursor
    call printInline
    asciz "id:   "
    ld a,(ix+tile_id)
    call printHexA

    call printInline
    asciz "\r\nx:   "
    ld hl,(ix+tile_x)
    call print_s168_hl

    call printInline
    asciz "\r\ny:   "
    ld hl,(ix+tile_y)
    call print_s168_hl
    
    call printInline
    asciz "\r\ntyp: "
    ld a,(ix+tile_type)
    call printHexA
    
    call printInline
    asciz "\r\nBId: "
    ld hl,(ix+tile_base_bufferId)
    call printHex24

    call printInline
    asciz "\r\nmPg: "
    ld hl,(ix+tile_move_program)
    call printHex24

    call printInline
    asciz "\r\ncol: "
    ld a,(ix+tile_collisions)
    call printBin8

    ; call printInline
    ; asciz "\r\nszx: "
    ; ld a,(ix+tile_dim_x)
    ; call printHexA
    
    ; call printInline
    ; asciz "\r\nszy: "
    ; ld a,(ix+tile_dim_y)
    ; call printHexA

    ; call printInline
    ; asciz "\r\n#ori:"
    ; ld a,(ix+tile_num_orientations)
    ; call printHexA

    ; call printInline
    ; asciz "\r\n#ani:"
    ; ld a,(ix+tile_num_animations)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nani: "
    ; ld a,(ix+tile_animation)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nanit:"
    ; ld a,(ix+tile_animation_timer)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nmvt: "
    ; ld a,(ix+tile_move_timer)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nmvs: "
    ; ld a,(ix+tile_move_step)
    ; call printHexA

    ; call printInline
    ; asciz "\r\npts: "
    ; ld a,(ix+tile_points)
    ; call printHexA

    ; call printInline
    ; asciz "\r\ndmg: "
    ; ld a,(ix+tile_shield_damage)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nfrm: "
    ; ld a,(ix+tile_frame)
    ; call printHexA

    call printNewLine
    call printNewLine

    ; call waitKeypress
    ret