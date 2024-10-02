	.assume adl=1   
    .org 0x040000    

    jp start       

    .align 64      
    .db "MOS"       
    .db 00h         
    .db 01h       

start:              
    push af
    push bc
    push de
    push ix
    push iy

; ###############################################
	call	init			; Initialization code
	call 	main			; Call the main function
; ###############################################

exit:
    pop iy
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0
    ret

; API includes
    include "mos_api.inc"
    include "functions.inc"
	include "files.inc"
	include "fonts_bmp.inc"
    include "timer.inc"
    include "vdu.inc"
    include "vdu_sound.inc"
    include "vdu_fonts.inc"

; Application includes
	include "font_rc.inc"
	include "input.inc"
    include "images.inc"
    include "images_sprites.inc"
	include "images_ui.inc"

main: