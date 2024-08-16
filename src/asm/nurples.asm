; macro files generally want to go here, before any of the other includes
; which call the macro, otherwise the assembler won't have the macro
; available to run when it is called, and will fail with something 
; along the lines of 'invalid label' at such and such a line
    include "nurples/src/asm/macros.inc"

;MOS INITIALIATION MUST GO HERE BEFORE ANY OTHER CODE
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
; ez80asmLinker.py loader code goes here if used.
; ###############################################

; ###############################################
	call	init			; Initialization code
	call 	main			; Call the main function
; ###############################################

exit:

    pop iy                              ; Pop all registers back from the stack
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0                             ; Load the MOS API return code (0) for no errors.

    ret                                 ; Return MOS

; after this we can put includes in any order we wish, even in between
; code blocks if there is any program-dependent or asethetic reason to do so
	include "nurples/src/asm/images2.asm"
	include "nurples/src/asm/fonts.asm"
	include "nurples/src/asm/levels.asm"

	include "nurples/src/asm/sprites.asm"
; API includes
    include "nurples/src/asm/mos_api.inc"
    include "nurples/src/asm/functions.inc"
    include "nurples/src/asm/vdu.inc"
    include "nurples/src/asm/vdu_buff.inc"
    ; include "nurples/src/asm/vdu_plot.inc"
	; include "nurples/src/asm/vdu_sprites.inc"
	; include "nurples/src/asm/vdp.inc"
	include "nurples/src/asm/div_168_signed.inc"
	include "nurples/src/asm/maths24.inc"
; App-specific includes
	include "nurples/src/asm/player.asm"
	include "nurples/src/asm/tiles.asm"
	include "nurples/src/asm/enemies.asm"
	include "nurples/src/asm/laser.asm"
	; include "nurples/src/asm/temp.asm"

hello_world: defb "Hello, World!\n\r",0

init:
; ; set fonts
; 	ld hl,font_nurples
; 	ld b,144 ; loop counter for 96 chars
; 	ld a,32 ; first char to define (space)
; @loop:
; 	push bc
; 	push hl
; 	push af
; 	call vdu_define_character
; 	pop af
; 	inc a
; 	pop hl
; 	ld de,8
; 	add hl,de
; 	pop bc
; 	djnz @loop

; set up the display
    ld a,8
    call vdu_set_screen_mode
    xor a
    call vdu_set_scaling
	ld bc,32
	ld de,16
	call vdu_set_gfx_origin

	call vdu_init ; grab a bunch of sysvars and stuff
	call cursor_off

; ; TESTING SOME MATHS
; 	ld bc,0x00A000 ; 160
; 	ld de,0x007800 ; 120
; 	ld ix,0x011F80 ; 287.5
; 	ld iy,0xFF9B2A ; -100.836
; 	;  hl=0x00FF00 255
; 	call distance168
; 	call dumpRegistersHex
; 	halt
; ; END TESTING SOME MATHS

; ; print a hello message
; 	ld hl,hello_world
; 	call printString

; load the bitmaps
	call bmp2_init

; initialize the first level
	xor a
	ld (cur_level),a
	call init_level

; set gfx viewport to scrolling window
	ld bc,0
	ld de,0
	ld ix,255
	ld iy,239-16
	call vdu_set_gfx_viewport

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
	call vdu_sprite_show

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

; new_game:
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
;     ; ld a,64 ; DEBUG: BRING IT
;     ld (max_enemy_sprites),a 
;     call dt_next_level
;     call dt

; ; spawn our intrepid hero
;     call player_init

; ; #### BEGIN GAME VARIABLES ####
speed_seeker: equ 0x000280 ; 2.5 pixels per frame
speed_player: equ 0x000300 ; 3 pixels per frame

main:
; move the background down one pixel
	ld a,2 ; current gfx viewport
	ld l,2 ; direction=down
	ld h,1 ; speed=1 px
	call vdu_scroll_down

; scroll tiles
	call tiles_plot

; get player input and update sprite position
	call player_input

; move enemies
	call move_enemies

; wait for the next vsync
	call vsync

; poll keyboard
    ld a, $08                           ; code to send to MOS
    rst.lil $08                         ; get IX pointer to System Variables
    
    ld a, (ix + $05)                    ; get ASCII code of key pressed
    cp 27                               ; check if 27 (ascii code for ESC)   
    jp z, main_end                     ; if pressed, jump to exit

    jp main

main_end:
    call cursor_on
	ret


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

;     call vsync
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
;     call vdu_bmp_select
;     ld a,(player_ships)
;     dec a ; we draw one fewer ships than lives
;     ret z ; nothing to draw here, move along
;     ld b,a ; loop counter
;     ld a,256-16 ; initial x position
; draw_lives_loop:
;     ld (sprite_x+1),a
;     push af
;     push bc
;     call vdu_bmp_draw
;     pop bc
;     pop af
;     sub 10
;     djnz draw_lives_loop
;     ret 