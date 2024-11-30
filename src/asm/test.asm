    assume adl=1 
    org 0x040000 

    ; include "mos_api.inc"

    jp start 

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
    include "../../fixed168/fixed168.inc"

; --- INITIALIZATION ---
init:
    ret

; --- MAIN PROGRAM ---
main:
    xor a
    call dumpRegistersHex
    ret
