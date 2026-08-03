electrode_defaults:
    db tile_electrode       ; type
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
    ld ix,player_begin
    bit sprite_alive,(ix+sprite_collisions)
    jp z,@laser ; player is dead so don't check for collisions
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
; Preserve the source record itself. Removing earlier stack entries may change
; its dense-stack slot, so the slot is rediscovered after the scan.
    ld (@source_tile_pointer),iy
    ld hl,(iy+tile_y)
    ld (@tile_y),hl
; activate blank tile bitmap only after source data no longer depends on IY
    ld hl,BUF_TILE_BLANK
    call vdu_buff_select
; Bound the scan to the entry count at its start. Each processed or removed
; entry consumes one unit; removal leaves IX on the shifted successor.
    ld a,(num_active_tiles)
    ld (@tiles_remaining),a
    ld ix,tile_stack ; set ix to first record in table
@check_loop:
    ld a,(@tiles_remaining)
    or a
    jp z,@scan_done
    dec a
    ld (@tiles_remaining),a
; validate the stack slot before loading its pointer
    push ix
    pop hl
    call tile_stack_pointer_valid
    jp nc,@scan_done
    ld (@tile_stack_pointer),ix ; update stack pointer
; validate the record before reading any fields
    ld iy,(ix)
    call tile_table_record_pointer_valid
    jp nc,@bad_record
; check top bit of tile_type to see if tile is just spawned
    bit 7,(iy+tile_type)
    jp nz,@next_record ; if just spawned, skip to next record
; check sprite_alive bit to see if tile is active
    bit sprite_alive,(iy+tile_collisions)
    jp z,@inactive_record
; check tile type
    ld a,(iy+tile_type)
    cp tile_lightning ; check if tile is lightning
    jp nz,@next_record ; if not lightning, skip to next record
; check tile position
    ld (tile_stack_pointer),ix
    ld (tile_table_pointer),iy
    ld hl,(iy+tile_y)
    ld de,(@tile_y)
    or a ; clear carry
    push hl
    sbc hl,de
    pop hl
    jp z,@kill_it
    ld bc,1*256
    add hl,bc
    or a
    sbc hl,de
    jp nz,@next_record
@kill_it: ; deactivate tile and replace with inactive blank tile
    ld bc,(iy+tile_x) ; get x position (ud.e already has y-position)
    push iy
    call vdu_plot_bmp168 ; plot blank tile (bitmap was activated at beginning of proc)
    pop iy
    call table_deactivate_tile
    ld ix,(@tile_stack_pointer) ; get back stack pointer
    jp @check_loop ; deactivate tile naturally advances to next record
@inactive_record:
; A valid inactive record violates the dense-stack invariant but is safe to
; remove. Keep IX on this slot so the shifted successor is checked next.
    ld (tile_stack_pointer),ix
    call table_deactivate_tile
    ld ix,(@tile_stack_pointer)
    jp @check_loop
@bad_record:
; The slot itself was validated, so it can be quarantined without touching the
; malformed record pointer. Continue at the successor shifted into this slot.
    ld hl,tile_fault_flags
    set tile_fault_bad_record,(hl)
    ld (tile_stack_pointer),ix
    call tile_stack_quarantine_entry
    ld ix,(@tile_stack_pointer)
    jp @check_loop
 ; get back stack pointer and loop
@next_record:
    ld ix,(@tile_stack_pointer)
    lea ix,ix+3 ; next tile stack record
    jp @check_loop ; loop until we've checked all the records
@scan_done:
; Rediscover the source cursor after any compaction. This search is bounded by
; the current dense-stack count and validates each slot before loading it.
    ld a,(num_active_tiles)
    ld (@tiles_remaining),a
    ld iy,tile_stack
@find_loop:
    ld a,(@tiles_remaining)
    or a
    jp z,@source_not_found
    dec a
    ld (@tiles_remaining),a
    push iy
    pop hl
    call tile_stack_pointer_valid
    jp nc,@source_not_found
    ld hl,(iy)
    lea iy,iy+3 ; next tile stack record
    ld de,(@source_tile_pointer)
    or a ; clear carry
    sbc hl,de ; if not the same address, loop
    jp nz,@find_loop
    lea iy,iy-3 ; back up to the record we want
    ld (tile_stack_pointer),iy ; update stack pointer
    ld iy,(iy) ; iy points to the tile stack record
    call tile_table_record_pointer_valid
    jp nc,@source_not_found
    ld (tile_table_pointer),iy ; set pointer to this tile record
    jp transform_active_tile_to_crater
@source_not_found:
; Fail closed and make the outer traversal's normal +3 step land exactly on
; the current logical end. It will then stop on the validated range check.
    ld hl,tile_fault_flags
    set tile_fault_bad_record,(hl)
    ld a,(num_active_tiles)
    ld hl,0
    ld h,a
    ld l,3
    mlt hl
    ld de,tile_stack-3
    add hl,de
    ld (tile_stack_pointer),hl
    ret
@tile_stack_pointer: dl tile_stack ; pointer to current stack record, initialized to tile_stack
@tile_y: dl 0 ; y position of tiles to kill
@source_tile_pointer: dl 0
@tiles_remaining: db 0
; end move_electrode
