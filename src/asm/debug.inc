printHexA:
    push af
    push bc
    call printHex8
    ld a,' '
    rst.lil 10h
    pop bc
    pop af
    ret

printHexHL:
    push af
    push bc
    call printHex16
    ld a,' '
    rst.lil 10h
    pop bc
    pop af
    ret

printHexUHL:
    push af
    push bc
    call printHex24
    ld a,' '
    rst.lil 10h
    pop bc
    pop af
    ret

printHexAUHL:
    push af
    push bc
    call printHex8
    ld a,'.'
    rst.lil 10h
    call printHex24
    ld a,' '
    rst.lil 10h
    pop bc
    pop af
    ret

printHexABHL:
; preserve registers
    push bc ; b will be ok c will not
    push af ; will get totally destroyed
; print a
    call printHex8 
; print b
    ld a,b
    call printHex8
; print hl
    call printHex16
; restore registers
    pop af
    pop bc
    ret

printHexBHL:
; preserve registers
    push bc ; b will be ok c will not
    push af ; will get totally destroyed
; print b
    ld a,b
    call printHex8
; print hl
    call printHex16
; restore registers
    pop af
    pop bc
    ret

printHexCDE:
; preserve registers
    push bc ; b will be ok c will not
    push af ; will get totally destroyed
; print c
    ld a,c
    call printHex8
; print de
    ex de,hl
    call printHex16
    ex de,hl
; restore registers
    pop af
    pop bc
    ret

printHexUIX:
; store everything in scratch
    ld (uhl),hl
    ld (ubc),bc
    ld (ude),de
    ld (uix),ix
    ld (uiy),iy
    push af ; fml

    ld hl,str_ixu
    call printString
    ld hl,(uix)
    call printHex24
    call printNewLine

; restore everything
    ld hl, (uhl)
    ld bc, (ubc)
    ld de, (ude)
    ld ix, (uix)
    ld iy, (uiy)
    pop af
; all done
    ret


; print registers to screen in hexidecimal format
; inputs: none
; outputs: values of every register printed to screen
;    values of each register in global scratch memory
; destroys: nothing
stepRegistersHex:
; store everything in scratch
    ld (uhl),hl
    ld (ubc),bc
    ld (ude),de
    ld (uix),ix
    ld (uiy),iy
    push af ; fml
    pop hl ; thanks, zilog
    ld (uaf),hl
    push af ; dammit

; home the cursor
    ; call vdu_home_cursor

; print each register
    ld hl,str_afu
    call printString
    ld hl,(uaf)
    call printHex24
    call printNewLine

    ld hl,str_hlu
    call printString
    ld hl,(uhl)
    call printHex24
    call printNewLine

    ld hl,str_bcu
    call printString
    ld hl,(ubc)
    call printHex24
    call printNewLine

    ld hl,str_deu
    call printString
    ld hl,(ude)
    call printHex24
    call printNewLine

    ld hl,str_ixu
    call printString
    ld hl,(uix)
    call printHex24
    call printNewLine

    ld hl,str_iyu
    call printString
    ld hl,(uiy)
    call printHex24
    call printNewLine

    ; call vsync

    call printNewLine

; check for right shift key and quit if pressed
    MOSCALL mos_getkbmap
@stayhere:
; 7 RightShift
    bit 6,(ix+0)
    jr nz,@RightShift
    jr @stayhere
@RightShift:
    res 0,(ix+14) ; debounce the key (hopefully)
    ld a,%10000000
    ; call multiPurposeDelay

; restore everything
    ld hl, (uhl)
    ld bc, (ubc)
    ld de, (ude)
    ld ix, (uix)
    ld iy, (uiy)
    pop af
; all done
    ret

; print registers to screen in hexidecimal format
; inputs: none
; outputs: values of every register printed to screen
;    values of each register in global scratch memory
; destroys: nothing
dumpRegistersHex:
; store everything in scratch
    ld (uhl),hl
    ld (ubc),bc
    ld (ude),de
    ld (uix),ix
    ld (uiy),iy
    push af ; fml
    pop hl ; thanks, zilog
    ld (uaf),hl
    push af ; dammit

; home the cursor
    ; call vdu_home_cursor
    ; call printNewLine

; print each register
    ld hl,str_afu
    call printString
    ld hl,(uaf)
    call printHex24
    call printNewLine

    ld hl,str_hlu
    call printString
    ld hl,(uhl)
    call printHex24
    call printNewLine

    ld hl,str_bcu
    call printString
    ld hl,(ubc)
    call printHex24
    call printNewLine

    ld hl,str_deu
    call printString
    ld hl,(ude)
    call printHex24
    call printNewLine

    ld hl,str_ixu
    call printString
    ld hl,(uix)
    call printHex24
    call printNewLine

    ld hl,str_iyu
    call printString
    ld hl,(uiy)
    call printHex24
    
    call printNewLine
    call printNewLine
