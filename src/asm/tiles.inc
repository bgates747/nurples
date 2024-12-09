; ######### TILES ######### 
; TODO: implement buffering of tiles here when there isn't other stuff to do
; tiles_defs: ds 256*16 ; 256 rows of 16 tiles, each tile is a byte
tiles_row_defs: dl 0x000000 ; pointer to current row tiles definitions
tiles_row: dl 0 ; decrements each time a row is drawn. level is over when hits zero
    ; initialize to zero for a maximum of 256 rows in a level
tiles_cols: db 0 ; number of tile columns in the current level

cur_baseBufferId: dl 0
tiles_x_plot: dl 0
tiles_y_plot: dl -15

tiles_plot:
; set gfx viewport to playing field window
    ld bc,field_left
    ld de,field_top
    ld ix,field_right
    ld iy,field_bottom
    call vdu_set_gfx_viewport
; move the background down one pixel
    ld a,2 ; current gfx viewport
    ld l,2 ; direction=down
    ld h,1 ; speed=1 px
    call vdu_scroll_down
; set gfx viewport to one scanline to optimise plotting tiles
    ld bc,0 ; leftmost x-coord
    ld de,0 ; topmost y-coord
    ld ix,255 ; rightmost x-coord
    ld iy,0 ; bottommost y-coord
    call vdu_set_gfx_viewport
; plot the background
    ; call bg_plot
    ; call xevious_plot
    ; ret ; DEBUG
; initialize tiles loop
    ld hl,0 ; init plotting x-coordinate
    ld (tiles_x_plot),hl
    ld hl,(tiles_row_defs)
    ld a,(tiles_cols)
    ld b,a ; column loop counter
@loop:
    push bc ; save the loop counter
; read the tile defintion for the current column
    ld a,(hl) ; a has tile definition
    push hl ; save pointer to tile definition
    ld hl,(cur_baseBufferId) ; hl base buffer id of tileset
    ld l,a ; a is the offset from the base so hl is now the bufferId of the tile
    call vdu_buff_select ; tile bitmap buffer is now active
; plot the active bitmap
    ld bc,(tiles_x_plot)
    ld de,(tiles_y_plot)
    call vdu_plot_bmp
; bump x-coords the width of one tile and save it
    ld hl,(tiles_x_plot)
    ld bc,16
    add hl,bc
    ld (tiles_x_plot),hl
; prepare to loop to next column
    pop hl ; get back pointer to tile def
    inc hl ; bump it to the next column
    pop bc ; snag our loop counter
    djnz @loop
; increment tiles plotting y-coordinate
; when it hits zero, we go to next row of tiles in the map
; (we use ix b/c we want to preserve hl for the next step)
    ld ix,tiles_y_plot
    inc (ix)
    ret nz
; do active tiles
    push hl ; save pointer to next row tile def
    call do_active_tiles
    pop hl ; get back pointer to next row tile def
; time to bump tiles_row_defs to next row
; (hl was already there at the end of the loop)
    ld (tiles_row_defs),hl
; reset coords to plot next row of tiles
    ld hl,0
    ld (tiles_x_plot),hl
    ld hl,-15
    ld (tiles_y_plot),hl
; decrement tiles row counter
    ld hl,tiles_row
    dec (hl)
    ret nz
@inc_level:
; increase the number of enemy sprites
    ld a,(max_enemy_sprites)
    inc a
    cp table_max_records ; if we're at the global limit,skip ahead at max level
    jp z,next_level
    ld (max_enemy_sprites),a ; otherwise save the updated number
    jp next_level
; end tiles_plot

do_active_tiles:
; initialize tiles loop
    ld hl,0 ; init plotting x-coordinate
    ld (tiles_x_plot),hl
    ld hl,(tiles_row_defs)
    ld a,(tiles_cols)
    ld b,a ; column loop counter
@loop:
    push bc ; save the loop counter
; read the tile defintion for the current column
    ld a,(hl) ; a has tile definition
    push hl ; save pointer to tile definition
    or a ; check for null tile
    jp z,@next_tile ; skip to next tile if null
    cp 17 ; carry reset if active tile
    jp nc,@next_tile ; skip to next tile if not active
