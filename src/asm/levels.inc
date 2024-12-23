    align 256

; tileset indices
tiles_dg: equ 0
; tiles_xevious: equ 1
tileset_ptr_idx: dl 0
tileset_ptrs:
    ; dl tiles_xevious_levels
    dl tileset_00_levels
tileset_ptrs_end: dl 0 ; list terminator

; get a pointer to the current tileset
; inputs: tileset_ptr_idx
; returns: ix = pointer to tileset levels lut
get_tileset_ptr:
    ld a,(tileset_ptr_idx)
    ld d,a ; d = tileset index
    ld e,3 ; three bytes per index record
    mlt de ; offset into tileset_ptrs lut
    ld ix,tileset_ptrs
    add ix,de ; ix = pointer to tileset_ptrs record
    ret
; end get_tileset_ptr

; intialize a tileset
; inputs: a = index into tileset_ptrs
init_tileset:
    ld (tileset_ptr_idx),a ; store new tileset index
    call get_tileset_ptr ; ix = pointer to tileset_ptrs record
    ld ix,(ix) ; ix = pointer to tileset levels lut
    ld a,(ix-1) ; number of levels in the tileset
    ld (num_levels),a
    xor a ; reset current level to zero
    ld (cur_level),a
    jp init_level
; end init_tileset

next_tileset:
    ld hl,tileset_ptr_idx
    inc (hl) ; bump to next index
    call get_tileset_ptr ; ix = pointer to tileset_ptrs record
    ld hl,(ix) ; ix = pointer to tileset levels lut
    sign_hlu ; check for list terminator (0)
    jp nz,@F ; not end of list
    ld a,-1 ; will wrap around to zero on the next iteration
    ld (tileset_ptr_idx),a
    jp next_tileset ; try again
@@:
    jp init_tileset
; end next_tileset

cur_level: db 0
num_levels: db 0

; get a pointer to the current level lookup record
; inputs: tileset_ptr_idx, cur_level
; returns: ix = pointer to level definition
get_level_ptr:
    call get_tileset_ptr
    ld ix,(ix) ; ix = pointer to tileset levels lut
    ld a,(cur_level)
    ld d,a ; d = level index
    ld e,3 ; three bytes per level record
    mlt de ; offset into tileset levels lut
    add ix,de ; ix = pointer to level record
    ret
; end get_level_ptr

; advance to the next level in the current tileset
; inputs: tileset_ptr_idx, cur_level
next_level:
    ld hl,cur_level
    inc (hl) ; bump to next level
    call get_level_ptr ; ix points to current level lookup record
    ld hl,(ix) ; hl = pointer to next level lookup record
    sign_hlu ; check for list terminator (0)
    jp nz,@F ; not end of list
    xor a ; reset level to zero
    ld (cur_level),a
    jp next_tileset ; try next tileset
@@: ; fall through to init_level
; end next_level

; inputs: ix pointed to tileset level definition
init_level:
    call get_level_ptr ; ix points to level lookup record
    ld ix,(ix) ; ix points to level definition
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
; print_level:
level_x: equ 48
level_y: equ 0
    ld c,level_x
    ld b,level_y
    call vdu_move_cursor
    call printInline
    asciz "Level: "
    ld a,(cur_level)
    call printHexA
    ; call printNewLine
    ; ld hl,(cur_baseBufferId)
    ; call printDec

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