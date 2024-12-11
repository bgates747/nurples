max_enemy_sprites: db 4

; sprite_type
enemy_dead: equ 0
enemy_air: equ 1
enemy_ground: equ 2
enemy_projectile: equ 3
explosion: equ 255
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
    ld b,table_max_records ; loop counter
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
    callHL
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
    lea iy,iy+table_bytes_per_record
    xor a ; clears carry flag
    ld (sprite_screen_edge),a ; clear screen edge collision flag
    pop bc ; get back our loop counter
    djnz @move_loop ; loop until we've checked all the records
; reset all just spawned flags
    ld iy,table_base
    ld b,table_max_records
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
    lea iy,iy+table_bytes_per_record
    pop bc ; get back loop counter
    djnz @reset_just_spawned
; and done
    ret

; ; ######### SPRITE BEHAVIOR ROUTINES #########
; ; each sprite in the table must have one of these defined
; ; but they need not be unique to a particular sprite
; ; these are jumped to from move_enemies_do_program,but could come from other places
; ; and have the option but not obligation to go back to move_enemies_loop_return
; ; but they can call anything they want between those two endpoints
; move_programs: ; bookmark in case we want to know the first address of the first subroutine

; move_nop: ; does nothing but burn a few cycles changing the PC
;     ret

; move_explosion:
;     call animate_explosion 
;     ret

check_enemy_collisions:
; check what we collide with
    ld a,(iy+sprite_collisions)
    push af ; save collision flags
    and collides_with_player ; check for collisions with player
    call nz,check_enemy_collision_player
    pop af ; restore collision flags
    and collides_with_laser ; check for collisions with laser
    call nz,check_enemy_collision_laser
    ret
check_enemy_collision_laser: ; check for collisions with laser
    ld ix,laser_start_variables
    bit sprite_alive,(ix+sprite_collisions)
    ret z ; laser dead, nothing to do
    call check_collision
    ret p ; no collision
    ld ix,laser_start_variables
    set sprite_just_died,(ix+sprite_collisions)
    set sprite_alive,(ix+sprite_collisions)
    call enemy_kill
    ret
check_enemy_collision_player: ; check for collisions with player
    ld ix,player_start_variables
    bit sprite_alive,(ix+sprite_collisions)
    ret z ; player dead, no collision
    call check_collision
    ret p ; no collision
    call enemy_kill
    ret
; end check_enemy_collisions

; move_fireballs:
;     call move_sprite ; move sprite 
;     ld a,(sprite_screen_edge) ; check for collision with screen edge
;     and a ; if zero we're still within screen bounds
;     jp z,move_fireballs_alive
; ; otherwise kill sprite
;     ld a,%10000000 ; any bit set in high nibble means sprite will die
;     ld (iy+sprite_collisions),a
;     ret
; move_fireballs_alive:
;     ld a,(sprite_animation_timer)
;     dec a
;     ld (sprite_animation_timer),a
;     jp nz,move_fireballs_draw
;     ld a,(sprite_animation)
;     xor %1
;     ld (sprite_animation),a
;     ld a,6 ; 1/10th of a second timer
;     ld (sprite_animation_timer),a
;     ; fall through

; move_fireballs_draw:
;     call vdu_buff_select ; TODO: convert to vdu_buff_select
;     call vdu_bmp_draw ; convert to vdu_bmp_plot
;     call check_collisions
;     ret

; this routine vanquishes the enemy sprite
; and replaces it with an animated explosion
; we jump here instead of call because
; we want to return to differing locations in the loop
; depending on whether we're still sploding
; destroys: everything except index registers
; returns: a=-1 to indicate enemy died, and an incandescent ball of debris and gas
enemy_kill:
    ld iy,(table_pointer)
; tally up points
    ld hl,(player_score)
    ld bc,0 ; clear bcu
    ld c,(iy+sprite_points)
    add hl,bc
    ld (player_score),hl
; ; initialize explosion
; init_explosion:
;     ld hl,explosion
;     ld (sprite_base_bufferId),hl
;     ld hl,move_explosion
;     ld (sprite_move_program),hl
;     ld a,%00000000 ; collides with nothing
;     ld (iy+sprite_collisions),a
;     ld hl,0 ; north
;     ld (sprite_heading),hl
;     ld a,0x04 ; will decrement to 03
;     ld (sprite_animation),a
;     ld a,0x07 ; 7/60th of a second timer
;     ld (sprite_animation_timer),a
;     xor a
;     ld (sprite_move_timer),a
;     call vdu_buff_select ; TODO: convert to vdu_buff_select
; ; fall through to next_explosion
; next_explosion:
;     ld a,(sprite_animation)
;     dec a ; if rolled negative from zero,we're done sploding
;     jp m,done_explosion
;     ld (sprite_animation),a
;     ld a,0x7 ; 7/60th of a second timer
;     ld (sprite_animation_timer),a
; ; fall through to animate_explosion
; animate_explosion:
;     ld hl,sprite_y+1
;     inc (hl) ; move explosion down 1 pixel
;     jp z, done_explosion ; if wraparound to top of screen, kill explosion
;     ld hl,sprite_animation_timer
;     dec (hl) ; if timer is zero,we do next animation
;     jp z,next_explosion
;     ;otherwise we fall through to draw the current one
;     call vdu_buff_select ; TODO: convert to vdu_buff_select
;     call vdu_bmp_draw ; convert to vdu_bmp_plot
;     ret ; now we go back to caller
; done_explosion:
    set sprite_just_died,(iy+sprite_collisions)
    ld a,-1 ; return -1 to indicate enemy died
    ret ; now we go back to caller
; end enemy_kill