; call the tile activation routine
    ld hl,tile_activation_routines
    ld d,3 ; three bytes per table entry
    ld e,a ; a is the tile type
    mlt de ; de is now the offset to the table entry
    add hl,de ; hl is now the address of the table entry
    ld hl,(hl) ; hl is now the activation routine address
    callHL ; call the activation routine
@next_tile:
; bump x-coords the width of one tile and save it
    ld hl,(tiles_x_plot)
    ld bc,16
    add hl,bc
    ld (tiles_x_plot),hl
; prepare to loop to next column
    pop hl ; get back pointer to tile def
    inc hl ; bump it to the next column
    pop bc ; snag our loop counter
    djnz @loop
    ret ; done
; end do_active_tiles

; create a new sprite at the same location as an active tile
; inputs: iy = pointer to new sprite default settings, tiles_x_plot, tiles_y_plot set
spawn_sprite_from_tile:
    call table_get_next_id ; if available: a = new sprite id, ix = pointer to new sprite vars
    ret nc ; no carry means no free sprite slots, so we go home
; copy default settings to new sprite
    push iy ; pointer to new sprite defaults
    pop hl ; address to copy from
    push ix ; back up pointer to new sprite table record
    lea ix,ix+sprite_definitions ; bump pointer to copy to
    push ix ; pointer to default sprite settings
    pop de ; address to copy to
    ld bc,table_bytes_per_record ; number of bytes to copy
    ldir ; copies defaults to new sprite record
    pop ix ; restore pointer to top of new sprite table record
; initialize the new sprite
    ld a,(ix+sprite_id)
    call vdu_sprite_select ; select sprite
    ld hl,(ix+sprite_base_bufferId)
    ld b,(ix+sprite_num_animations)
    call sprite_load_frames
    ld hl,1*256 ; set y velocity to 1 pixel per frame
    ld (ix+sprite_yvel),hl
; update the new sprite's position and make it visible
    ld hl,(tiles_x_plot)
    hlu_mul256
    push hl
    pop bc
    ld (ix+sprite_x),bc
    ld de,-1*256 ; TODO: implement y-coord for reals
    ld (ix+sprite_y),de
    call vdu_sprite_move_abs168
    call vdu_sprite_show
    ret
; end spawn_sprite_from_tile

move_active_tile:
    ld a,(iy+sprite_id)
    call vdu_sprite_select
; y-axis movement
    ld hl,(iy+sprite_y)
    ld de,(iy+sprite_yvel)
    add hl,de
    ld (iy+sprite_y),hl
; detect screen edge collisions
    xor a ; reset screen edge collision flag and clear carry
    ld (sprite_screen_edge),a
; y-axis bottom is all we need to test
    ld de,-sprite_bottom*256
    adc hl,de    
    jp p,@bottom ; scrolled off screen so deactivate
    ret ; still on screen so do nothing further
@bottom:
    ld a,collide_bottom
    ld (sprite_screen_edge),a
    ret


; bg_y_plot: dl -1
; bg_bufferId: dl BUF_STATION_BG_00 ; default bufferId for background tiles

; bg_plot:
; ; load current bg tile bufferId as default
;     ld hl,(bg_bufferId)
; ; check whether it is time for a new background tile
;     ld a,(bg_y_plot)
;     inc a
;     ld (bg_y_plot),a
;     jp nz,@F ; not time for a new tile so draw current one
; ; load the next background tile
;     ld hl,BUF_STATION_BG_00
;     call rand_8
;     and %0000011
;     add a,l
;     ld l,a
;     ld a,0
;     adc a,h
;     ld h,a
;     ld (bg_bufferId),hl
; @@:
;     call vdu_buff_select
;     ld bc,0
;     ld de,(bg_y_plot)
;     inc de
;     call vdu_plot_bmp
; ; all done
;     ret

; ###### TODO: NEW CODE TO IMPLEMENT ######
tile_pad_small: equ 1
tile_pad_med: equ 2
tile_pad_large: equ 3
tile_pad_huge: equ 4
tile_fireball_turret: equ 5
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
    dl activate_null_tile  ;  activate_fireball_turret
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

; ld_act_laser_turret:
;     ; call sprite_variables_to_stack

;     ld hl,move_laser_turret
;     ld (sprite_move_program),hl

