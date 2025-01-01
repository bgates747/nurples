; hello_world: asciz "Welcome to Purple Nurples!"
; loading_time: asciz "Loading time:"
loading_complete: asciz "Press any key to continue."
game_assets_loaded: db 0 ; non-zero when assets are loaded
original_screen_mode: db 0

game_init:
; check whether assets are loaded
    ld a,(game_assets_loaded)
    or a
    jp nz,@assets_loaded
; get current screen mode and save it so we can return to it on exit
    call vdu_get_screen_mode
    ld (original_screen_mode),a
; print ascii art splash screen
    call vdu_cls
    call printInline
    asciz "Welcome to\r\n"
    ld hl,purple_nurples_ascii
    call printString
    call printInline
    asciz "Loading UI...\r\n"
; clear all buffers
    call vdu_clear_all_buffers
; load fonts
	call fonts_load
; select font
    ld hl,Lat38_VGA8_8x8
    ld a,1 ; flags
    call vdu_font_select
; load UI images
    call load_ui_images
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
; load background and sprite images
    call img_load_init
    call load_backgrounds
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
    ld a,1
    ld (game_assets_loaded),a
    call vdu_cls
    call printInline
    asciz "Press any key to continue."
    call vdu_flip 
    call waitKeypress
; set up display for gameplay
    ld a,20
    call vdu_set_screen_mode
    xor a
    call vdu_set_scaling
    call vdu_cursor_off
; select font
    ld hl,Lat38_VGA8_8x8
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
; draw player cockpit
    call draw_player_cockpit
@assets_loaded:
; set gfx origin and viewport to playing field window
    call set_gfx_playing_field
    call init_bg ; plot the starry background
; VDU 28, left, bottom, right, top: Set text viewport **
    ld c,0 ; left
    ld d,0 ; top
    ld e,62 ; right
    ld b,48; bottom
    call vdu_set_txt_viewport
; initialize the global timestamp
    call timestamp_tick
; reset the sprite table
    call table_init
    ld hl,table_base
    ld (table_pointer),hl
; initialize sprites and active tiles table
    call sprites_init
    call table_init
    call tile_table_init
; initialize the first level
    ld a,tiles_dg
    call init_tileset
; initialize player
    ld hl,0
    ld (player_score),hl
    ld a,(player_ships_max)
    ld (player_ships),a
    call draw_player_ships
    call player_init
; set the game state to playing
    ld hl,game_playing
    ld (game_state), hl
; set game playing substates
    ld hl,player_alive
    ld (player_state), hl
    ld hl,enemies_alive
    ld (enemies_state), hl
; done initializing
    ret
; end game_init