; restore everything
    ld hl, (uhl)
    ld bc, (ubc)
    ld de, (ude)
    ld ix, (uix)
    ld iy, (uiy)
    pop af
; all done
    ret
    
dumpRegistersHexPrime:
    exx
    ex af,af'
    call dumpRegistersHex
    ex af,af'
    exx
    ret

; additionally dump prime registers
; inputs: none
; outputs: values of every register printed to screen
; destroys: nothing
dumpRegistersHexAll:
    call dumpRegistersHex
    ex af,af'
    exx
    call dumpRegistersHex
    ex af,af'
    exx
    ret

; print hlu to screen in hexidecimal format
; inputs: none
; destroys: nothing
print_hex_hl:
    push af
    push hl
    ld hl,str_hlu
    call printString
    pop hl
    push hl
    call printHex24
    ld a,' '
    rst.lil 10h
    pop hl
    pop af
    ret

; print bcu to screen in hexidecimal format
; inputs: none
; destroys: nothing
print_hex_bc:
    push af
    push hl
    push bc
    ld hl,str_bcu
    call printString
    pop hl
    push hl
    call printHex24
    ld a,' '
    rst.lil 10h
    pop bc
    pop hl
    pop af
    ret

; print deu to screen in hexidecimal format
; inputs: none
; destroys: nothing
print_hex_de:
    push af
    push hl
    push de
    ld hl,str_deu
    call printString
    pop hl
    push hl
    call printHex24
    ld a,' '
    rst.lil 10h
    pop de
    pop hl
    pop af
    ret


; inputs: whatever is in the flags register
; outputs: binary representation of flags
;          with a header so we know which is what
; destroys: nothing
; preserves: everything
dumpFlags:
; first we curse zilog for not giving direct access to flags
    push af ; this is so we can send it back unharmed
    push af ; this is so we can pop it to hl
; store everything in scratch
    ld (uhl),hl
    ld (ubc),bc
    ld (ude),de
    ld (uix),ix
    ld (uiy),iy
; next we print the header 
    ld hl,@header
    call printString
    pop hl ; flags are now in l
    ld a,l ; flags are now in a
    call printBin8
    call printNewLine
; restore everything
    ld hl, (uhl)
    ld bc, (ubc)
    ld de, (ude)
    ld ix, (uix)
    ld iy, (uiy)
    pop af ; send her home the way she came
    ret
; Bit 7 (S): Sign flag
; Bit 6 (Z): Zero flag
; Bit 5 (5): Reserved (copy of bit 5 of the result)
; Bit 4 (H): Half Carry flag
; Bit 3 (3): Reserved (copy of bit 3 of the result)
; Bit 2 (PV): Parity/Overflow flag
; Bit 1 (N): Subtract flag
; Bit 0 (C): Carry flag
@header: db "SZxHxPNC\r\n",0 ; cr/lf and 0 terminator


; print bytes from an address to the screen in hexidecimal format
; inputs: hl = address of first byte to print, a = number of bytes to print
; outputs: values of each byte printed to screen separated by spaces
; destroys: nothing
dumpMemoryHex:
; save registers to the stack
    push bc
    push hl
    push af

; print the address and separator
    call printHex24
    ld a,':'
    rst.lil 10h
    ld a,' '
    rst.lil 10h

; set b to be our loop counter
    pop af
    ld b,a
    pop hl
    push hl
    push af
@loop:
; print the byte
    ld a,(hl)
    call printHex8
; print a space
    ld a,' '
    rst.lil 10h
    inc hl
    djnz @loop
    call printNewLine

; restore everything
    pop af
    pop hl
    pop bc

; all done
    ret


; print bytes from an address to the screen in binary format
; inputs: hl = address of first byte to print, a = number of bytes to print
; outputs: values of each byte printed to screen separated by spaces
; destroys: nothing
dumpMemoryBin:
; save all registers to the stack
    push af
    push bc
    push de
    push hl
    push ix
    push iy

; set b to be our loop counter
    ld b,a
@loop:
; print the byte
    ld a,(hl)
    push hl
    push bc
    call printBin8
    pop bc
; print a space
    ld a,' '
    rst.lil 10h
    pop hl
    inc hl
    djnz @loop
    call printNewLine

