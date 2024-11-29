    assume adl=1   
    org 0x040000    
    include "mos_api.inc"
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
	call	init
	call 	main

exit:
    pop iy
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0

    ret

	include "levels.inc"
	include "sprites.inc"
    include "functions.inc"
    include "vdu.inc"
    include "vdu_plot.inc"
	include "vdu_sprites.inc"
	include "maths.inc"
	include "fixed168.inc"
	include "player.inc"
	include "tiles.inc"
	include "enemies.inc"
	include "laser.inc"
	include "timer.inc"
	include "images.inc"
	include "images_sprites.inc"
	include "images_ui.inc"
	include "files.inc"

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

; ; enable additional audio channels
; 	call vdu_enable_channels

; set text background color
	ld a,26+128 ; violet
	call vdu_colour_text

; set text foreground color
	ld a,47 ; aaaaff lavenderish
	call vdu_colour_text

; set gfx bg color
	xor a ; plotting mode 0
	ld a,26+128 ; violet
	call vdu_gcol
	call vdu_cls

; set the cursor off
	call vdu_cursor_off

; VDU 28, left, bottom, right, top: Set text viewport **
; MIND THE LITTLE-ENDIANESS
; inputs: c=left,b=bottom,e=right,d=top
	ld c,0 ; left
	ld d,29 ; top
	ld e,39 ; right
	ld b,29; bottom
	call vdu_set_txt_viewport

; print loading ui message
	ld hl,loading_ui
	call printString
	call vdu_flip

; load UI images
	call load_ui_images

; ; load fonts ; TODO
; 	call load_font_rc

; load sprites
	call img_load_init ; sets up the animated load screen
	call load_sprite_images

; ; load sound effects ; TODO
; 	ld bc,SFX_num_buffers
; 	ld hl,SFX_buffer_id_lut
; 	ld (cur_buffer_id_lut),hl
; 	ld hl,SFX_load_routines_table
; 	ld (cur_load_jump_table),hl
; 	call sfx_load_main

; print loading complete message and wait for user keypress
	call vdu_cls
	ld hl,loading_complete
	call printString
	call vdu_flip 
	call waitKeypress

; set up display for gameplay
    ; ld a,8
	ld a,20
    call vdu_set_screen_mode
    xor a
    call vdu_set_scaling
	call vdu_cursor_off
; plot bezel art
	ld hl,BUF_BEZEL_L
	call vdu_buff_select
	ld bc,0
	ld de,0
	call vdu_plot_bmp
	ld hl,BUF_BEZEL_R
	call vdu_buff_select
	ld bc,384
	ld de,0
	call vdu_plot_bmp
; set gfx origin and viewport to playing field window
	ld bc,origin_left
	ld de,origin_top
	call vdu_set_gfx_origin
	ld bc,field_left
	ld de,field_top
	ld ix,field_right
	ld iy,field_bottom
	call vdu_set_gfx_viewport
; set background color
	ld a,26+128 ; violet
	call vdu_gcol
	call vdu_clg
; VDU 28, left, bottom, right, top: Set text viewport **
	ld c,0 ; left
	ld d,0 ; top
	ld e,62 ; right
	ld b,7; bottom
	call vdu_set_txt_viewport

	ret

; origin_top: equ 48
origin_top: equ 0 ; DEBUG
origin_left: equ 128
field_top: equ 0
field_bottom: equ 383-origin_top
field_left: equ 0
field_right: equ 255
sprite_top: equ 0
sprite_bottom: equ field_bottom-16
sprite_left: equ field_left
sprite_right: equ field_right-16
collide_top: equ %00001000
collide_bottom: equ %00000100
collide_left: equ %00000010
collide_right: equ %00000001

; ; #### BEGIN GAME VARIABLES ####
speed_seeker: equ 0x000280 ; 2.5 pixels per frame
speed_player: equ 0x000300 ; 3 pixels per frame

main:
; start a new game
	call new_game

main_loop:
; scroll tiles
	call tiles_plot

; get player input and update sprite position
	call player_input

; move enemies
	call move_enemies

; wait for the next vblank mitigate flicker and for loop timing
	call vdu_vblank

; poll keyboard
    ld a, $08                           ; code to send to MOS
    rst.lil $08                         ; get IX pointer to System Variables
    
    ld a, (ix + $05)                    ; get ASCII code of key pressed
    cp 27                               ; check if 27 (ascii code for ESC)   
    jp z, main_end                      ; if pressed, jump to exit

    jp main_loop

main_end:
    call vdu_cursor_on
	ret

new_game:
; initialize sprites
	call sprites_init

; initialize the first level
	xor a
	ld (cur_level),a
	call init_level

; initialize player
	call player_init

; spawn an enemy sprite
	ld b,table_max_records
@spawn_enemy_loop:
	push bc
	call enemy_init_from_landing_pad
	pop bc
	djnz @spawn_enemy_loop

	ret

; ; ###### INITIALIZE GAME #######
; ; clear the screen
;     ld a,3
;     out (81h),a

