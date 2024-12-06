    assume adl=1 
    org 0x040000 
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

; API INCLUDES
    include "mos_api.inc"
    include "functions.inc"
    include "maths.inc"
    include "files.inc"
    include "vdu.inc"
    include "vdu_plot.inc"

; APPLICATION INCLUDES
    macro get_label name
        ld hl,name
    endmacro

; --- INITIALIZATION ---
init:

    ret

; --- MAIN PROGRAM ---
main:
    get_label some_label
    call printString
    call printNewLine
    ret

some_label: asciz "some string"

tbl0.field0.s: equ 1
tbl0.field1.s: equ 3
tbl0.field2.s: equ 6
tbl0.field3.s: equ 3
tbl0.field4.s: equ 1

tbl0.field0: equ 0
tbl0.field1: equ tbl0.field0.s
tbl0.field2: equ tbl0.field1 + tbl0.field1.s
tbl0.field3: equ tbl0.field2 + tbl0.field2.s
tbl0.field4: equ tbl0.field3 + tbl0.field3.s

tbl0.struct:
    blkb tbl0.field0.s,0 ; field0
    blkb tbl0.field1.s,0 ; field1
    blkb tbl0.field2.s,0 ; field2
    blkb tbl0.field3.s,0 ; field3
    blkb tbl0.field4.s,0 ; field4