;     xor a 
;     ld (sprite_animation),a
;     ld (sprite_move_step),a

;     call rand_8     ; snag a random number
;     and %00011111   ; keep only 5 lowest bits (max 31)
;     add a,64 ; range is now 64-127
;     ld (sprite_move_timer),a ; when this hits zero,will spawn a fireball

;     ld a,%10 ; collides with laser but not player
;     ld (iy+sprite_collisions),a 

;     ld a,#10 ; BCD
;     ld (sprite_points),a
;     ld a,0 ; binary
;     ld (sprite_shield_damage),a

;     ld hl,tile_fireball_turret ; dt_ld_act loads this to sprite_base_bufferId
;     jp dt_ld_act


; ; moves active tile sprites down one pixel in sync with tiles movement
; ; deletes sprites from table when they wrap around to top of screen
; move_active_tiles:
; ; get current position
;     ld a,(sprite_y+1) ; we only need the integer part
;     inc a 
; ; are we at the bottom of the screen?
;     jr nz,move_active_tiles_draw_sprite ; nope
; ; otherwise kill sprite
;     ld a,%10000000 ; any bit set in high nibble means sprite will die
;     ld (iy+sprite_collisions),a
;     ret ; debug
; move_active_tiles_draw_sprite:
;     ld (sprite_y+1),a ; update tile y position integer part
;     ;     call vdu_buff_select ; TODO: convert to vdu_buff_select
;     ; call vdu_bmp_draw ; convert to vdu_bmp_plot ; draw it
;     ret ; and done


; move_landing_pad:
;     call move_active_tiles
;     call check_collisions
; ; is it time to launch an enemy?
;     ld hl,sprite_move_timer
;     dec (hl)
;     jp nz,move_enemies_loop_return
;     call enemy_spawn_seeker
;     ; reset move timer so can spawn again if player doesn't take us out
;     call rand_8     ; snag a random number
;     and %00011111   ; keep only 5 lowest bits (max 31)
;     add a,64 ; range is now 64-127
;     ld (sprite_move_timer),a ; when this hits zero,will spawn an enemy
;     ret


; move_laser_turret:
; ; compute orientation to player
;     call orientation_to_player
; ; h.l 8.8 fixed angle256 to player
; ; bc and de as signed 16-bit integers
; ; representing delta-x/y *to* target respectively
;     ld (Bearing_t),hl
;     ld hl,0x0400
;     ld (Vp),hl
;     call targeting_computer
;     ld (sprite_heading),hl ; store bearing to player
; ; is it time to launch a fireball?
;     ld hl,sprite_move_timer
;     dec (hl)
;     jp nz,move_laser_turret_boilerplate
;     call fireballs_init
;     ; reset move timer so can fire again if player doesn't take us out
;     call rand_8     ; snag a random number
;     and %00011111   ; keep only 5 lowest bits (max 31)
;     add a,64 ; range is now 64-127
;     ld (sprite_move_timer),a ; when this hits zero,will spawn a fireball
; move_laser_turret_boilerplate:
;     call move_active_tiles
;     call check_collisions
;     ret


; fireballs_init:
;     ; call sprite_variables_to_stack

;     ld hl,fireballs
;     ld (sprite_base_bufferId),hl

;     ld hl,move_fireballs
;     ld (sprite_move_program),hl 

;     ld a,%11 ; collides with laser and player
;     ; ld a,%10 ; collides with laser DEBUG
;     ld (iy+sprite_collisions),a

;     ld hl,(Vp)
;     ld (sprite_vel),hl
;     ld hl,(Vp_x)
;     ld (sprite_xvel),hl
;     ld hl,(Vp_y)
;     inc h ; account for ground movement
;     ld (sprite_yvel),hl

;     xor a ; zero a
;     ld (sprite_animation),a
;     ld (sprite_move_step),a
;     ld (sprite_move_timer),a

;     ld a,6 ; 1/10th of a second timer
;     ld (sprite_animation_timer),a

;     ld a,0x00 ; BCD
;     ld (sprite_points),a
;     ld a,1 ; binary
;     ld (sprite_shield_damage),a

;     call table_add_record ; plops that on the sprite stack for later
;     ; call sprite_variables_from_stack ; come back to where we started
;     ret