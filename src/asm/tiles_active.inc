
; ###### TODO: NEW CODE TO IMPLEMENT ######
tile_pad_small: equ 1
tile_pad_med: equ 2
tile_pad_large: equ 3
tile_charge_shields: equ 4
tile_turret_fireball: equ 5
tile_laser_turret: equ 6
tile_twin_laser_turret: equ 7
tile_charge_fuel: equ 8
tile_electrode: equ 9
tile_lightning: equ 10
; tile_electrode_r: equ 11 ; same as tile_electrode
tile_12: equ 12
tile_13: equ 13
tile_14: equ 14
tile_15: equ 15
tile_crater: equ 16

tile_activation_routines:
    dl activate_null_tile ; dummy spacer since tiled is 1-based
    dl activate_pad_small
    dl activate_null_tile
    dl activate_null_tile
    dl activate_null_tile
    dl activate_turret_fireball
    dl activate_null_tile
    dl activate_null_tile
    dl activate_null_tile
    dl activate_electrode
    dl activate_lightning
    dl activate_electrode
    dl activate_null_tile
    dl activate_null_tile
    dl activate_null_tile
    dl activate_null_tile ; tile 15
    dl activate_crater ; tile 16
tile_activation_routines_end:
tile_activation_routines_count: equ [tile_activation_routines_end-tile_activation_routines]/3
; end tile_activation_routines

activate_null_tile:
    ret ; dummy routine, do nothing
move_active_tile:
; y-axis movement
    ld hl,(iy+tile_y)
    ld de,(iy+tile_yvel)
    add hl,de
    ld (iy+tile_y),hl
; detect screen edge collisions
; y-axis bottom is all we need to test
    ld de,-tile_bottom*256
    adc hl,de    
    ret m ; still on screen so do nothing further
    xor a ; signal caller that tile has scrolled off screen
    ret
; end move_active_tile

; Exact stabilization whitelist for writable legacy tile callbacks.
; inputs: iy = validated tile record, hl = stored callback
; outputs: carry set and hl = expected callback on an exact type/program match
; destroys: a,de,hl
tile_move_program_valid:
    ld a,(iy+tile_type)
    cp tile_pad_small
    jp z,@pad_small
    cp tile_turret_fireball
    jp z,@turret_fireball
    cp tile_electrode
    jp z,@electrode
    cp tile_lightning
    jp z,@lightning
    cp tile_crater
    jp z,@crater
    jp @invalid
@pad_small:
    ld de,move_pad_small
    jp @compare
@turret_fireball:
    ld de,move_turret_fireball
    jp @compare
@electrode:
    ld de,move_electrode
    jp @compare
@lightning:
    ld de,move_lightning
    jp @compare
@crater:
    ld de,move_crater
@compare:
    or a
    sbc hl,de
    jp nz,@invalid
    push de
    pop hl ; publish the fixed expected address, not writable record data
    scf
    ret
@invalid:
    xor a
    ret
; end tile_move_program_valid

move_tiles:
; initialize the dense-stack cursor
    ld a,(num_active_tiles)
    or a
    ret z
    ld iy,tile_stack ; set iy to first record in table
@move_loop:
    push iy
    pop hl
    call tile_stack_pointer_valid
    ret nc ; normal end-of-stack or fail-closed malformed cursor
    ld (tile_stack_pointer),iy ; update stack pointer
; point iy to tile record
    ld iy,(iy) ; iy points to the current tile record
    call tile_table_record_pointer_valid
    jp nc,@bad_record
    ld (tile_table_pointer),iy ; update table pointer
; check top bit of tile_type to see if tile is just spawned
    bit 7,(iy+tile_type)
    jp nz,@just_spawned ; if just spawned, skip to next record
; check sprite_alive bit to see if tile is active
    bit sprite_alive,(iy+tile_collisions)
    jp z,@inactive_record
; otherwise we prepare to move the tile
    ld hl,(iy+tile_move_program) ; load the behavior subroutine address
    call tile_move_program_valid
    jp nc,@bad_move_program
    push iy ; callbacks may use index registers internally but do not own them
    CALL_HL
    pop iy
 ; get back stack pointer and loop
@next_record:
    ld iy,(tile_stack_pointer)
    lea iy,iy+3 ; next tile stack record
    jp @move_loop ; loop until we've checked all the records
@just_spawned:
    res 7,(iy+tile_type) ; clear just spawned flag
    jp @next_record
@inactive_record:
    call table_deactivate_tile
    jp @next_record
@bad_record:
    ld hl,tile_fault_flags
    set tile_fault_bad_record,(hl)
    call tile_stack_quarantine_entry
    jp @next_record
@bad_move_program:
    ld hl,tile_fault_flags
    set tile_fault_bad_move_program,(hl)
    call table_deactivate_tile
    jp @next_record
; end move_tiles

draw_active_tile:
    ld hl,(iy+tile_base_bufferId)
    ld a,(iy+tile_frame)
    add a,l ; TODO: with a little work we could make all tile base buffer ids align to 256
    ld l,a
    ld a,0 ; preserve carry
    adc a,h
    ld h,a
    call vdu_buff_select ; buffer id of animation frame
    ld bc,(iy+tile_x)
    ld de,(iy+tile_y)
    call vdu_plot_bmp168
    ret
; end draw_active_tile

; create a new sprite at the same location as the current tile
; inputs: hl = pointer to new sprite default settings, tile_table_pointer = current tile
;         ix = pointer to sprite group, b = number of records in sprite group
spawn_sprite_from_tile: 
    push hl ; back up pointer to new sprite defaults
    call table_get_next_id ; if available: a = new sprite id, ix = pointer to new sprite vars
    pop hl ; restore pointer to new sprite defaults
    ret nc ; no carry means no free sprite slots, so we go home
; copy default settings to new sprite
    push ix ; back up pointer to new sprite record
    lea ix,ix+sprite_definitions ; bump pointer to copy to
    push ix ; pointer to default sprite settings
    pop de ; address to copy to
    ld bc,sprite_definitions_size ; copy only bytes provided by defaults
    ldir ; copies defaults to new sprite record
    pop ix ; restore pointer to new sprite record
; copy sprite position from existing tile
    ld iy,(tile_table_pointer) ; get existing tile record
    ld hl,(iy+tile_x)
    ld (ix+sprite_x),hl
    ld hl,(iy+tile_y)
    ld (ix+sprite_y),hl
; clear motion state retained by a previously used physical record
    ld hl,0
    ld (ix+sprite_xvel),hl
    ld (ix+sprite_yvel),hl
    ld (ix+sprite_vel),hl
    ld (ix+sprite_heading),hl
    ld (ix+sprite_orientation),hl
; return to caller
    set 7,(ix+sprite_type) ; set just spawned flag
    set sprite_alive,(ix+sprite_collisions) ; set sprite alive flag
    scf ; set carry flag to indicate success
    ret
; end spawn_sprite_from_tile

; create a new tile at the same location as the current tile
; inputs: tile_table_pointer = current tile
spawn_active_tile_from_tile:
; copy default settings to new tile
    ld ix,(tile_table_pointer) ; get existing tile record
    lea ix,ix+tile_definitions ; bump pointer to copy to
    push ix ; pointer to default tile settings
    pop de ; address to copy to
    ld bc,tile_definitions_size ; copy only bytes provided by defaults
    ldir ; copies defaults to new tile record
    ld ix,(tile_table_pointer) ; restore pointer to new tile record
    ret
; end spawn_active_tile_from_tile
