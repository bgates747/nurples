max_enemy_sprites: db 4

move_enemies:
; initialize pointers and loop counter
    ld iy,enemy_sprites_begin ; set iy to first record in table
    ld b,table_num_enemy_records ; loop counter
@move_loop:
    ld (table_pointer),iy ; update table pointer
    push bc ; backup loop counter
; check top bit of sprite_type to see if sprite is just spawned
    bit 7,(iy+sprite_type)
    jp nz,@next_record ; if just spawned, skip to next record
; check whether sprite is active
    bit sprite_alive,(iy+sprite_collisions)
    jp z,@next_record ; not active, skip to next record
; otherwise we prepare to move the sprite
    ld hl,(iy+sprite_move_program) ; load the behavior subroutine address
    CALL_HL
; move_enemies_loop_return: return from behavior subroutines
    ld iy,(table_pointer) ; get back table pointer
; now we check results of all the moves
    bit sprite_alive,(iy+sprite_collisions)
    jp nz,@draw_sprite ; if not dead, draw sprite
    call table_deactivate_sprite 
    jp @next_record ; and to the next record
@draw_sprite:
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
    ld iy,table_base
    ld b,table_num_records ; loop counter
@reset_just_spawned:
    push bc ; back up loop counter
    bit 7,(iy+sprite_type) ; check if just spawned
    jp z,@F
    res 7,(iy+sprite_type) ; clear just spawned flag
    ld a,(iy+sprite_id)
    call vdu_sprite_select
    ld hl,(iy+sprite_base_bufferId)
    ld b,(iy+sprite_num_animations)
    ld c,(iy+sprite_num_orientations)
    mlt bc ; bc total frames
    call sprite_load_frames
    ld a,(iy+sprite_frame)
    call vdu_sprite_select_frame
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    call vdu_sprite_move_abs168
    call vdu_sprite_show
@@:
    lea iy,iy+table_record_size
    pop bc ; get back loop counter
    djnz @reset_just_spawned
; and done
    ret