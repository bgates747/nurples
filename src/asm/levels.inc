cur_level: db 0
num_levels: db 0

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

cur_tileset: dl tilesets
tilesets:
    dl tiles_dg_levels
    dl tiles_xevious_levels
    dl 0 ; list terminator

; intialize a tileset
; inputs: ix = pointer to tileset definition
init_tileset:
    ld ix,(cur_tileset)
    ld ix,(ix)
    ld a,(ix-1) ; number of levels in the tileset
    ld (num_levels),a
    xor a
    ld (cur_level),a
    call init_level
    ret
; end init_tileset

next_tileset:
    ld ix,(cur_tileset)
    lea ix,ix+3 ; ix points to next tileset
    ld hl,(ix)
    sign_hlu
    jp nz,@F
    ld ix,tilesets
@@:
    ld ix,(ix)
    call init_tileset
    ret