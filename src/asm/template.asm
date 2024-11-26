    assume adl=1   
    org 0x040000    

    include "mos_api.inc"

    MACRO PROGNAME
    ASCIZ "flower_demo"
    ENDMACRO

    jp start       

_exec_name:
	PROGNAME

    align 64      
    db "MOS"       
    db 00h         
    db 01h

start:              
    push af
    push bc
    push de
    push ix
    push iy

    call init
    call main

exit:

    pop iy 
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0

    ret 

; --- MAIN PROGRAM ---
; APPLICATION INCLUDES
; API includes
    include "functions.inc"
    include "vdu.inc"
	include "maths.inc"
	include "trig24fast.inc"
	include "fixed24.inc"
	include "files.inc"
    include "timer.inc"

; --- INITIALIZATION ---
init:
    ret

; --- MAIN PROGRAM ---
main:
    ld iy,tmr_test
    ld hl,120 ; 10 seconds
    call tmr_set
    ld hl,0
    push hl
    call vdu_vblank
@loop:
    ld hl,65535
    ld de,1023
    call udiv24
    pop hl
    inc hl
    push hl
    ld iy,tmr_test
    call tmr_get
    jp p,@loop
    pop hl
    call printDec
    call printNewLine

fast_div:
    ld iy,tmr_test
    ld hl,120 ; 10 seconds
    call tmr_set
    ld hl,0
    push hl
    call vdu_vblank
@loop:
    ld hl,65535
    ld c,128
    call HL_Div_C
    pop hl
    inc hl
    push hl
    ld iy,tmr_test
    call tmr_get
    jp p,@loop
    pop hl
    call printDec
    call printNewLine

    ret

HL_Div_C:
   ;Inputs:
   ;     HL is the numerator
   ;     C is the denominator
   ;Outputs:
   ;     A is the remainder
   ;     B is 0
   ;     C is not changed
   ;     DE is not changed
   ;     HL is the quotient
   ;
          ld b,16
          xor a
            add hl,hl
            rla
            cp c
            jr c,$+4
              inc l
              sub c
            djnz $-7
          ret