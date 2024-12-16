    assume adl=1 
    org 0x040000 
    jp start 
    align 64 
    db "MOS" 
    db 00h 
    db 01h 

scratch: dl 0
start: 
    push af
    push bc
    push de
    push ix
    push iy

; MAIN PROGRAM
    ld (scratch),sp
    ld hl,(scratch)
    call printHexUHL
    call printNewLine

    push.s af
    ld (scratch),sp
    ld hl,(scratch)
    call printHexUHL
    call printNewLine
    pop.s af

    push af
    ld (scratch),sp
    ld hl,(scratch)
    call printHexUHL
    call printNewLine
    pop af
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

; put the value in HLU into the accumulator
; destroys: af
    MACRO HLU_TO_A
    push hl ; 4 cycles
    inc sp ; 1 cycle
    pop af ; 4 cycles
    dec sp ; 1 cycle
    ; 10 cycles total
    ENDMACRO

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