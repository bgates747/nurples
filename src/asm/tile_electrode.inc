    align 256
electrode_defaults:
    db 9                    ; type
    dl BUF_TILE_BLANK       ; base_bufferId
    dl move_electrode       ; move_program
    db %01000000            ; collisions bit 6 signals sprite alive
    db 16                   ; dim_x
    db 16                   ; dim_y
    db 1                    ; num_orientations
    db 1                    ; num_animations
    db 0                    ; animation
    db 0                    ; animation_timer
    db 0                    ; move_timer
    db 0                    ; move_step
    db 5                    ; points
    db -4                   ; shield_damage
    db 0                    ; frame

activate_electrode:
    ld hl,electrode_defaults
; get next available active tile id
    call spawn_active_tile
    ; ret nc ; no carry means no free active tile slots, so we go home
; other specific initialization
    ret
; end activate_electrode

move_electrode:
    call move_active_tile 
    jp z,table_deactivate_tile
    call check_enemy_collision_player
    jp nz,@laser ; no collision so check laser collision    
    call update_shields
    jp kill_electrode
@laser:
    call check_enemy_collision_laser
    ret nz ; no collision so we're done
    call update_score
    jp kill_electrode
; end move_electrode

kill_electrode:
; activate blank tile bitmap
    ld hl,BUF_TILE_BLANK
    call vdu_buff_select
; initialize pointers
    ld hl,(tile_stack_pointer)
    ld hl,(hl) ; pointer to this tile table record
    push hl ; save pointer to this tile table record
    ld ix,tile_stack ; set ix to first record in table
@check_loop:
    ld (@tile_stack_pointer),ix ; update stack pointer
    ld a,(ix+2)
    or a ; will be zero if at end of stack
    jp z,@done ; end of stack so we're done
; point ix to tile record
    ld ix,(ix) ; ix points to the tile record being checked
; check top bit of tile_type to see if tile is just spawned
    bit 7,(ix+tile_type)
    jp nz,@next_record ; if just spawned, skip to next record
; check sprite_alive bit to see if tile is active
    bit sprite_alive,(ix+sprite_collisions)
    ret z ; end of stack so we're done
; check tile type
    ld a,(ix+tile_type)
    cp tile_lightning ; check if tile is lightning
    jp nz,@next_record ; if not an electrode, skip to next record
; check tile position
    ld iy,(@tile_stack_pointer)
    ld (tile_stack_pointer),iy
    ld iy,(iy) ; pointer to tile record to kill
    ld (tile_table_pointer),iy
    ld hl,(ix+tile_y)
    ld de,(iy+tile_y)
    or a ; clear carry
    sbc hl,de ; if not at the same position, skip to next record
    jp nz,@next_record
; deactivate tile and replace with inactive blank tile
    ld bc,(iy+tile_x) ; get x position (ud.e already has y-position)
    call vdu_plot_bmp168 ; plot blank tile (bitmap was activated at beginning of proc)
    call table_deactivate_tile
    ld ix,(@tile_stack_pointer) ; get back stack pointer
    jp @check_loop ; deactivate tile naturally advances to next record
 ; get back stack pointer and loop
@next_record:
    ld ix,(@tile_stack_pointer)
    lea ix,ix+3 ; next tile stack record
    jp @check_loop ; loop until we've checked all the records
@done:
; find this tile record in the tile stack
    pop de ; restore pointer to this tile table record (was hl)
    ld iy,tile_stack ; set ix to first record in table
@find_loop:
    ld hl,(iy)
    lea iy,iy+3 ; next tile stack record
    or a ; clear carry
    sbc hl,de ; if not the same address, loop
    jp nz,@find_loop
    lea iy,iy-3 ; back up to the record we want
    ld (tile_stack_pointer),iy ; update stack pointer
    ld iy,(iy) ; iy points to the tile stack record
    ld (tile_table_pointer),iy ; set pointer to this tile record
    jp activate_crater
@tile_stack_pointer: dl tile_stack ; pointer to current stack record, initialized to tile_stack
; end move_electrode