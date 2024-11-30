    assume adl=1 
    org 0x040000 

    include "mos_api.inc"

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
    include "functions.inc"
    include "maths.inc"
    include "enemies.inc"
    include "files.inc"
    include "fixed168.inc"
    include "fonts.inc"
    include "fonts_list.inc"
    include "images.inc"
    include "images_sprites.inc"
    include "images_ui.inc"
    include "laser.inc"
    include "levels.inc"
    include "player.inc"
    include "sprites.inc"
    include "state.inc"
    include "tiles.inc"
    include "timer.inc"
    include "vdu.inc"
    include "vdu_fonts.inc"
    include "vdu_plot.inc"
    include "vdu_sprites.inc"

; --- INITIALIZATION ---
init:
    ret

; --- MAIN PROGRAM ---
main:
; load fonts
	; call fonts_load
    ; call printNewLine
    ld hl,0x0B0000 ; moslet
    callHL
; select font
    ld hl,computer_pixel_7_8x16
    ld a,1 ; flags
    call vdu_font_select
    ret
