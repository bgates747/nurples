; ######### TILES ######### 
; TODO: implement buffering of tiles here when there isn't other stuff to do
; tiles_defs: ds 256*16 ; 256 rows of 16 tiles, each tile is a byte
tiles_row_defs: dl 0x000000 ; pointer to current row tiles definitions
tiles_row: dl 0 ; decrements each time a row is drawn. level is over when hits zero
    ; initialize to zero for a maximum of 256 rows in a level
tiles_cols: db 0 ; number of tile columns in the current level
cur_level: db 0
num_levels: db 0
cur_baseBufferId: dl 0
tiles_x_plot: dl 0
tiles_y_plot: dl -15
cur_tileset: dl 0

; intialize a tileset
; inputs: ix = pointer to tileset definition
init_tileset:
    ld a,(ix-1) ; number of levels in the tileset
    ld (num_levels),a
    ld (cur_tileset),ix
    xor a
    ld (cur_level),a
    ret
; end init_tileset

init_level:
    ; call vdu_home_cursor ; DEBUG
; look up address of level's tile defintion
    ld a,(cur_level)
    ld d,a
    ld e,3
    mlt de ; de = index into tileset lut
    ld ix,(cur_tileset)
    add ix,de ; ix = pointer to level lookup record
    ld ix,(ix) ; ix = pointer to level definintion
; set tiles_cols
    ld a,(ix+0)
    ld (tiles_cols),a
; set tiles_row counter
    ld a,(ix+1)
    ld (tiles_row),a
; set cur_baseBufferId
    ld hl,(ix+4)
    ld (cur_baseBufferId),hl
; set tiles_row_defs
    lea ix,ix+7 ; ix points to definition of first tile of first row
    ld (tiles_row_defs),ix

; print level
level_x: equ 0
level_y: equ 0
    ld c,level_x
    ld b,level_y
    call vdu_move_cursor
    call printInline
    asciz "Level: "
    ld a,(cur_level)
    call printHexA
    call printNewLine
    ld hl,(cur_baseBufferId)
    call printDec

; ; DEBUG
;     call dumpRegistersHex
;     call waitKeypress
; ; END DEBUG

; ; draw initial background tiles 
; ; TODO: make this a stars background
;     ld hl,BUF_STATION_BG_00
;     call vdu_buff_select
;     ld bc,0
;     ld de,field_top
;     call vdu_plot_bmp

;     ld hl,BUF_STATION_BG_01
;     call vdu_buff_select
;     ld bc,0
;     ld de,field_top+256
;     call vdu_plot_bmp

    ret
; end init_level

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
; queue up next level
    ld hl,cur_level
    inc (hl) ; bump to next level
    ld a,(num_levels)
    cp (hl) ; are we past max levels?
    jp nz,@inc_level ; no, so load next level
    xor a ; reset to first level
    ld (hl),a ; and fall through to @inc_level
@inc_level:
; increase the number of enemy sprites
    ld a,(max_enemy_sprites)
    inc a
    cp table_max_records ; if we're at the global limit,skip ahead at max level
    jp z,init_level
    ld (max_enemy_sprites),a ; otherwise save the updated number
    jp init_level
; end tiles_plot

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

; xevious_plot:
; ; load current bg tile bufferId as default
;     ld hl,(bg_bufferId)
; ; check whether it is time for a new background tile
;     ld a,(bg_y_plot)
;     inc a
;     ld (bg_y_plot),a
;     jp nz,@F ; not time for a new tile so draw current one
; ; load the next background tile
;     ld hl,Xevious_tile
;     inc (hl)
;     ld a,%00011111 ; 31
;     and (hl)
;     ld hl,BUF_XEVIOUS_00
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
; Xevious_tile: db 0

; ###### TODO: NEW CODE TO IMPLEMENT ######
; dt_is_active:
; ; a lands here containing a tile index in the low nibble
; ; we test the values for the tiles which are active
;     cp #07
;     call z,ld_act_landing_pad
;     cp #08
;     call z,ld_act_laser_turret
;     ; fall through
;     ret

; ; some tiles become active sprites,so we load those here
; ; sprite_x/y have already been loaded
; ; sprite_dim_x/y are loaded by table_add_record
; ; we don't want sprite drawn to background like other tiles
; ; so this routine only adds them to the sprite table
; dt_ld_act:
;     ld a,#48 ; top of screen + 1/2 tile height
;     ld (sprite_y+1),a ; just the integer part
;     ld (sprite_base_bufferId),hl
;     call vdu_bmp_select ; TODO: convert to vdu_buff_select
;     call table_add_record
;     call sprite_variables_from_stack
;     ld a,#FF ; lets calling proc know we loaded an active tile
;     ret ; and back

; ld_act_landing_pad:
;     call sprite_variables_to_stack

;     ld hl,move_landing_pad
;     ld (sprite_move_program),hl

;     xor a 
;     ld (sprite_animation),a ; animation 0

;     call rand_8     ; snag a random number
;     and %00011111   ; keep only 5 lowest bits (max 31)
;     add a,64 ; range is now 64-127
;     ld (sprite_move_timer),a ; when this hits zero,will spawn an enemy

;     ld a,%10 ; collides with laser but not player
;     ld (iy+sprite_collisions),a 

;     ld a,#05 ; BCD
;     ld (sprite_points),a
;     ld a,0 ; binary
;     ld (sprite_shield_damage),a

;     ld hl,landing_pad ; dt_ld_act loads this to sprite_base_bufferId
;     jr dt_ld_act

; ld_act_laser_turret:
;     call sprite_variables_to_stack

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

;     ld hl,laser_turret ; dt_ld_act loads this to sprite_base_bufferId
;     jp dt_ld_act


; moves active tile sprites down one pixel in sync with tiles movement
; deletes sprites from table when they wrap around to top of screen
move_active_tiles:
; get current position
    ld a,(sprite_y+1) ; we only need the integer part
    inc a 
; are we at the bottom of the screen?
    jr nz,move_active_tiles_draw_sprite ; nope
; otherwise kill sprite
    ld a,%10000000 ; any bit set in high nibble means sprite will die
    ld (iy+sprite_collisions),a
    ret ; debug
move_active_tiles_draw_sprite:
    ld (sprite_y+1),a ; update tile y position integer part
    ;     call vdu_bmp_select ; TODO: convert to vdu_buff_select
    ; call vdu_bmp_draw ; convert to vdu_bmp_plot ; draw it
    ret ; and done


; move_landing_pad:
;     call move_active_tiles
;     call check_collisions
; ; is it time to launch an enemy?
;     ld hl,sprite_move_timer
;     dec (hl)
;     jp nz,move_enemies_loop_return
;     call enemy_init_from_landing_pad
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
;     call sprite_variables_to_stack

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
;     call sprite_variables_from_stack ; come back to where we started
;     ret