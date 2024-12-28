
game_init:
; set up display for gameplay
    ld a,20
    call vdu_set_screen_mode
    xor a
    call vdu_set_scaling
    call vdu_cursor_off
; load fonts
	call fonts_load
; select font
    ld hl,amiga_forever_8x8
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
; reset the sprite table
    call table_init
    ld hl,table_base
    ld (table_pointer),hl
    ld (player_score_last),hl ; anything but zero so print_score will update
; initialize sprites and active tiles table
    call sprites_init
    call table_init
    call tile_table_init
; initialize the first level
    ld a,tiles_dg
    call init_tileset
; initialize player
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