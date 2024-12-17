
; ###### TODO: NEW CODE TO IMPLEMENT ######
tile_pad_small: equ 1
tile_pad_med: equ 2
tile_pad_large: equ 3
tile_pad_huge: equ 4
tile_turret_fireball: equ 5
tile_laser_turret: equ 6
tile_twin_laser_turret: equ 7
tile_gatling_turret: equ 8
tile_electrode_r: equ 9
tile_electrode_u: equ 10
tile_piston_r: equ 11
tile_piston_u: equ 12
tile_spark_r: equ 13
tile_spark_u: equ 14
tile_flame_r: equ 15
tile_flame_u: equ 16

active_tile_timer: equ 24 ; 2/10ths of a second
tile_activation_routines:
    dl activate_null_tile ; dummy spacer since tiled is 1-based
    dl activate_pad_small
    dl activate_null_tile  ;  activate_pad_med
    dl activate_null_tile  ;  activate_pad_large
    dl activate_null_tile  ;  activate_pad_huge
    dl activate_turret_fireball  
    dl activate_null_tile  ;  activate_laser_turret
    dl activate_null_tile  ;  activate_twin_laser_turret
    dl activate_null_tile  ;  activate_gatling_turret
    dl activate_null_tile  ;  activate_electrode_r
    dl activate_null_tile  ;  activate_electrode_u
    dl activate_null_tile  ;  activate_piston_r
    dl activate_null_tile  ;  activate_piston_u
    dl activate_null_tile  ;  activate_spark_r
    dl activate_null_tile  ;  activate_spark_u
    dl activate_null_tile  ;  activate_flame_r
    dl activate_null_tile  ;  activate_flame_u
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
    set tile_just_died,(iy+tile_collisions) ; scrolled off screen so deactivate
    ret
; end move_active_tile

move_tiles:
; initialize pointers and loop counter
    ld iy,tile_stack ; set iy to first record in table
    ; ld b,table_num_records ; loop counter
@move_loop:
    ld (tile_stack_pointer),iy ; update stack pointer
    ld a,(iy+2)
    or a ; will be zero if at end of stack
    ret z ; end of stack so we're done
; point iy to tile record
    ld iy,(iy) ; iy points to the current tile record
    ld (tile_table_pointer),iy ; update table pointer
    ; push bc ; backup loop counter
; check top bit of tile_type to see if tile is just spawned
    ld a,(iy+tile_type)
    bit 7,a
    jp nz,@just_spawned ; if just spawned, skip to next record
; check tile_type to see if tile is active
    and a ; if zero, tile is dead 
    jp z,@next_record ; ... and we skip to next record
; otherwise we prepare to move the tile
    ld hl,(iy+tile_move_program) ; load the behavior subroutine address
    CALL_HL
; move_tiles_loop_return: return from behavior subroutines
    ld iy,(tile_table_pointer) ; get back table pointer
; now we check results of all the moves
    bit tile_just_died,(iy+tile_collisions)
    jp z,@next_record ; if not dead, go to next record
    call table_deactivate_tile ; otherwise, deactivate tile
    ld iy,(tile_stack_pointer) ; get back stack pointer
    jp @move_loop
@just_spawned:
    res 7,(iy+tile_type) ; clear just spawned flag
    ; fall through to @next_record
@next_record:
    ld iy,(tile_stack_pointer)
    lea iy,iy+3 ; next tile stack record
    jp @move_loop ; loop until we've checked all the records
; end move_tiles

; TODO: much if this is deprecated by new tile table system and needs to be rewritten
check_tile_collisions:
; ; check what we collide with
;     ld a,(iy+tile_collisions)
;     push af ; save collision flags
;     and collides_with_player ; check for collisions with player
;     call nz,check_tile_collision_player
;     pop af ; restore collision flags
;     and collides_with_laser ; check for collisions with laser
;     call nz,check_tile_collision_laser
    ret
check_tile_collision_laser: ; check for collisions with laser
    ; ld ix,laser_start_variables
    ; bit tile_alive,(ix+tile_collisions)
    ; ret z ; laser dead, nothing to do
    ; call check_collision
    ; ret p ; no collision
    ; ld ix,laser_start_variables
    ; set tile_just_died,(ix+tile_collisions)
    ; set tile_alive,(ix+tile_collisions)
    ; call tile_kill
    ret
check_tile_collision_player: ; check for collisions with player
    ; ld ix,player_begin
    ; bit tile_alive,(ix+tile_collisions)
    ; ret z ; player dead, no collision
    ; call check_collision
    ; ret p ; no collision
    ; call tile_kill
    ret
; end check_tile_collisions
; END TODO

; this routine vanquishes the tile tile
; and replaces it with an animated explosion
; destroys: everything except index registers
; returns: a=-1 to indicate tile died, and an incandescent ball of debris and gas
tile_kill:
    ld iy,(tile_stack_pointer)
    ld hl,(player_score)
    ld bc,0 ; clear bcu
    ld c,(iy+tile_points)
    add hl,bc
    ld (player_score),hl
    jp spawn_explosion
; end tile_kill

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