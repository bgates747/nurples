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