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
    ld bc,0
    ld de,0
    call vdu_set_gfx_origin
    ld bc,0
    ld de,0
    ld ix,127
    ld iy,240
    call vdu_set_gfx_viewport
    call vdu_clg
    call vdu_home_cursor
    call printNewLine
    call printNewLine
    ld hl,(tile_stack_pointer)
    call printHexUHL
    call printNewLine
    ld a,(num_active_tiles)
    call printHexA
    call printNewLine
    and a 
    jp z,@done
    ld ix,tile_stack
    ld a,(num_active_tiles)
    ld b,a
@loop:
    push bc
    push ix
    ld ix,(ix)
    ld a,(ix+tile_id)
    call printHexA
    ld a,(ix+tile_type)
    call printHexA
    call printNewLine
    pop ix
    lea ix,ix+3
    pop bc
    djnz @loop
@done:
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
    asciz "X2 y2:"
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
@end:  db 0x00 ; padding
; end DEBUG_PRINT_BAR_UPDATE


dump_sprite_record:
    ; call vdu_home_cursor
    call printInline
    asciz "id  : "
    ld a,(ix+sprite_id)
    call printHexA

    call printInline
    asciz "\r\nx   :"
    ld hl,(ix+sprite_x)
    call print_s168_hl

    call printInline
    asciz "\r\ny   :"
    ld hl,(ix+sprite_y)
    call print_s168_hl

    call printInline
    asciz "\r\nxv  :"
    ld hl,(ix+sprite_xvel)
    call print_s168_hl

    call printInline
    asciz "\r\nyv  :"
    ld hl,(ix+sprite_yvel)
    call print_s168_hl

    call printInline
    asciz "\r\nv   :"
    ld hl,(ix+sprite_vel)
    call print_s168_hl

    call printInline
    asciz "\r\nhdg :"
    ld hl,(ix+sprite_heading)
    call print_s168_hl

    call printInline
    asciz "\r\nori :"
    ld hl,(ix+sprite_orientation)
    call print_s168_hl
    
    call printInline
    asciz "\r\ntyp : "
    ld a,(ix+sprite_type)
    call printHexA
    
    call printInline
    asciz "\r\nBId : "
    ld hl,(ix+sprite_base_bufferId)
    call printHex24

    call printInline
    asciz "\r\nmPg : "
    ld hl,(ix+sprite_move_program)
    call printHex24

    call printInline
    asciz "\r\ncol : "
    ld a,(ix+sprite_collisions)
    call printBin8

    call printInline
    asciz "\r\nszx : "
    ld a,(ix+sprite_dim_x)
    call printHexA
    
    call printInline
    asciz "\r\nszy : "
    ld a,(ix+sprite_dim_y)
    call printHexA

    ; call printInline
    ; asciz "\r\n#ori: "
    ; ld a,(ix+sprite_num_orientations)
    ; call printHexA

    ; call printInline
    ; asciz "\r\n#ani: "
    ; ld a,(ix+sprite_num_animations)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nani : "
    ; ld a,(ix+sprite_animation)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nanit: "
    ; ld a,(ix+sprite_animation_timer)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nmvt : "
    ; ld a,(ix+sprite_move_timer)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nmvs : "
    ; ld a,(ix+sprite_move_step)
    ; call printHexA

    ; call printInline
    ; asciz "\r\npts : "
    ; ld a,(ix+sprite_points)
    ; call printHexA

    ; call printInline
    ; asciz "\r\ndmg : "
    ; ld a,(ix+sprite_shield_damage)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nfrm : "
    ; ld a,(ix+sprite_frame)
    ; call printHexA

    ; call printNewLine

    ; call waitKeypress
    ret

dump_tile_record:
    call printInline
    asciz "id  : "
    ld a,(ix+tile_id)
    call printHexA

    call printInline
    asciz "\r\nx   :"
    ld hl,(ix+tile_x)
    call print_s168_hl

    call printInline
    asciz "\r\ny   :"
    ld hl,(ix+tile_y)
    call print_s168_hl

    call printInline
    asciz "\r\nxv  :"
    ld hl,(ix+tile_xvel)
    call print_s168_hl

    call printInline
    asciz "\r\nyv  :"
    ld hl,(ix+tile_yvel)
    call print_s168_hl

    call printInline
    asciz "\r\nv   :"
    ld hl,(ix+tile_vel)
    call print_s168_hl

    call printInline
    asciz "\r\nhdg :"
    ld hl,(ix+tile_heading)
    call print_s168_hl

    call printInline
    asciz "\r\nori :"
    ld hl,(ix+tile_orientation)
    call print_s168_hl
    
    call printInline
    asciz "\r\ntyp : "
    ld a,(ix+tile_type)
    call printHexA
    
    call printInline
    asciz "\r\nBId : "
    ld hl,(ix+tile_base_bufferId)
    call printHex24

    call printInline
    asciz "\r\nmPg : "
    ld hl,(ix+tile_move_program)
    call printHex24

    call printInline
    asciz "\r\ncol : "
    ld a,(ix+tile_collisions)
    call printBin8

    ; call printInline
    ; asciz "\r\nszx : "
    ; ld a,(ix+tile_dim_x)
    ; call printHexA
    
    ; call printInline
    ; asciz "\r\nszy : "
    ; ld a,(ix+tile_dim_y)
    ; call printHexA

    ; call printInline
    ; asciz "\r\n#ori: "
    ; ld a,(ix+tile_num_orientations)
    ; call printHexA

    ; call printInline
    ; asciz "\r\n#ani: "
    ; ld a,(ix+tile_num_animations)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nani : "
    ; ld a,(ix+tile_animation)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nanit: "
    ; ld a,(ix+tile_animation_timer)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nmvt : "
    ; ld a,(ix+tile_move_timer)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nmvs : "
    ; ld a,(ix+tile_move_step)
    ; call printHexA

    ; call printInline
    ; asciz "\r\npts : "
    ; ld a,(ix+tile_points)
    ; call printHexA

    ; call printInline
    ; asciz "\r\ndmg : "
    ; ld a,(ix+tile_shield_damage)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nfrm : "
    ; ld a,(ix+tile_frame)
    ; call printHexA

    ; call printNewLine
    ; call printNewLine

    ; call waitKeypress
    ret