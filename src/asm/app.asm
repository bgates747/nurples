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
	include "vdu_plot.inc"
    include "vdu_sound.inc"
	include "maths.inc"

; Application includes
	include "font_rc.inc"
	include "input.inc"
    include "images.inc"
    include "images_sprites.inc"
	include "images_ui.inc"

hello_world: defb "Welcome to Purple Nurples!",0
loading_ui: defb "Loading UI",0
loading_time: defb "Loading time:",0
loading_complete: defb "Press any key to continue.\r\n",0

init:
; clear all buffers
    call vdu_clear_all_buffers

; set up the display
    ld a,8 ; 320x240x64 single-buffered
    call vdu_set_screen_mode
    xor a
    call vdu_set_scaling
	
; start generic stopwatch to time setup loop 
; so we can determine if we're running on emulator or hardware
	call stopwatch_set

; initialize global timestamp
    ld hl,(ix+sysvar_time) ; ix was set by stopwatch_start
    ld (timestamp_now),hl

; enable additional audio channels
	call vdu_enable_channels

; set the cursor off
	call cursor_off

; set text background color
	ld a,4 + 128
	call vdu_colour_text

; set text foreground color
	ld a,47 ; aaaaff lavenderish
	call vdu_colour_text

; set gfx bg color
	xor a ; plotting mode 0
	ld c,4+128 ; dark blue bg
	call vdu_gcol
	call vdu_clg

; set the cursor off again since we changed screen modes
	call cursor_off

; print loading ui message
	ld hl,loading_ui
	call printString

; load UI images
	call load_ui_images

; ; VDU 28, left, bottom, right, top: Set text viewport **
; ; MIND THE LITTLE-ENDIANESS
; ; inputs: c=left,b=bottom,e=right,d=top
; 	ld c,0 ; left
; 	ld d,20 ; top
; 	ld e,39 ; right
; 	ld b,29; bottom
; 	call vdu_set_txt_viewport

; ; load fonts ; TODO
; 	call load_font_rc

; ; load images ; TODO
; 	call img_load_init

; ; load sprites
; 	ld bc,sprites_num_images
; 	ld hl,sprites_image_list
; 	ld (cur_image_list),hl
; 	call img_load_main

; ; load sound effects ; TODO
; 	ld bc,SFX_num_buffers
; 	ld hl,SFX_buffer_id_lut
; 	ld (cur_buffer_id_lut),hl
; 	ld hl,SFX_load_routines_table
; 	ld (cur_load_jump_table),hl
; 	call sfx_load_main

; DEBUG: plot ui images
	ld hl,BUF_SPLASH_BG
	call vdu_buff_select
	ld bc,0
	ld de,0
	call vdu_plot_bmp

	ld hl,BUF_SPLASH_LOGO
	call vdu_buff_select
	ld bc,0
	ld de,0
	call vdu_plot_bmp

; print loading complete message and wait for user keypress
	ld hl,loading_complete
	call printString
	call vdu_flip 
	call waitKeypress

; initialization done
	ret

main_loop_tmr: ds 6
framerate: equ 30

new_game:

	ret

main:


main_loop:
; update global timestamp
    call timestamp_tick

; move player


; move enemies


; render frame

@wait:
	call set_keys
	ld iy,main_loop_tmr
	call tmr_get
	jp z,@continue
	jp m,@continue
	jp @wait
@continue:

; reset main loop timer
	ld iy,main_loop_tmr
	ld hl,120/framerate
	call tmr_set

; check for escape key and quit if pressed
	MOSCALL mos_getkbmap
; 113 Escape
    bit 0,(ix+14)
	jr nz,main_end
@Escape:
	jr main_loop

main_end:
	; call do_outro

    call vdu_clear_all_buffers
	call vdu_disable_channels

; restore screen to something normalish
	xor a
	call vdu_set_screen_mode
	call cursor_on
	ret