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
img_load_init:
; initialize logo's position parameters
	ld hl,0
	ld (logo_yvel),hl

    ld hl,45
	ld (logo_y_cur),hl
	ld (logo_y_min),hl
    ld (logo_y_max),hl

	ld hl,1
	ld (logo_xvel),hl

	ld hl,10
	ld (logo_x_cur),hl
	ld (logo_x_min),hl

	ld hl,320-120
	ld (logo_x_max),hl

    ret

; inputs: bc is the number of images to load, cur_buffer_id_lut and cur_load_jump_table set to the address of the first entry in the respective lookup tables
img_load_main:
    ld hl,0
    ld (cur_file_idx),hl
img_load_main_loop:
; back up loop counter
    push bc
; load the next image
    call load_next_image
; draw all the things
    call draw_all_the_things
; draw the most recently loaded image
	ld hl,(cur_buffer_id)
	call vdu_buff_select
	ld bc,0
	ld de,0
	call vdu_plot_bmp
; move logo
	call move_logo
; print welcome message
	ld ix,font_rc
	ld hl,hello_world
	ld bc,32
	ld de,2
	call font_bmp_print
; print current filename
	call vdu_cls
	ld hl,(cur_filename)
	call printString
	call printNewLine
	
; print current load stopwatch
	ld hl,loading_time
	call printString
	call stopwatch_get ; hl = elapsed time in 120ths of a second
	call printDec

; flip screen 
    call vdu_flip 
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
    ld (cur_file_idx),de
    ld d,image_record_size
    mlt de
    ld iy,(cur_image_list)
    add iy,de
    ld a,(iy+image_type) ; get image type
    ld bc,(iy+image_width) ; get image width
    ld de,(iy+image_height) ; get image height
    ld ix,(iy+image_filesize) ; get image file size
	ld hl,(iy+image_bufferId) ; get image bufferId
    ld iy,(iy+image_filename) ; get image filename
    call vdu_load_img
	ret

move_logo:
; activate logo bitmap
	ld hl, BUF_SPLASH_LOGO
	call vdu_buff_select
; update position based on velocity parameters
	ld hl, (logo_x_cur)
	ld de, (logo_xvel)
	add hl, de
	ld (logo_x_cur), hl
	ex de,hl ; store x_cur in de
; check if we're < x_min
	ld hl,(logo_x_min)
	xor a ; clear carry
	sbc hl,de ; x_min - x_cur
	jp p, move_logo_x_min
; check if we're > x_max
	ld hl,(logo_x_max)
	xor a ; clear carry
	sbc hl,de ; x_max - x_cur
	jp m, move_logo_x_max
; if not at either boundary, fall through to draw logo's
draw_logo:
	ld bc,(logo_x_cur)
	ld de,(logo_y_cur)
	call vdu_plot_bmp
	ret
move_logo_x_min:
	ld hl,1
	ld (logo_xvel),hl
	ld hl,(logo_x_min)
	ld (logo_x_cur),hl
	jr draw_logo
move_logo_x_max:
	ld hl,-1
	ld (logo_xvel),hl
	ld hl,(logo_x_max)
	ld (logo_x_cur),hl
	jr draw_logo
logo_xvel: dl 0
logo_x_cur: dl 0
logo_x_min: dl 0
logo_x_max: dl 0

logo_yvel: dl 0
logo_y_cur: dl 0
logo_y_min: dl 0
logo_y_max: dl 0

draw_all_the_things:
    ld hl,BUF_SPLASH_BG
    call vdu_buff_select
    ld bc,0
    ld de,0
    call vdu_plot_bmp
	ret