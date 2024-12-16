    align 256
tiles_row_defs: dl 0x000000 ; pointer to current row tiles definitions
tiles_row: dl 0 ; decrements each time a row is drawn. level is over when hits zero
    ; initialize to zero for a maximum of 256 rows in a level
tiles_cols: db 0 ; number of tile columns in the current level

cur_baseBufferId: dl 0
    db 0 ; padding for byte-shifting
tiles_x_plot: dl 0
    db 0 ; padding for byte-shifting
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
; ; plot the background
;     call bg_plot
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
; (we use ix b/c we want to preserve hl for the next step)
    ld ix,(tiles_y_plot)
    ld de,1
    add ix,de
    ld (tiles_y_plot),ix
    ld a,ixl
    cp 1
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
    CALL_HL ; call the activation routine
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

; bg_y_plot: dl -1
; bg_bufferId: dl BUF_STATION_BG_00 ; default bufferId for background tiles

bg_plot: ; TODO: implement this
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
; all done
    ret