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

;--- APPLICATION INCLUDES ---
    include "fixed168.inc"

; --- INITIALIZATION ---
init:
   ret

; --- MAIN PROGRAM ---
main:
    call printInline
    asciz "rounding error in print_u168: "
    ld hl,@arg1
    call asc_to_s168 ; de = @arg1
    ex de,hl
    call printHexUHL
    push hl
    call printInline
    asciz " -> "
    pop hl
    call print_u168
    call printInline
    asciz " should be: "
    ld hl,@arg1
    call printString
    call printNewLine

    call printInline
    asciz "sign overflow in print_u168: "
    ld hl,@arg2
    call asc_to_s168 ; de = @arg2
    ex de,hl
    call printHexUHL
    push hl
    call printInline
    asciz " -> "
    pop hl
    call print_u168
    call printInline
    asciz " should be: "
    ld hl,@arg2
    call printString
    call printNewLine

    call printInline
    asciz "unsigned fixed place multiplication: e * pi = "
    ld hl,0x0002B7 ; 2.718
    ld de,0x000324 ; 3.141
    call umul168
    call print_u168
    call printInline
    asciz " should be: 8.539\r\n"

    call printInline
    asciz "unsigned fixed place division: e / pi = "
    ld hl,0x0002B7 ; 2.718
    ld de,0x000324 ; 3.141
    call udiv168
    call print_u168
    call printInline
    asciz " should be: 0.864\r\n"

    call printInline
    asciz "distance between two points: "
    ld bc,0x000000 ; x0 0
    ld de,0x000000 ; y0 0
    ld ix,0x010000 ; x1 256
    ld iy,0x010000 ; y1 256
    call distance168
    call print_s168
    call printInline
    asciz " should be: 0x016A09 362.039\r\n"

    ret

@arg1: asciz "32767.999" ; 0x7FFFFF
@arg2: asciz "-32768" ; 0x800000