max_enemy_sprites: db 4

move_enemies:
; are there any active enemies or explosions?
    ld a,(table_active_sprites)
    and a ; will be zero if no alive enemies or explosions
    jp nz,@they_live
    ; DEPRECATED: though may be used in the future
    ; ld hl,enemies_dead
    ; ld (enemies_state),hl
    ; END DEPRECATED
    ret
@they_live:
; initialize pointers and loop counter
    ld iy,table_base ; set iy to first record in table
    ld b,table_num_records ; loop counter
@move_loop:
    ld (table_pointer),iy ; update table pointer
    push bc ; backup loop counter
; check top bit of sprite_type to see if sprite is just spawned
    ld a,(iy+sprite_type)
    bit 7,a
    jp nz,@next_record ; if just spawned, skip to next record
; check sprite_type to see if sprite is active
    and a ; if zero, sprite is dead 
    jp z,@next_record ; ... and we skip to next record
; otherwise we prepare to move the sprite
    ld hl,(iy+sprite_move_program) ; load the behavior subroutine address
    CALL_HL
; move_enemies_loop_return: return from behavior subroutines
    ld iy,(table_pointer) ; get back table pointer
; now we check results of all the moves
    bit sprite_just_died,(iy+sprite_collisions)
    jp z,@draw_sprite ; if not dead, draw sprite
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

; TODO: much if this is deprecated by new sprite table system and needs to be rewritten
check_enemy_collisions:
; ; check what we collide with
;     ld a,(iy+sprite_collisions)
;     push af ; save collision flags
;     and collides_with_player ; check for collisions with player
;     call nz,check_enemy_collision_player
;     pop af ; restore collision flags
;     and collides_with_laser ; check for collisions with laser
;     call nz,check_enemy_collision_laser
    ret
check_enemy_collision_laser: ; check for collisions with laser
    ; ld ix,laser_start_variables
    ; bit sprite_alive,(ix+sprite_collisions)
    ; ret z ; laser dead, nothing to do
    ; call check_collision
    ; ret p ; no collision
    ; ld ix,laser_start_variables
    ; set sprite_just_died,(ix+sprite_collisions)
    ; set sprite_alive,(ix+sprite_collisions)
    ; call enemy_kill
    ret
check_enemy_collision_player: ; check for collisions with player
    ; ld ix,player_begin
    ; bit sprite_alive,(ix+sprite_collisions)
    ; ret z ; player dead, no collision
    ; call check_collision
    ; ret p ; no collision
    ; call enemy_kill
    ret
; end check_enemy_collisions
; END TODO

; this routine vanquishes the enemy sprite
; and replaces it with an animated explosion
; destroys: everything except index registers
; returns: a=-1 to indicate enemy died, and an incandescent ball of debris and gas
enemy_kill:
    ld iy,(table_pointer)
    ld hl,(player_score)
    ld bc,0 ; clear bcu
    ld c,(iy+sprite_points)
    add hl,bc
    ld (player_score),hl
    jp spawn_explosion
; end enemy_kill