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
    include "macros.inc"
    include "functions.inc"
    include "arith24.inc"
    include "maths.inc"
    include "files.inc"
    include "fixed168.inc"
    include "fonts.inc"
    include "fonts_list.inc"
    include "timer.inc"
    include "vdu.inc"
    include "vdu_fonts.inc"
    include "vdu_plot.inc"
    ; include "vdu_sprites.inc"

; --- MAIN PROGRAM FILE ---
hello_world: asciz "Hello, World!\r\n"
main:
; set up screen
    ld a,20
    call vdu_set_screen_mode
    xor a ; screen scaling off
    call vdu_set_scaling

; set gfx color
    xor a ; color mode
    ld c,c_magenta ; text color
    call vdu_gcol

; test printStringGfx
    ld bc,64 ; x
    ld de,128 ; y
    ld hl,hello_world
    call printStringGfx

; test print a filled rectangle to make sure gcol is working
    ld bc,64 ; x
    ld de,136 ; y
    ld ix,64+32
    ld iy,136+8
    call vdu_plot_rf
    ret
; end main

