tiles_row_defs: dl 0x000000 ; pointer to current row tiles definitions
tiles_row: dl 0 ; decrements each time a source row is admitted
    ; initialize to zero for a maximum of 256 rows in a level
tiles_cols: db 0 ; number of tile columns in the current level
tiles_level_complete: db 0 ; latched only once the last terrain is off screen
tiles_drain_remaining: dl 0 ; remaining one-pixel scrolls after final row admission

cur_baseBufferId: dl 0
    db 0 ; padding for byte-shifting
tiles_x_plot: dl 0
    db 0 ; padding for byte-shifting
tiles_y_plot: dl -15
tiles_visual_row: ds 16 ; render IDs only; activation still reads the source row

; GAMEPLAY-ONLY TERRAIN PIPELINE.
; This loop is deliberately synchronized to one scanline per frame and to the
; 16x16 terrain-row admission/activation cadence below. Do not generalize it to
; multi-scanline scrolling or reuse it for level-transition hyperwarp effects.
; Level transitions must wait for terrain to clear, then use the separate
; starfield-only transition scroller.
tiles_plot:
; The terrain pipeline consumes the published player lifecycle itself. Death,
; entrance and grace keep the existing field spatially fixed without teaching
; the global scheduler how to rewrite tile state.
    call player_world_travel_enabled
    jp c,@travel
    xor a
    ret
@travel:
    ld a,(tiles_level_complete)
    or a
    ret nz
    call tiles_scroll_background
    ld hl,(tiles_drain_remaining)
    ld a,h
    or l ; bounded by the 336-pixel playfield, so H:L covers the counter
    jp nz,@drain
; initialize tiles loop
    ld hl,0 ; init plotting x-coordinate
    ld (tiles_x_plot),hl
    ld hl,(tiles_row_defs)
; Resolve shared pad artwork once per incoming row, not per scanline.
    ld a,(tiles_y_plot)
    cp 0xF1 ; -15, first scanline of every new terrain row
    jp nz,@visual_ready
    ld de,tiles_visual_row
    ld a,(tiles_cols)
    ld b,a
@resolve_art:
    ld a,(hl)
    inc hl
    cp tile_activation_routines_count
    jp nc,@store_art
    push hl
    ld hl,0
    ld l,a
    push de
    ld de,tile_active_art
    add hl,de
    ld a,(hl)
    pop de
    pop hl
@store_art:
    ld (de),a
    inc de
    djnz @resolve_art
@visual_ready:
    ld hl,tiles_visual_row
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
    jp z,@next_row
    xor a ; no level-complete result this frame
    ret
@next_row:
; do active tiles
    ld hl,(tiles_row_defs)
    ld de,0
    ld a,(tiles_cols)
    ld e,a
    add hl,de
    push hl ; save pointer to next source row, not end of visual cache
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
    jp z,@begin_drain
    xor a ; no level-complete result this frame
    ret
@begin_drain:
; The last row's top scanline is at Y=0. Count scrolls through Y=field_bottom
; and one beyond it. Do not revisit the source row pointer while draining.
    ld hl,field_bottom+1
    ld (tiles_drain_remaining),hl
    xor a
    ret
@drain:
    dec hl
    ld (tiles_drain_remaining),hl
    ld a,h
    or l
    jp z,@level_complete
    xor a
    ret
@level_complete:
    ld a,1
    ld (tiles_level_complete),a
    ret
; end tiles_plot

terrain_step_pixels: db 0 ; actual downward terrain travel this logical frame
tiles_scroll_background:
; GAMEPLAY ONLY: this one-scanline scroll and one-scanline redraw are coupled to
; tiles_plot's optimized terrain cadence. Do not pass a variable speed here and
; do not call this helper for fast level-transition scrolling.
; set gfx origin to playing field window
    ld bc,origin_left
    ld de,origin_top
    call vdu_set_gfx_origin
; set gfx viewport to playing field window
    call set_gfx_playing_field
; move the background down one pixel
    ld l,2 ; direction=down
    ld h,1 ; speed=1 px
    ld a,h
    ld (terrain_step_pixels),a
    ld a,2 ; current gfx viewport
    call vdu_scroll_down
; set gfx viewport to one scanline to optimise plotting tiles
    ld bc,0 ; leftmost x-coord
    ld de,0 ; topmost y-coord
    ld ix,255 ; rightmost x-coord
    ld iy,0 ; bottommost y-coord
    call vdu_set_gfx_viewport
; plot the background
terrain_bg_call_operand: equ $+1 ; level-entry binding; same CALL, no dispatch overhead
    call bg_plot
    ret
; end tiles_scroll_background

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
    cp tile_activation_routines_count ; carry reset if active tile
    jp nc,@next_tile ; skip to next tile if not active
; call the tile activation routine
    ld hl,tile_activation_routines
    ld de,0 ; MLT writes DE[15:0] only; clear DEU before pointer arithmetic
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

bg_y_plot: dl -1
bg_bufferId: dl BUF_BG_STARS_0 ; default bufferId for background tiles

bg_plot:
; load current bg tile bufferId as default
    ld hl,(bg_bufferId)
; check whether it is time for a new background tile
    ld a,(bg_y_plot)
    inc a
    ld (bg_y_plot),a
    jp nz,@F ; not time for a new tile so draw current one
; load the next background tile
    ld hl,BUF_BG_STARS_0
    call rand_8
    and %0000011
    add a,l
    ld l,a
    ld a,0
    adc a,h
    ld h,a
    ld (bg_bufferId),hl
@@:
    call vdu_buff_select
    ld bc,0
    ld de,(bg_y_plot)
    inc de
    call vdu_plot_bmp
    ret
; end bg_plot
