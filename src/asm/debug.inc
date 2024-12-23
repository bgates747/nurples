    align 256

DEBUG_PRINT:
    PUSH_ALL
    ld c,0 ; X
    ld b,0 ; Y
    call vdu_move_cursor
    POP_ALL
    PUSH_ALL
    call dumpFlags
    POP_ALL
    PUSH_ALL
    call dumpRegistersHex
    ; call waitKeypress
    POP_ALL
    ret

DEBUG_PRINT_TILE_TABLE:
    PUSH_ALL
    call printNewLine
    ld ix,tile_stack
    ld ix,(ix)
    call dump_tile_record
    call printNewLine
    POP_ALL
    ret
; end DEBUG_PRINT_TILE_TABLE

DEBUG_PRINT_TABLE:
    PUSH_ALL
    call printNewLine
    call dump_sprite_record
    call printNewLine
    call printNewLine

    push iy
    pop ix
    call dump_sprite_record
    call printNewLine
    call printNewLine
    POP_ALL
    RET

DEBUG_WAITKEYPRESS:
    PUSH_ALL
    call waitKeypress
    POP_ALL
    RET

DEBUG_PRINT_FIELDS:
    ; PUSH_ALL
    ld bc,0
    ld c,a
    ld ix,table_base
    add ix,bc
    ld b,table_num_records
@@:
    push ix
    pop hl
    push bc ; save loop counter
    ld a,1 ; print one byte
    call dumpMemoryHex
    lea ix,ix+table_record_size
    pop bc ; restore loop counter
    djnz @b
    ; POP_ALL
    ret

DEBUG_PRINT_TILE_STACK:
    PUSH_ALL
    call printNewLine
    call printNewLine
    ld hl,(tile_stack_pointer)
    call printHexUHL
    call printNewLine
    ld a,(num_active_tiles)
    call printHexA
    call printNewLine
    ld ix,tile_stack
    ld b,8
@loop:
    push bc
    ld hl,(ix)
    call printHexUHL
    call printNewLine
    lea ix,ix+3
    pop bc
    djnz @loop
    POP_ALL
    ret

DEBUG_DUMP_PLAYER_RECORD:
    PUSH_ALL
    call printNewLine
    CALL dump_player_record
    call printNewLine
    POP_ALL
    RET

DEBUG_PRINT_SHIELDS:
    ld c,0 ; X
    ld b,0 ; Y
    call vdu_move_cursor
    
    ld ix,bar_shields

    call printInline
    asciz "val:"
    ld hl,(ix+bar_val)
    call printDec
    call printNewLine

    call printInline
    asciz "x0 :"
    ld hl,(ix+bar_x0)
    call printDec
    call printNewLine

    call printInline
    asciz "y0 :"
    ld hl,(ix+bar_y0)
    call printDec
    call printNewLine

    call printInline
    asciz "x1 :"
    ld hl,(ix+bar_x1)
    call printDec
    call printNewLine

    call printInline
    asciz "y1 :"
    ld hl,(ix+bar_y1)
    call printDec
    call printNewLine

    call printInline
    asciz "y  :"
    ld hl,(ix+bar_y)
    call printDec
    call printNewLine

    ; call waitKeypress
    ret
; end DEBUG_PRINT_SHIELDS

DEBUG_PRINT_BAR_UPDATE:
    ld c,0 ; X
    ld b,0 ; Y
    call vdu_move_cursor

    call printInline
    asciz "x0  :"
    ld hl,0 ; clear hlu
    ld l,(ix+@x0-@cmd0+0)
    ld h,(ix+@x0-@cmd0+1)
    call printDec
    call printNewLine

    call printInline
    asciz "y0  :"
    ld hl,0 ; clear hlu
    ld l,(ix+@y0-@cmd0+0)
    ld h,(ix+@y0-@cmd0+1)
    call printDec
    call printNewLine

    call printInline
    asciz "x1  :"
    ld hl,0 ; clear hlu
    ld l,(ix+@x1-@cmd0+0)
    ld h,(ix+@x1-@cmd0+1)
    call printDec
    call printNewLine

    call printInline
    asciz "y1  :"
    ld hl,0 ; clear hlu
    ld l,(ix+@y1-@cmd0+0)
    ld h,(ix+@y1-@cmd0+1)
    call printDec
    call printNewLine

    call printInline
    asciz "x2  :"
    ld hl,0 ; clear hlu
    ld l,(ix+@x2-@cmd0+0)
    ld h,(ix+@x2-@cmd0+1)
    call printDec
    call printNewLine

    call printInline
    asciz "y2  :"
    ld hl,0 ; clear hlu
    ld l,(ix+@y2-@cmd0+0)
    ld h,(ix+@y2-@cmd0+1)
    call printDec
    ret
@cmd0:   db 24 
@x0: 	dw 0x0000 
@y1: 	dw 0x0000 
@x1: 	dw 0x0000 
@y0: 	dw 0x0000 
; plot bitmap command
@cmd1:   db 25 ; stomped on by @y0 load
@mode:  db plot_bmp+dr_abs_fg ; 0xED stomped on by @x2 load
@x2: 	dw 0x0000
@y2: 	dw 0x0000
@end:  ; no padding required
; end DEBUG_PRINT_BAR_UPDATE