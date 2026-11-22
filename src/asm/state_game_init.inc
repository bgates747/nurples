original_screen_mode: db 0
original_text_foreground: db c_white
original_text_background: db c_black
; bit 0 = foreground valid, bit 1 = background valid
original_text_colours_valid: db 0

game_init:
    .if mus_all50
    ld a,(lp_started)
    or a
    jr z,@new_run
    ld (game_exit_requested),a ; corrupt dispatch re-entered init; unwind the owned timer
    ret
@new_run:
    .endif
    ld a,STATE_NEW_LOADING
    call telemetry_publish_phase
; The VDP alone owns the truth about its resource registries. Never infer that
; assets survived a prior game or MOS RUN from application RAM. Select the
; permanent system font before deleting the custom-font buffer, detach sprites
; from their bitmap pointers, then rebuild every runtime VDP resource.
    ld hl,65535
    xor a
    call vdu_font_select
    call vdu_sprite_reset
    call vdu_clear_all_buffers
; Select or reuse the full playfield before resident assets consume VDP memory.
    call playfield_display_select
    jp nc,presentation_flow_fail
    ld a,26 ; reset viewports and origin even when MODE was skipped
    rst.lil 10h
    call vdu_select_text_cursor
; print ascii art splash screen
    call vdu_cls
    ld hl,purple_nurples_ascii
    call printString
    call printInline
    asciz "Loading UI...\r\n"
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
; Set up the splash in the already-verified 64-colour display.
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
    ld c,26+128 ; violet
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
    .if mus_campaign
    ld de,vox_filename
    call agnb_load_audio
    jp nz,@agnb_load_error
    .endif
; Keep the splash visible for the bounded controller choice. Only a valid
; choice enters the resident-asset attract state and its fresh start gate.
    ld a,STATE_NEW_EVENT_LOAD_COMPLETE
    call telemetry_post_events
    ld a,STATE_NEW_TITLE
    call telemetry_publish_phase
    call game_start_config_begin
    ret

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
; end game_init

; A = display checkpoint. Verify and redraw the existing playfield surface
; without reallocating it. Carry clear on a rejected display.
game_prepare_playfield:
    call playfield_display_verify
    ret nc
    ld a,26 ; reset text/graphics viewports and origin after loading/config
    rst.lil 10h
    call vdu_select_text_cursor
    xor a
    call vdu_set_scaling
    call vdu_cursor_off
    ld a,c_black+128
    call vdu_colour_text
    ld a,c_white
    call vdu_colour_text
    xor a
    ld c,c_black+128
    call vdu_gcol
    call vdu_cls
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
; set gfx origin and viewport to playing field window
    call set_gfx_playing_field
; The resident RAM value may name the last level's background after an
; in-process restart. Publish the canonical starfield before plotting it.
    ld hl,BUF_BG_STARS_0
    ld (bg_bufferId),hl
    call init_bg ; plot the starry background
; VDU 28, left, bottom, right, top: Set text viewport **
    ld c,0 ; left
    ld d,0 ; top
    ld e,62 ; right
    ld b,48; bottom
    call vdu_set_txt_viewport
; initialize the global timestamp
    call timestamp_tick
    scf
    ret
; end game_prepare_playfield

; Initialize one clean playable session from already resident assets.
; outputs: carry set after publishing game_playing; carry clear on player init
;          failure, with the caller's global state left in place for recovery
game_new_session_init:
    ld a,STATE_NEW_EVENT_START
    call telemetry_post_events
    ld a,STATE_NEW_NEW_GAME
    call telemetry_publish_phase
; initialize new-session values before init_level displays them
    ld hl,0
    ld (player_score),hl
    ld (player_starting_score),hl
    ld a,(player_ships_max)
    ld (player_ships),a
; initialize runtime tables and the first level
    call reset_runtime_tables
    ld a,STATE_NEW_LEVEL_ENTER
    call telemetry_publish_phase
    ld a,tiles_dg
    call init_tileset
; initialize player
    call draw_player_ships
    call player_init_level_entry
    ret nc ; caller retains its state rather than publishing an invalid player
; set the game state to playing
    ld hl,game_playing
    ld (game_state), hl
; set game playing substates
    ld hl,enemies_alive
    ld (enemies_state), hl
; done initializing
    scf
    ret
; end game_new_session_init

; Use the existing one-line splash viewport; Y/N is case-insensitive.
; Every visit chooses afresh, including a game restart without reloading RAM.
choose_joystick:
    call vdu_cls
    call printInline
    asciz "Enable joystick (y/n)?"
    call vdu_flip
@choice:
    call waitKeypress
    or 020h
    cp 'y'
    jr z,@yes
    cp 'n'
    jr nz,@choice
    call player_joystick_disable
    jr @release
@yes:
    call player_joystick_enable
@release:
; Do not let a held/repeating Y or N also consume the subsequent start prompt.
; Read the held-key map, rather than the last-event-only vkeydown sysvar.
    MOSCALL mos_getkbmap
@scan:
    push ix
    pop hl
    ld b,16
    xor a
@byte:
    or (hl)
    inc hl
    djnz @byte
    jr nz,@scan
    ret
; end choose_joystick

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
    ld (player_death_pending),a
    ret
; end reset_runtime_tables
