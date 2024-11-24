    assume adl=1   
    org 0x040000    

    jp start       

_exec_name:
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
; API includes
    include "mos_api.inc"
    include "functions.inc"
	include "files.inc"
	include "fonts_bmp.inc"
    include "timer.inc"
    include "vdu.inc"
	include "vdu_plot.inc"
    include "vdu_sound.inc"
	include "vdu_sprites.inc"
	include "maths.inc"
	include "arith24.inc"
	include "fixed24.inc"
	include "trig24.inc"

; Application includes
    include "temp.inc"

; --- INITIALIZATION ---
init:
    ld iy,sprites_image_list
    ld a,(iy+image_type) ; get image type
    ld bc,(iy+image_width) ; get image width
    ld de,(iy+image_height) ; get image height
    ld ix,(iy+image_filesize) ; get image file size
	ld hl,(iy+image_bufferId) ; get image bufferId
    ld iy,(iy+image_filename) ; get image filename
    call vdu_load_img

; set up the display
    ld a,8;+128 ; 136   320   240   64    60hz double-buffered
    call vdu_set_screen_mode
    xor a
    call vdu_set_scaling


; set up sprites
    xor a
    call vdu_sprite_select
    call vdu_sprite_clear_frames
    ld hl,BUF_SHIP_1C
    call vdu_sprite_add_buff

    ld a,1
    call vdu_sprite_activate

    xor a
    call vdu_sprite_select
    call vdu_sprite_show


	ret


; --- MAIN PROGRAM ---
main:
    ; ld hl,BUF_SHIP_1C
    ; call vdu_buff_select
    ; ld bc,0
    ; ld de,223*256
    ; call vdu_plot_bmp168

    ld bc,0
    ld de,223*256
    call vdu_sprite_move_abs168

    ret