; restore everything
    pop iy
    pop ix
    pop hl
    pop de
    pop bc
    pop af
; all done
    ret

; print bytes from an address to the screen in binary format
; with the bits of each byte in reverse order (lsb first)
; inputs: hl = address of first byte to print, a = number of bytes to print
; outputs: values of each byte printed to screen separated by spaces
; destroys: nothing
dumpMemoryBinRev:
; save all registers to the stack
    push af
    push bc
    push de
    push hl
    push ix
    push iy

; set b to be our loop counter
    ld b,a
@loop:
; print the byte
    ld a,(hl)
    push hl
    push bc
    call printBin8Rev
    pop bc
; print a space
    ld a,' '
    rst.lil 10h
    pop hl
    inc hl
    djnz @loop
    call printNewLine

; restore everything
    pop iy
    pop ix
    pop hl
    pop de
    pop bc
    pop af
; all done
    ret

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
    PUSH_ALL
    ld c,0 ; X
    ld b,0 ; Y
    call vdu_move_cursor

    call dumpFlags
    call dumpRegistersHex
    
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

    POP_ALL
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

    call printInline
    asciz "\r\n#ori: "
    ld a,(ix+sprite_num_orientations)
    call printHexA

    call printInline
    asciz "\r\n#ani: "
    ld a,(ix+sprite_num_animations)
    call printHexA

    call printInline
    asciz "\r\nani : "
    ld a,(ix+sprite_animation)
    call printHexA

    call printInline
    asciz "\r\nanit: "
    ld a,(ix+sprite_animation_timer)
    call printHexA

    call printInline
    asciz "\r\nmvt : "
    ld a,(ix+sprite_move_timer)
    call printHexA

    call printInline
    asciz "\r\nmvs : "
    ld a,(ix+sprite_move_step)
    call printHexA

    call printInline
    asciz "\r\npts : "
    ld a,(ix+sprite_points)
    call printHexA

    call printInline
    asciz "\r\ndmg : "
    ld a,(ix+sprite_shield_damage)
    call printHexA

    call printInline
    asciz "\r\nfrm : "
    ld a,(ix+sprite_frame)
    call printHexA

    call printNewLine

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

    ; call printInline
    ; asciz "\r\nxv  :"
    ; ld hl,(ix+tile_xvel)
    ; call print_s168_hl

    ; call printInline
    ; asciz "\r\nyv  :"
    ; ld hl,(ix+tile_yvel)
    ; call print_s168_hl

    ; call printInline
    ; asciz "\r\nv   :"
    ; ld hl,(ix+tile_vel)
    ; call print_s168_hl

    ; call printInline
    ; asciz "\r\nhdg :"
    ; ld hl,(ix+tile_heading)
    ; call print_s168_hl

    ; call printInline
    ; asciz "\r\nori :"
    ; ld hl,(ix+tile_orientation)
    ; call print_s168_hl
    
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

    call printInline
    asciz "\r\nszx : "
    ld a,(ix+tile_dim_x)
    call printHexA
    
    call printInline
    asciz "\r\nszy : "
    ld a,(ix+tile_dim_y)
    call printHexA

    call printInline
    asciz "\r\n#ori: "
    ld a,(ix+tile_num_orientations)
    call printHexA

    call printInline
    asciz "\r\n#ani: "
    ld a,(ix+tile_num_animations)
    call printHexA

    call printInline
    asciz "\r\nani : "
    ld a,(ix+tile_animation)
    call printHexA

    call printInline
    asciz "\r\nanit: "
    ld a,(ix+tile_animation_timer)
    call printHexA

    call printInline
    asciz "\r\nmvt : "
    ld a,(ix+tile_move_timer)
    call printHexA

    call printInline
    asciz "\r\nmvs : "
    ld a,(ix+tile_move_step)
    call printHexA

    call printInline
    asciz "\r\npts : "
    ld a,(ix+tile_points)
    call printHexA

    call printInline
    asciz "\r\ndmg : "
    ld a,(ix+tile_shield_damage)
    call printHexA

    call printInline
    asciz "\r\nfrm : "
    ld a,(ix+tile_frame)
    call printHexA

    call printNewLine
    call printNewLine

    ; call waitKeypress
    ret

dump_player_record:
    ld ix,player_begin

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

    ; call printInline
    ; asciz "\r\nszx : "
    ; ld a,(ix+sprite_dim_x)
    ; call printHexA
    
    ; call printInline
    ; asciz "\r\nszy : "
    ; ld a,(ix+sprite_dim_y)
    ; call printHexA

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
    ; call printNewLine

    ; call waitKeypress
    ret