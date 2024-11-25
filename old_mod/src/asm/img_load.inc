cur_file_idx: dl 0
cur_filename: dl 0
cur_buffer_id: dl 0
cur_buffer_id_lut: dl 0
cur_load_jump_table: dl 0

img_load_init:
; initialize bj's position parameters
	ld hl,0
	ld (bj_yvel),hl

    ld hl,45
	ld (bj_y_cur),hl
	ld (bj_y_min),hl
    ld (bj_y_max),hl

	ld hl,1
	ld (bj_xvel),hl

	ld hl,10
	ld (bj_x_cur),hl
	ld (bj_x_min),hl

	ld hl,320-120
	ld (bj_x_max),hl

    ret

; inputs: bc is the number of images to load, cur_buffer_id_lut and cur_load_jump_table set to the address of the first entry in the respective lookup tables
img_load_main:
    ld hl,0
    ld (cur_file_idx),hl
img_load_main_loop:
; back up loop counter
    push bc
; load the next panel
    call load_next_panel
; draw all the things
    call tmp_draw_all_the_things
; draw the most recently loaded panel
	ld hl,(cur_buffer_id)
	call vdu_buff_select
	ld bc,0
	ld de,0
	call vdu_plot_bmp
; move bj
	call move_bj
; print welcome message
	ld ix,font_itc_honda
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

load_next_panel:
; look up the load routine for the current file index
	ld hl,(cur_file_idx) 
	add hl,hl ; multiply current index by 2 ...
	ld de,(cur_file_idx)
	add hl,de ; ... now by 3
	ld de,(cur_load_jump_table) ; tack it on to the base address of the jump table
	add hl,de 
	ld hl,(hl) ; hl is pointing to load routine address
	ld (@jump_addr+1),hl ; self-modifying code ...
@jump_addr:
	call 0 ; call the panel load routine
; look up the buffer id for the current file
	ld hl,(cur_file_idx) 
	add hl,hl ; multiply current index by 2 ...
	ld de,(cur_file_idx)
	add hl,de ; ... now by 3
	ld de,(cur_buffer_id_lut) ; tack it on to the base address of the lookup table
	add hl,de 
	ld hl,(hl)
	ld (cur_buffer_id),hl
; bump the current file index
	ld hl,(cur_file_idx)
	inc hl
	ld (cur_file_idx),hl
	ret

move_bj:
; activate bj bitmap
	ld hl, BUF_UI_BJ_120_120
	call vdu_buff_select
; update position based on velocity parameters
	ld hl, (bj_x_cur)
	ld de, (bj_xvel)
	add hl, de
	ld (bj_x_cur), hl
	ex de,hl ; store x_cur in de
; check if we're < x_min
	ld hl,(bj_x_min)
	xor a ; clear carry
	sbc hl,de ; x_min - x_cur
	jp p, move_bj_x_min
; check if we're > x_max
	ld hl,(bj_x_max)
	xor a ; clear carry
	sbc hl,de ; x_max - x_cur
	jp m, move_bj_x_max
; if not at either boundary, fall through to draw bj's
draw_bj:
	ld bc,(bj_x_cur)
	ld de,(bj_y_cur)
	call vdu_plot_bmp
	ret
move_bj_x_min:
	ld hl,1
	ld (bj_xvel),hl
	ld hl,(bj_x_min)
	ld (bj_x_cur),hl
	jr draw_bj
move_bj_x_max:
	ld hl,-1
	ld (bj_xvel),hl
	ld hl,(bj_x_max)
	ld (bj_x_cur),hl
	jr draw_bj
bj_xvel: dl 0
bj_x_cur: dl 0
bj_x_min: dl 0
bj_x_max: dl 0

bj_yvel: dl 0
bj_y_cur: dl 0
bj_y_min: dl 0
bj_y_max: dl 0

tmp_draw_all_the_things:
    ld hl,BUF_UI_SPLASH
    call vdu_buff_select
    ld bc,0
    ld de,0
    call vdu_plot_bmp
	ret

; load an rgba2222 image file to a buffer
; inputs: bc,de image width,height ; hl = bufferId ; ix = file size ; iy = pointer to filename
vdu_load_img_2: 
; back up image dimension parameters and buffer id
	push hl
	push bc
	push de
; load the image
	call vdu_load_buffer_from_file
; now make it a bitmap
	pop de
	pop bc
	pop hl
	ld a,1 ; the magic number for rgba2222
	jp vdu_bmp_create ; will return to caller from there

; load an rgba8888 image file to a buffer
; inputs: bc,de image width,height ; hl = bufferId ; ix = file size ; iy = pointer to filename
vdu_load_img_8: 
; back up image dimension parameters and buffer id
	push hl
	push bc
	push de
; load the image
	call vdu_load_buffer_from_file
; now make it a bitmap
	pop de
	pop bc
	pop hl
	ld a,0 ; the magic number for rgba8888
	jp vdu_bmp_create ; will return to caller from there