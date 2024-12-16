
; test the sign of HL
; inputs: HL obviously
; outputs: sign flag set if HL is negative, zero flag set if HL is zero
; destroys: flags    
    MACRO sign_hlu
    add hl,de
    or a ; clear flags
    sbc hl,de
    ENDMACRO

    MACRO hlu_mul256
    add hl,hl ; * 2
    add hl,hl ; * 4
    add hl,hl ; * 8
    add hl,hl ; * 16 
    add hl,hl ; * 32
    add hl,hl ; * 64
    add hl,hl ; * 128
    add hl,hl ; * 256
    ENDMACRO

; https://discord.com/channels/1158535358624039014/1282290921815408681/1317793870070812715
    MACRO uhl_udiv256
    ld a,l
    dec sp
    push hl
    inc sp
    pop hl
    inc hl
    dec.s hl ; <-- the .s sets hlu to zero (undocumented)
    ENDMACRO

    MACRO printChar char
    LD A, char
    RST.LIL 10h
    ENDMACRO

; Simulated call to subroutine at HL
; inputs: HL pointing to the subroutine address plus whatever the called function expects
; outputs: whatever the subroutine does, including HL and BC
; destroys: only what the subroutine does, but always BC
    MACRO callHL
    ld bc,@F ; Address of first instruction after the jump
    push bc ; which constitutes the return address
    jp (hl) ; Jump to the address in HL
@@:
    ENDMACRO

; Simulated call to subroutine at IX
; inputs: IX pointing to the subroutine address plus whatever the called function expects
; outputs: whatever the subroutine does, including IX and BC
; destroys: only what the subroutine does, but always BC
    MACRO callIX
    ld bc,@F ; Address of first instruction after the jump
    push bc ; which constitutes the return address
    jp (ix) ; Jump to the address in IX
@@:
    ENDMACRO

; Simulated call to soubroutinte at IY
; inputs: IY pointing to the subroutine address plus whatever the called function expects
; outputs: whatever the subroutine does, including IY and BC
; destroys: only what the subroutine does, but always BC
    MACRO callIY
    ld bc,@F ; Address of first instruction after the jump
    push bc ; which constitutes the return address
    jp (iy) ; Jump to the address in IY
@@:
    ENDMACRO

; put the value in HLU into the accumulator
; destroys: af
    MACRO HLU_TO_A
    push hl ; 4 cycles
    inc sp ; 1 cycle
    pop af ; 4 cycles
    dec sp ; 1 cycle
    ; 10 cycles total
    ENDMACRO

; put the value in a into hlu
; affects: HLU
; destroys: nothing
    MACRO A_TO_HLU
    ld (@F+1),hl ; 7 cycles
    ld (@F+3),a ; 5 cycles
@@: ld hl,0x000000 ; 4 cycles
    ; 16 cycles total
    ENDMACRO

    MACRO PUSH_ALL
    ex af,af'
    exx
    push af
    push hl
    push bc
    push de

    ex af,af'
    exx
    push af
    push hl
    push bc
    push de
    push ix
    push iy
    ENDMACRO

    MACRO POP_ALL
    pop iy
    pop ix
    pop de
    pop bc
    pop hl
    pop af
    ex af,af'
    exx

    pop de
    pop bc
    pop hl
    pop af
    ex af,af'
    exx
    ENDMACRO
