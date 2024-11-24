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
	include "macros.inc" ; DEBUG
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
	include "font_rc.inc"
    ; include "images.inc"
    ; include "images_sprites.inc"
	include "images2.asm" ; DEBUG
	include "images_ui.inc"
	include "player.inc"
	include "laser.inc"
	include "sprites.inc"
	include "tiles.inc"
	include "levels.inc"
	include "enemies.inc"

hello_world: asciz "Welcome to Purple Nurples!"
loading_ui: asciz "Loading UI"
loading_time: asciz "Loading time:"
loading_complete: asciz "Press any key to continue."

init:
; clear all buffers
    call vdu_clear_all_buffers

; set up the display
    ld a,8;+128 ; 136   320   240   64    60hz double-buffered
    call vdu_set_screen_mode
    xor a
    call vdu_set_scaling

; enable additional audio channels
	call vdu_enable_channels

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

; set the cursor off
	call vdu_cursor_off

; ; VDU 28, left, bottom, right, top: Set text viewport **
; ; MIND THE LITTLE-ENDIANESS
; ; inputs: c=left,b=bottom,e=right,d=top
; 	ld c,0 ; left
; 	ld d,29 ; top
; 	ld e,39 ; right
; 	ld b,29; bottom
; 	call vdu_set_txt_viewport

; ; print loading ui message
; 	ld hl,loading_ui
; 	call printString
; 	call vdu_flip

; ; load UI images
; 	call load_ui_images

; ; ; load fonts ; TODO
; ; 	call load_font_rc

; ; initialize animated splash screen during assets loading
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

; ; print loading complete message and wait for user keypress
; 	call vdu_cls
; 	ld hl,loading_complete
; 	call printString
; 	call vdu_flip 
; 	call waitKeypress

; set up display for gameplay
    ld a,8
    call vdu_set_screen_mode
    xor a
    call vdu_set_scaling
	ld bc,32
	ld de,16
	call vdu_set_gfx_origin
	call vdu_cursor_off
; set gfx viewport to scrolling window
	ld bc,0
	ld de,0
	ld ix,255
	ld iy,239-16
	call vdu_set_gfx_viewport

; load sprite bitmaps
	call bmp2_init
	
; initialize sprites
	call vdu_sprite_reset ; out of an abundance of caution (copilot: and paranoia)
	xor a
@sprite_loop:
	push af
	call vdu_sprite_select
	ld hl,BUF_0TILE_EMPTY ; can be anything, but why not blank?
	call vdu_sprite_add_buff
	pop af
	inc a
	cp table_max_records+1 ; tack on sprites for player and laser
	jr nz,@sprite_loop
	inc a
	call vdu_sprite_activate

; define player sprite
	ld a,16
	call vdu_sprite_select
	call vdu_sprite_clear_frames
	ld hl,BUF_SHIP_0L
	ld bc,3 ; three bitmaps for player ship
@sprite_player_loop:
	push bc
	push hl
	call vdu_sprite_add_buff
	pop hl
	inc hl
	pop bc
	djnz @sprite_player_loop

; initialization done
	ret

main:
	; call new_game

; initialize player
	call player_init
	call vdu_sprite_show

; spawn an enemy sprite
	ld b,table_max_records
@spawn_enemy_loop:
	push bc
	call enemy_init_from_landing_pad
	pop bc
	djnz @spawn_enemy_loop

main_loop:
; move player

; move enemies

; move tiles

; render ui

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
	call vdu_cursor_on
	ret

new_game:
; initialize the first level
	xor a
	ld (cur_level),a
	call init_level
; initialize player
	call player_init
	ret