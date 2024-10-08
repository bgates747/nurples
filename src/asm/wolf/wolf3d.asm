    .assume adl=1   
    .org 0x040000    

    jp start       

    .align 64      
    .db "MOS"       
    .db 00h         
    .db 01h

	include "src/asm/mos_api.asm" ; wants to be first include b/c it has macros
	include "src/asm/vdu_sound.asm" ; also has macros
	include "src/asm/images.asm"
	include "src/asm/fonts_bmp.asm"
	include "src/asm/maps.asm"
	include "src/asm/render.asm"
	include "src/asm/polys.asm"
	include "src/asm/font_rc.asm"
	include "src/asm/font_rc.asm"
	include "src/asm/ui.asm"
	include "src/asm/ui_img.asm"
	include "src/asm/ui_img_bj.asm"
	include "src/asm/sprites.asm"
	include "src/asm/vdu.asm"
    include "src/asm/functions.asm"
	include "src/asm/player.asm"
	include "src/asm/maths.asm"
	include "src/asm/img_load.asm"
	include "src/asm/sfx.asm"
	include "src/asm/timer.asm"


start:              
    push af
    push bc
    push de
    push ix
    push iy

	call init ; Initialization code
    call main ; Call the main function

exit:

    pop iy 
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0

    ret 

hello_world: defb "Welcome to Agon Wolf3D",0
loading_ui: defb "Loading UI",0
loading_time: defb "Loading time:",0
loading_complete: defb "Press any key to continue.\r\n",0
is_emulator: defb 0
on_emulator: defb "Running on emulator.\r\n",0
on_hardware: defb "Running on hardware.\r\n",0

init:
; clear all buffers
    call vdu_clear_all_buffers

; set up the display
    ld a,8+128 ; 320x240x64 double-buffered
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

; print loading ui message
	ld hl,loading_ui
	call printString

; load fonts
	call load_font_rc
	call load_font_rc

; load UI images
	call load_ui_images
	call load_ui_images_bj

; set text background color
	ld a,4 + 128
	call vdu_colour_text

; set text foreground color
	ld a,47 ; aaaaff lavenderish
	call vdu_colour_text

; set gfx bg color
	xor a ; plotting mode 0
	ld c,4 ; dark blue
	call vdu_gcol_bg
	call vdu_clg

; set the cursor off again since we changed screen modes
	call cursor_off

; VDU 28, left, bottom, right, top: Set text viewport **
; MIND THE LITTLE-ENDIANESS
; inputs: c=left,b=bottom,e=right,d=top
	ld c,0 ; left
	ld d,20 ; top
	ld e,39 ; right
	ld b,29; bottom
	call vdu_set_txt_viewport

; initialize image load routine
	call img_load_init

; load panels
	ld bc,cube_num_panels
	ld hl,cube_buffer_id_lut
	ld (cur_buffer_id_lut),hl
	ld hl,cube_load_panels_table
	ld (cur_load_jump_table),hl
	call img_load_main

; load sprites
	ld bc,sprite_num_panels
	ld hl,sprite_buffer_id_lut
	ld (cur_buffer_id_lut),hl
	ld hl,sprite_load_panels_table
	ld (cur_load_jump_table),hl
	call img_load_main

; load distance walls
	ld bc,dws_num_panels
	ld hl,dws_buffer_id_lut
	ld (cur_buffer_id_lut),hl
	ld hl,dws_load_panels_table
	ld (cur_load_jump_table),hl
	call img_load_main

; load sound effects
	ld bc,SFX_num_buffers
	ld hl,SFX_buffer_id_lut
	ld (cur_buffer_id_lut),hl
	ld hl,SFX_load_routines_table
	ld (cur_load_jump_table),hl
	call sfx_load_main

; self modify vdu_play_sfx to enable sound
	xor a
	ld (vdu_play_sfx_disable),a

; use loading time to determine if we're running on emulator or hardware
	call stopwatch_get ; hl = elapsed time in 120ths of a second
	ld de,8000 ; emulator loads in about 2,400 ticks, hardware about 15,000
	xor a ; clear carry, default is running on hardware
	ld (is_emulator),a
	sbc hl,de
	jp m,@on_emulator
	call vdu_home_cursor
	ld hl,on_hardware
	call printString
	jp @test_done

@on_emulator:
; print emulator message
	ld a,1
	ld (is_emulator),a
	call vdu_home_cursor
	ld hl,on_emulator
	call printString

@test_done:
; print final loading time
	ld hl,loading_time
	call printString
	call stopwatch_get ; hl = elapsed time in 120ths of a second
	call printDec
	call printNewLine

; print loading complete message and wait for user keypress
	ld hl,loading_complete
	call printString
	call vdu_flip 
	call waitKeypress

; initialization done
	ret

; DEBUG: set up a simple countdown timer
debug_timer: db 0x01

main_loop_tmr: ds 6
framerate: equ 30

new_game:
; initialize map variables and load map file
	ld hl,room_flags
	xor a
	ld b,10
@room_flags_loop:
	ld (hl),a
	inc hl
	djnz @room_flags_loop
; map_init:
	ld (cur_floor),a
	ld (cur_room),a
; load room file
	call map_load
; initialize sprite data
	call map_init_sprites
; initialize player position
	call plyr_init

	ret

main:
	call new_game

; main:
; ; set map variables and load initial map file
; 	call map_init
; ; initialize player position
; 	call plyr_init


main_loop:
; update global timestamp
    call timestamp_tick

; move enemies
	call sprites_see_plyr ; 220-285  prt ticks

; get player input and update sprite position
	; 0-1 prt ticks
	call plyr_input ; ix points to cell defs/status, a is target cell current obj_id

; render the updated scene
	call render_scene ; 6-12 prt ticks
; full loop 12-16 prt ticks

; flip the screen
	call vdu_flip

@wait:
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

; files.asm must go here so that filedata doesn't stomp on program data
	include "src/asm/files.asm"
