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

    ; jp temp
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

    include "mos_api.inc"
    include "functions.inc"
    include "input.inc"
    include "timer.inc"
    include "vdu.inc"
    include "images.inc"
    include "temp.inc"

init:
; set screen mode
    ; ld a,0 ; 640x480x16 single-buffered
    ; ld a,19 ; 1024x768x4 single-buffered
    ; ld a,8 ; 320x240x64 single-buffered
    ld a,20 ; 512x384x64 single-buffered
    ; ld a,23 ; 512x384x2 single-buffered
    call vdu_set_screen_mode

; set screen scaling and background colors
    ld hl,@beg
    ld bc,@end-@beg
    rst.lil $18
    jp @end
@beg:
;   VDU 23, 0, &C0, 0: Normal coordinates
    db 23,0,$C0,0
;   VDU 17, color : set text background color
    db 17,12+128 ; blue
;   VDU 18, color : set gfx background color
    db 18,0,12+128 ; blue
@end:
    call cursor_off
    call vdu_cls

; initialize main loop timer
main_loop_timer_reset: equ 60 ; 120ths of a second
    ld hl,main_loop_timer_reset
    call tmr_main_loop_set

    call tmr_slideshow_set

    ret ; init

main:
    ld de, 0
    jp rendbmp

mainloop:
    call reset_keys

waitloop:
    call set_keys
    call tmr_main_loop_get
    jp z, do_input
    jp m, do_input
    jp waitloop

rendbmp:
; test de for wraparound
    ld hl,0
    xor a ; clear carry
    sbc hl,de
    jp z,@not_neg
    jp m,@not_neg
    ld de,num_images-1
    jp @load_image
@not_neg:
    ld hl,num_images-1
    xor a ; clear carry
    sbc hl,de
    jp p,@load_image
    ld de,0
@load_image:
    ld (current_image_index),de
    ld d,image_record_size
    mlt de
    ld iy,image_list
    add iy,de
    ld a,(iy+image_type) ; get image type
    ld bc,(iy+image_width) ; get image width
    ld de,(iy+image_height) ; get image height
    ld ix,(iy+image_filesize) ; get image file size
    ld hl,(iy+image_filename) ; get image filename
    push hl
    pop iy 
    ld hl,256 ; set image bufferId
    call vdu_load_img
; plot image
    call vdu_cls
    ld bc,0 ; x
    ld de,0 ; y
    call vdu_plot_bmp

no_move:
    ld hl,main_loop_timer_reset
    call tmr_main_loop_set
    jp mainloop

main_end:
; exit program gracefully
    xor a ; 640x480x16 single-buffered
    call vdu_set_screen_mode
    ld a,1 ; scaling on
    call vdu_set_scaling
    call cursor_on
    ret

; load to onboard 8k sram
filedata: equ 0xB7E000