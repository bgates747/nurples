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

load_ui_images:
; initialize image loading variables
	ld hl,0
	ld (cur_file_idx),hl
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

; inputs: bc is the number of images to load, cur_buffer_id_lut and cur_load_jump_table set to the address of the first entry in the respective lookup tables
img_load_main:
	xor a
    ld (cur_file_idx),a

img_load_main_loop:
; back up loop counter
    push bc

; plot the background
	call vdu_cls
    ld hl,BUF_SPLASH_BG
    call vdu_buff_select
    ld bc,0
    ld de,0
    call vdu_plot_bmp

; load the next image
    call load_next_image

; ; draw the most recently loaded image
; 	ld hl,(cur_buffer_id)
; 	call vdu_buff_select
; 	ld bc,0
; 	ld de,0
; 	call vdu_plot_bmp

; move logo
	call move_logo

; print current filename
	ld c,3 ; x
	ld b,0 ; y
	call vdu_move_cursor
	ld hl,(cur_filename)
	call printString

; wait for vblank to mitigate flicker
	call vdu_vblank

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

img_load_init:
; initialize logo's position parameters
	ld hl,0
	ld (logo_xvel),hl
	ld (logo_xpos),hl
; begin 16.8 maths to determine logo's y position and velocity
	ld hl,240-16*256 ; allow space for text at bottom of screen
	ld (logo_ypos),hl
	ex de,hl
	ld hl,-8*256 ; 8 pixels from top of screen
	add hl,de ; distance for logo to travel
	ld de,-sprites_num_images*256
	call sdiv168 ; ud.e = distance / num images = y velocity
	ld (logo_yvel),de
; all done
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