
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
    dl activate_crater
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

move_tiles:
; initialize pointers and loop counter
    ld iy,tile_stack ; set iy to first record in table
@move_loop:
    ld (tile_stack_pointer),iy ; update stack pointer
    ld a,(iy+2)
    or a ; will be zero if at end of stack
    ret z ; end of stack so we're done
; point iy to tile record
    ld iy,(iy) ; iy points to the current tile record
    ld (tile_table_pointer),iy ; update table pointer
; check top bit of tile_type to see if tile is just spawned
    bit 7,(iy+tile_type)
    jp nz,@just_spawned ; if just spawned, skip to next record
; check sprite_alive bit to see if tile is active
    bit sprite_alive,(iy+sprite_collisions)
    ret z ; end of stack so we're done
; otherwise we prepare to move the tile
    ld hl,(iy+tile_move_program) ; load the behavior subroutine address
    CALL_HL
 ; get back stack pointer and loop
@next_record:
    ld iy,(tile_stack_pointer)
    lea iy,iy+3 ; next tile stack record
    jp @move_loop ; loop until we've checked all the records
@just_spawned:
    res 7,(iy+tile_type) ; clear just spawned flag
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
    ld bc,table_record_size-sprite_definitions ; number of bytes to copy
    ldir ; copies defaults to new sprite record
    pop ix ; restore pointer to new sprite record
; copy sprite position from existing tile
    ld iy,(tile_table_pointer) ; get existing tile record
    ld hl,(iy+tile_x)
    ld (ix+sprite_x),hl
    ld hl,(iy+tile_y)
    ld (ix+sprite_y),hl
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
    ld bc,tile_table_record_size-tile_definitions ; number of bytes to copy
    ldir ; copies defaults to new tile record
    ld ix,(tile_table_pointer) ; restore pointer to new tile record
    ret
; end spawn_active_tile_from_tile