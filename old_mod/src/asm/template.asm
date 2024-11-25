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

; --- INITIALIZATION ---
init:
    ret

; --- MAIN PROGRAM ---
main:
    call vdu_cls

; testing atan2_168fast
    ; call printNewLine
    ld bc,256*1
    ld de,256*-1
    call atan2_168fast
    call print_s168_hl
    call printNewLine
    ret

; testing division
    ld hl,1
    ld de,2
    call udiv168
    call dumpRegistersHex
    call print_s168_hl
    call print_s168_de
    call printNewLine
    ret

; testing trig
    ld de,0x006100 ; 97
    ld hl,0x007C71 ; 124.444444444444444444

    push de
    push hl

    call sin168
    call print_s168_hl
    call printNewLine

    pop hl
    push hl

    call cos168
    call print_s168_hl
    call printNewLine

    pop hl
    pop de

    call polar_to_cartesian
    call print_s168_bc
    call print_s168_de
    call print_s168_hl
    call printNewLine

    ret