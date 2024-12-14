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
    include "arith24.inc"
    include "maths.inc"
    include "files.inc"
    include "fixed168.inc"
    include "fonts.inc"
    include "images.inc"
    include "timer.inc"
    include "vdu.inc"
    include "vdu_fonts.inc"
    include "vdu_plot.inc"
    include "vdu_sprites.inc"

; APPLICATION INCLUDES
    include "collisions.inc"
    include "enemies.inc"
    include "enemy_fireball.inc"
    include "enemy_seeker.inc"
    include "explosion.inc"
    include "fonts_list.inc"
    include "images_tiles_dg.inc"
    ; include "images_tiles_xevious.inc"
    include "images_sprites.inc"
    include "images_ui.inc"
    ; include "laser.inc"
    include "levels.inc"
    include "levels_tileset_0.inc"
    ; include "levels_xevious.inc"
    include "player.inc"
    include "player_laser.inc"
    include "player_weapons.inc"
    include "state.inc"
    include "targeting.inc"
    include "tile_table.inc"
    include "tiles.inc"
    include "tiles_active.inc"
    include "tile_pad_small.inc"
    include "tile_turret_fireball.inc"
    include "sprites.inc"
    include "sprites_new.inc"

    align 256

; --- MAIN PROGRAM FILE ---
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
    ld d,0 ; top
    ld e,39 ; right
    ld b,0; bottom
    call vdu_set_txt_viewport

; print loading ui message
    ld hl,loading_ui
    call printString
    call vdu_flip

; load UI images
    call load_ui_images

; load sprites
    call img_load_init ; sets up the animated load screen
    call load_sprite_images

; load tileset_ptrs
    call load_tilesets

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
    ; call waitKeypress

; set up display for gameplay
    ; ld a,8
    ld a,20
    call vdu_set_screen_mode
    xor a
    call vdu_set_scaling
    call vdu_cursor_off
; load fonts
	call fonts_load
; select font
    ; ld hl,computer_pixel_7_8x16
    ld hl,amiga_forever_8x8 ; DEBUG
    ld a,1 ; flags
    call vdu_font_select
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
    ld b,48; bottom
    call vdu_set_txt_viewport

; initialize the global timestamp
    call timestamp_tick

; done with init
    ret

main:
; start a new game
    call game_initialize

; DEBUG
    ld hl,128 ; x
    ld (tiles_x_plot),hl
    ld hl,0 ; y
    ld (tiles_y_plot),hl
    call activate_turret_fireball

@main_loop:
    call vdu_clg
    call vdu_home_cursor

; move_tiles:
; are there any active tiles?
    ld a,(num_active_tiles)
    and a ; will be zero if no alive tiles
    ret z ; if no active tiles, we're done
; initialize pointers and loop counter
    ld iy,tile_stack ; set iy to first record in table
@move_loop:
    ld (tile_stack_pointer),iy ; update stack pointer
    ld hl,(iy)
    sign_hlu ; check for list terminator
    ret z ; end of stack so we're done
; point iy to tile record
    ld iy,(iy) ; iy points to the current tile record
    ld (tile_table_pointer),iy ; update table pointer
; check top bit of tile_type to see if tile is just spawned
    ld a,(iy+tile_type)
    bit 7,a
    ; jp nz,@just_spawned ; if just spawned, skip to next record
; otherwise we prepare to move the tile
    ld hl,(iy+tile_move_program) ; load the behavior subroutine address
    callHL
; move_tiles_loop_return: return from behavior subroutines
    ld iy,(tile_table_pointer) ; get back table pointer
    JP @next_record ; DEBUG
    
; now we check results of all the moves
    bit tile_just_died,(iy+tile_collisions)
    jp z,@next_record ; if not dead, go to next record
    call table_deactivate_tile ; otherwise, deactivate tile
    ld iy,(tile_stack_pointer) ; get back stack pointer
    jp @move_loop
@just_spawned:
    res 7,(iy+tile_type) ; clear just spawned flag
    ; fall through to @next_record
@next_record:
    ; CALL DEBUG_PRINT
    ; CALL DEBUG_WAITKEYPRESS

    ld iy,(tile_stack_pointer)
    lea iy,iy+3 ; next tile stack record
    jp @move_loop ; loop until we've checked all the records
; end move_tiles

; poll keyboard for escape keypress
    ld a, $08 ; code to send to MOS
    rst.lil $08 ; get IX pointer to System Variables
    ld a, (ix + $05) ; get ASCII code of key pressed
    cp 27 ; check if 27 (ascii code for ESC)   
    jp z, main_end ; if pressed, jump to exit
    
    call vdu_vblank

    jp @main_loop
; END DEBUG

main_loop:
; update the global timestamp
    call timestamp_tick
; do gamestate logic
    call do_game

; DEBUG
    ; CALL DEBUG_PRINT_TABLE
    ; CALL DEBUG_WAITKEYPRESS
; END DEBUG

; wait for the next vblank mitigate flicker and for loop timing
    call vdu_vblank
    ; call vdu_vblank ; DEBUG
    ; call vdu_vblank ; DEBUG

; poll keyboard for escape keypress
    ld a, $08 ; code to send to MOS
    rst.lil $08 ; get IX pointer to System Variables
    ld a, (ix + $05) ; get ASCII code of key pressed
    cp 27 ; check if 27 (ascii code for ESC)   
    jp z, main_end ; if pressed, jump to exit
    
; escape not pressed so loop
    jp main_loop

main_end:
    call vdu_cursor_on
    ret

DEBUG_PRINT:
    PUSH_ALL
    ld c,0
    ld b,0
    call vdu_move_cursor
    POP_ALL
    PUSH_ALL
    call dumpFlags
    POP_ALL
    PUSH_ALL
    call dumpRegistersHex
    ; call waitKeypress
    POP_ALL
    ret

DEBUG_PRINT_TABLE:
    PUSH_ALL
    ; call vdu_home_cursor
    ld c,0
    ld b,0
    call vdu_move_cursor

    ; ld a,(player_weapons_count)
    ; call printHexA
    ; call printNewLine

    ; LIST_FIELD sprite_move_program,3 ; DEBUG
    ; LIST_FIELD sprite_type,1 ; DEBUG

    ld ix,player_weapons_begin
    call dump_sprite_record
    call printNewLine
    call printNewLine

    lea ix,ix+table_record_size
    call dump_sprite_record
    call printNewLine
    call printNewLine

    lea ix,ix+table_record_size
    call dump_sprite_record
    call printNewLine
    call printNewLine

    lea ix,ix+table_record_size
    call dump_sprite_record
    call printNewLine
    call printNewLine

    ; ld ix,player_begin
    ; call dump_sprite_record

    ; call waitKeypress
    POP_ALL
    RET

DEBUG_WAITKEYPRESS:
    PUSH_ALL
    call waitKeypress
    POP_ALL
    RET

DEBUG_PRINT_FIELDS:
    ; PUSH_ALL
    ld bc,0
    ld c,a
    ld ix,table_base
    add ix,bc
    ld b,table_num_records
@@:
    push ix
    pop hl
    push bc ; save loop counter
    ld a,1 ; print one byte
    call dumpMemoryHex
    lea ix,ix+table_record_size
    pop bc ; restore loop counter
    djnz @b
    ; POP_ALL
    ret

    align 256
    include "tables.inc"