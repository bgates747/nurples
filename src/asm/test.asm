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
    include "images_tiles_dg.inc"
    include "images_sprites.inc"

; --- INITIALIZATION ---
init:
; clear all buffers
    call vdu_clear_all_buffers

; set up the display
    ld a,8;+128 ; 136   320   240   64    60hz double-buffered
    call vdu_set_screen_mode
    xor a
    call vdu_set_scaling


    ret

; --- MAIN PROGRAM ---
main:
load_sprite_images:
; initialize image loading variables
    ld hl,0
    ld (cur_file_idx),hl
    ld hl,sprites_image_list
    ld (cur_image_list),hl
    ld bc,sprites_num_images
; load images
    call img_load_main
load_tilesets:
; initialize image loading variables
    ld hl,0
    ld (cur_file_idx),hl
    ld hl,tiles_dg_image_list
    ld (cur_image_list),hl
    ld bc,tiles_dg_num_images
; load images
    call img_load_main
    ret

image_type: equ 0
image_width: equ image_type+3
image_height: equ image_width+3
image_filesize: equ image_height+3
image_filename: equ image_filesize+3
image_bufferId: equ image_filename+3
image_record_size: equ image_bufferId+3

cur_image_list: dl 0
cur_file_idx: dl 0
cur_filename: dl 0
cur_buffer_id: dl 0

; inputs: bc is the number of images to load, cur_image_list set
img_load_main:
    xor a
    ld (cur_file_idx),a

img_load_main_loop:
; back up loop counter
    push bc

; load the next image
    call load_next_image

; draw the most recently loaded image
    call vdu_cls
	ld hl,(cur_buffer_id)
	call vdu_buff_select
	ld bc,64
	ld de,64
	call vdu_plot_bmp

; print current filename
    ld hl,(cur_filename)
    call printString

; WAIT FOR KEY PRESS
    call waitKeypress

; decrement loop counter
    pop bc
    dec bc
    ld a,c
    or a
    jp nz,img_load_main_loop
    ld a,b
    or a
    jp nz,img_load_main_loop
    ret

load_next_image:
    ld d,image_record_size
    ld a,(cur_file_idx)
    ld e,a
    mlt de
    ld iy,(cur_image_list)
    add iy,de

    ld a,(iy+image_type) ; get image type
    ld bc,(iy+image_width) ; get image width
    ld de,(iy+image_height) ; get image height
    ld ix,(iy+image_filesize) ; get image file size
    ld hl,(iy+image_bufferId) ; get image bufferId
    ld (cur_buffer_id),hl
    ld iy,(iy+image_filename) ; get image filename
    ld (cur_filename),iy
    call vdu_load_img
    ld iy,cur_file_idx
    inc (iy)
    ret