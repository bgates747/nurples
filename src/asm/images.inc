    align 256

image_type: equ 0
image_width: equ image_type+3
image_height: equ image_width+3
image_filesize: equ image_height+3
image_filename: equ image_filesize+3
image_bufferId: equ image_filename+3
image_record_size: equ image_bufferId+3

cur_image_list: dl 0
cur_filename: dl 0
cur_buffer_id: dl 0

img_load_init:
; initialize logo's position parameters
    ld hl,0
    ld (logo_xvel),hl
    ld (logo_xpos),hl
; begin 16.8 maths to determine logo's y position and velocity
    ld hl,224*256 
    ld (logo_ypos),hl
    ex de,hl
    ld hl,240*256
    ld de,-256*[sprites_num_images+tiles_dg_num_images+ui_num_images]
    call sdiv168 ; ud.e = distance / num images = y velocity
    ld (logo_yvel),de
; all done
    ret

; inputs: bc is the number of images to load, cur_image_list set
img_load_main:
; back up loop counter
    push bc

; load the next image
    call load_next_image

; plot the background
    call vdu_cls
    ld hl,BUF_SPLASH_BG
    call vdu_buff_select
    ld bc,0
    ld de,0
    call vdu_plot_bmp

; ; draw the most recently loaded image
; 	ld hl,(cur_buffer_id)
; 	call vdu_buff_select
; 	ld bc,0
; 	ld de,0
; 	call vdu_plot_bmp

; move logo
    call move_logo

; print current filename
    ld hl,(cur_filename)
    call printString

; flip the framebuffer
    call vdu_flip

; decrement loop counter
    pop bc
    dec bc
    ld a,c
    or a
    jp nz,img_load_main
    ld a,b
    or a
    jp nz,img_load_main
    ret

load_next_image:
    ld iy,(cur_image_list)
    push iy
    ld a,(iy+image_type) ; get image type
    ld bc,(iy+image_width) ; get image width
    ld de,(iy+image_height) ; get image height
    ld ix,(iy+image_filesize) ; get image file size
    ld hl,(iy+image_bufferId) ; get image bufferId
    ld (cur_buffer_id),hl
    ld iy,(iy+image_filename) ; get image filename
    ld (cur_filename),iy
    call vdu_load_img
    pop iy
    lea iy,iy+image_record_size
    ld (cur_image_list),iy
    ret

move_logo:
; activate logo bitmap
    ld hl, BUF_SPLASH_LOGO
    call vdu_buff_select
; update position based on velocity parameters
    ld hl,(logo_ypos)
    ld de,(logo_yvel)
    add hl,de
    ld (logo_ypos),hl
; draw logo
    ld bc,(logo_xpos)
    ld de,(logo_ypos)
    call vdu_plot_bmp168
    ret

logo_xvel: dl 0
logo_xpos: dl 0

logo_yvel: dl 0
logo_ypos: dl 0

load_ui_images:
; initialize image loading variables
    ld hl,ui_image_list
    ld (cur_image_list),hl
; load images
    ld b,ui_num_images
@loop:
    push bc
    call load_next_image
    pop bc
    djnz @loop
    ret

load_sprite_images:
; initialize image loading variables
    ld hl,sprites_image_list
    ld (cur_image_list),hl
    ld bc,sprites_num_images
; load images
    call img_load_main
    ret

load_tilesets:
; initialize image loading variables
    ld hl,tiles_dg_image_list
    ld (cur_image_list),hl
    ld bc,tiles_dg_num_images
; load images
    call img_load_main

; ; initialize image loading variables
;     ld hl,tiles_xevious_image_list
;     ld (cur_image_list),hl
;     ld bc,tiles_xevious_num_images
; ; load images
;     call img_load_main
    ret