; ; reset the sprite table
;     xor a
;     ld (table_active_sprites),a
;     ld hl,table_limit
;     ld (table_base),hl
;     ld (table_pointer),hl

; ; draw a starfield over the entire screen
;     ld b,#50 ; first row of visible screen
; new_game_draw_stars_loop:
;     push bc
;     call draw_stars
;     pop bc
;     ld a,#10
;     add a,b
;     ld b,a
;     jr nz,new_game_draw_stars_loop

; ; ; print a welcome message
; ;     ld de,msg_welcome
; ;     ld hl,#581C
; ;     ld c,218 ; a bright pastel purple d677e3
; ;     call print_string

; ; push all that to frame buffer
;     ld a,#01 ; send video to frame buffer
;     out (81h),a

; ; reset score, lives, shields
;     xor a
;     ld hl,player_score
;     ld (hl),a ; player_score 0
;     inc hl
;     ld (hl),a ; player_score 1
;     inc hl
;     ld (hl),a ; player_score 3
;     inc hl
;     ld a,16
;     ld (hl),a ; player_shields
;     inc hl
;     ld (hl),a ; player_max_shields
;     inc hl
;     ld a,3
;     ld (hl),a ; player_ships
;     inc hl

; ; initialize first level
;     ld a,1 ; levels are zero-based, so this will wrap around
;     ld (cur_level),a
;     ld a,3 ; set max enemy sprites to easy street
;     ld (max_enemy_sprites),a 
;     call dt_next_level
;     call dt

; ; spawn our intrepid hero
;     call player_init


; ; #### BEGIN GAME MAIN LOOP ####
; main_loop:
; ; ; debug: start execution counter 
; ;     ld a,1
; ;     out (#e0),a ; start counting instructions
    
; ; refresh background from frame buffer
;     ld a,#02
;     out (81h),a
;     call move_background ; now move it
;     ld a,#01
;     out (81h),a ; save it back to buffer
; ; do all the things
;     call move_enemies
;     call player_move
;     call laser_control
;     call print_score
;     call draw_shields
;     call draw_lives
; ; ; debug: stop execution counter and print results
; ;     ld a,0
; ;     out (#e0),a ; stop counting instructions

; ; ; debug: start execution counter 
; ;     ld a,1
; ;     out (#e0),a ; start counting instructions

;     call vdu_vblank
; ; ; debug: stop execution counter and print results
; ;     ld a,0
; ;     out (#e0),a ; stop counting instructions

;     jr main_loop
; #### END GAME MAIN LOOP ####

; draws the player's shields level
; draw_shields:
; TODO: Agonize this routine
; ; prep the loop to draw the bars
;     ld a,(player_shields) ; snag shields
;     and a 
;     ret z ; don't draw if zero shields
; ; set loop counter and drawing position
;     ld b,a ; loop counter
;     ld hl,#5300+48+12
; ; set color based on bars remaining
;     ld c,103 ; bright green 28fe0a
;     cp 9
;     jp p,draw_shields_loop
;     ld c,74 ; bright yellow eafe5b 
;     cp 3
;     jp p,draw_shields_loop
;     ld c,28 ; bright red fe0a0a 
; draw_shields_loop:
;     push bc ; yup,outta
;     push hl ; registers again
;     ; ld a,#A8 ; ▀,168 
;     ld a,10 ; ▀,168 ; we renumber because we don't use the full charset
;     ; call draw_char
;     call draw_num ; we nuked draw_char for the time being
;     pop hl
;     ld a,8
;     add a,l
;     ld l,a
;     pop bc
;     djnz draw_shields_loop
    ; ret

; prints the player's score
; print_score:
; TODO: Agonize this
; ; draw score (we do it twice for a totally unecessary drop-shadow effect)
;     ld c,42 ; dark orange b74400
;     ld hl,#5200+1+8+6*6
;     ld a,3 ; print 6 bdc digits
;     ld de,player_score
;     call print_num

;     ld c,58 ; golden yellow fec10a
;     ld hl,#5100+8+6*6
;     ld a,3 ; print 6 bdc digits
;     ld de,player_score
;     call print_num
    ; ret

; draw_lives:
;     ld hl,player_small ; make small yellow ship the active sprite
;     ld (sprite_base_bufferId),hl
;     ; ld a,#80 ; northern orientation
;     ; ld (sprite_orientation),a
;     ld hl,0 ; north
;     ld (sprite_heading),hl
;     xor a
;     ld (sprite_animation),a
;     ld a,#56 ; top of visible screen
;     ld (sprite_y+1),a
;     call vdu_bmp_select ; TODO: convert to vdu_buff_select
;     ld a,(player_ships)
;     dec a ; we draw one fewer ships than lives
;     ret z ; nothing to draw here, move along
;     ld b,a ; loop counter
;     ld a,256-16 ; initial x position
; draw_lives_loop:
;     ld (sprite_x+1),a
;     push af
;     push bc
;     call vdu_bmp_draw ; convert to vdu_bmp_plot
;     pop bc
;     pop af
;     sub 10
;     djnz draw_lives_loop
;     ret 