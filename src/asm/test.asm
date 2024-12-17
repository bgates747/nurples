    assume adl=1 
    org 0x040000 
    include "mos_api.inc"
    include "macros.inc"

    jp start 
    align 64 
    db "MOS" 
    db 00h 
    db 01h 

; ###### tile TABLE VARIABLES ######
    align 256
tile_stack: ; stack of pointers to tile records
    db 0x01, 0x02, 0x03
    db 0x04, 0x05, 0x06
    db 0x07, 0x08, 0x09
    db 0x0A, 0x0B, 0x0C
    db 0x0D, 0x0E, 0x0F
    dl 0x000000 ; list terminator
tile_stack_end:
tile_stack_pointer: dl tile_stack ; pointer to current stack record, initialized to tile_stack
; tile_table_pointer: dl tile_table_base ; pointer to top address of current record, initialized to tile_table_base
num_active_tiles: dl 0 ; how many active tiles
next_tile_id: db 0 ; next available tile id
new_tile_table_pointer: dl 0 ; pointer to new tile record

start: 
    push af
    push bc
    push de
    push ix
    push iy

; MAIN PROGRAM
; set up a test case
    ld a,5
    ld (num_active_tiles),a
    ld l,1 ; index into stack
    ld h,3
    mlt hl ; offset into stack
    ld de,tile_stack
    add hl,de ; address of stack record
    ld (tile_stack_pointer),hl ; set pointer to stack record
    call DEBUG_PRINT_TILE_STACK

; compute address to copy from
    ld hl,(tile_stack_pointer)
    inc hl
    inc hl
    inc hl
    push hl ; save copy from address

; compute bytes to copy
    ld a,(num_active_tiles)
    ld l,a
    ld h,3
    mlt hl
    ld de,tile_stack
    add hl,de ; hl = bottom of stack address
    ld de,(tile_stack_pointer)
    or a ; clear carry
    sbc hl,de ; hl = bytes to copy
    push hl
    pop bc ; bytes to copy

; compute target address
    ld de,(tile_stack_pointer)

; copy bytes
    pop hl ; copy from address
    ldir

; update stack pointer and active tile count
    ld hl,num_active_tiles
    dec (hl)
    ld hl,(tile_stack_pointer)
    dec hl
    dec hl
    dec hl
    ld (tile_stack_pointer),hl

; output results
    CALL DEBUG_PRINT_TILE_STACK
; END MAIN PROGRAM

exit:
    pop iy
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0
    ret

; BASIC DEBUG FUNCTIONS

; Print a zero-terminated string
; HL: Pointer to string
printString:
    PUSH BC
    LD BC,0
    LD A,0
    RST.LIL 18h
    POP BC
    RET
; print a VDU sequence
; HL: Pointer to VDU sequence - <1 byte length> <data>
sendVDUsequence:
    PUSH BC
    LD BC, 0
    LD C, (HL)
    RST.LIL 18h
    POP BC
    RET
; Print Newline sequence to VDP
; destroys bc
printNewLine:
    push af ; for some reason rst.lil 10h sets carry flag
    LD A, '\r'
    RST.LIL 10h
    LD A, '\n'
    RST.LIL 10h
    pop af
    RET

; Print a 24-bit HEX number
; HLU: Number to print
printHex24:
    HLU_TO_A
    CALL printHex8
; Print a 16-bit HEX number
; HL: Number to print
printHex16:
    LD A,H
    CALL printHex8
    LD A,L
; Print an 8-bit HEX number
; A: Number to print
printHex8:
    LD C,A
    RRA 
    RRA 
    RRA 
    RRA 
    CALL @F
    LD A,C
@@:
    AND 0Fh
    ADD A,90h
    DAA
    ADC A,40h
    DAA
    RST.LIL 10h
    RET

printHexA:
    push af
    push bc
    call printHex8
    ld a,' '
    rst.lil 10h
    pop bc
    pop af
    ret

printHexUHL:
    push af
    push bc
    push hl
    call printHex24
    pop hl
    pop bc
    pop af
    ret

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
    ; call printNewLine

    ; call vdu_vblank

    ; call printNewLine
; restore everything
    ld hl, (uhl)
    ld bc, (ubc)
    ld de, (ude)
    ld ix, (uix)
    ld iy, (uiy)
    pop af
; all done
    ret
str_afu: db " af=",0
str_hlu: db " hl=",0
str_bcu: db " bc=",0
str_deu: db " de=",0
str_ixu: db " ix=",0
str_iyu: db " iy=",0
; global scratch memory for registers
uaf: dl 0
uhl: dl 0
ubc: dl 0
ude: dl 0
uix: dl 0
uiy: dl 0
usp: dl 0
upc: dl 0


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


; print the binary representation of the 8-bit value in a
; destroys a, hl, bc
printBin8:
    ld b,8 ; loop counter for 8 bits
    ld hl,@cmd ; set hl to the low byte of the output string
    ; (which will be the high bit of the value in a)
@loop:
    rlca ; put the next highest bit into carry
    jr c,@one
    ld (hl),'0'
    jr @next_bit
@one:
    ld (hl),'1'
@next_bit:
    inc hl
    djnz @loop
; print it
    ld hl,@cmd 
    ld bc,@end-@cmd 
    rst.lil $18 
    ret
@cmd: ds 8 ; eight bytes for eight bits
@end:

DEBUG_PRINT_TILE_STACK:
    PUSH_ALL
    call printNewLine
    call printNewLine
    ld hl,(tile_stack_pointer)
    ld a,3
    call dumpMemoryHex
    ld a,(num_active_tiles)
    call printHexA
    call printNewLine
    ld ix,tile_stack
    ld b,6
@loop:
    push bc
    push ix
    pop hl
    ld a,3
    call dumpMemoryHex
    lea ix,ix+3
    pop bc
    djnz @loop
    POP_ALL
    ret

DEBUG_PRINT:
    call printNewLine
    call dumpFlags
    call dumpRegistersHex
    call printNewLine
    ret