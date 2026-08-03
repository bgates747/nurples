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
	call printInline
	asciz "Loading font: "
	call fonts_load
	jp nc,@font_load_error
	call printNewLine
; select font
    ld hl,Lat38_VGA8_8x8
    ld a,1 ; flags
    call vdu_font_select
; load UI images
	call printInline
	asciz "Loading UI: "
    call load_ui_images
	jp nz,@agnb_load_error
	call printNewLine
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
; load all backgrounds, sprites, and active DG tiles from one container
    call load_game_images
    jp nz,@agnb_load_error
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
    jp @assets_loaded

@font_load_error:
    call printNewLine
    call printInline
    asciz "Font load failed. Press any key to return to MOS.\r\n"
    call waitKeypress
    ld a,1
    ld (game_exit_requested),a
    ret

@agnb_load_error:
; Preserve the loader result while returning to a predictable full screen.
    push af
    ld a,(original_screen_mode)
    call vdu_set_screen_mode
    call vdu_cls
    call printInline
    asciz "AGNB image load failed: "
    pop af
    call printHex8
    call printNewLine
    call printInline
    asciz "Press any key to return to MOS.\r\n"
    call waitKeypress
    ld a,1
    ld (game_exit_requested),a
    ret
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
; initialize new-session values before init_level displays them
    ld hl,0
    ld (player_score),hl
    ld (player_starting_score),hl
    ld a,(player_ships_max)
    ld (player_ships),a
; initialize runtime tables and the first level
    call reset_runtime_tables
    ld a,tiles_dg
    call init_tileset
; initialize player
    call draw_player_ships
    call player_init
    ret nc ; retry game_init next frame rather than publishing an invalid player
; set the game state to playing
    ld hl,game_playing
    ld (game_state), hl
; set game playing substates
    ld hl,enemies_alive
    ld (enemies_state), hl
; done initializing
    ret
; end game_init

; Reset only shared runtime infrastructure. Session and level semantics remain
; with their respective callers.
reset_runtime_tables:
    call table_init
    ld hl,table_base
    ld (table_pointer),hl
    call sprites_init
    call table_init
    call tile_table_init
    xor a
    ld (player_event),a
    ret
; end reset_runtime_tables
