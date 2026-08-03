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
    include "macros.inc"
    include "functions.inc"
    include "arith24.inc"
    include "maths.inc"
    include "files.inc"
    include "fixed168.inc"
    include "fonts.inc"
    include "images_agnb.inc"
    include "images.inc"
    include "timer.inc"
    include "vdu.inc"
    include "vdu_buffered_api.inc"
    include "vdu_fonts.inc"
    include "vdu_plot.inc"
    include "vdu_sprites.inc"
    include "agnb_api.inc"

; APPLICATION INCLUDES
    include "state_ids.inc"
    include "ascii.inc"
    include "collisions.inc"
    include "enemies.inc"
    include "enemy_fireball.inc"
    include "enemy_seeker.inc"
    include "explosion.inc"
    include "fonts_list.inc"
    include "images_bg.inc"
    include "images_tiles_dg.inc"
    include "images_sprites.inc"
    include "images_ui.inc"
    include "levels.inc"
    include "levels_tileset_0.inc"
    include "player_state.inc"
    include "player_cockpit.inc"
    include "player_fuel.inc"
    include "player_input.inc"
    include "player_laser.inc"
    include "player_score.inc"
    include "player_shields.inc"
    include "screen.inc"
    include "sprites.inc"
    include "state.inc"
    include "state_game_init.inc"
    include "state_game_playing.inc"
    include "targeting.inc"
    include "tile_table.inc"
    include "tiles.inc"
    include "tiles_active.inc"
    include "tile_crater.inc"
    include "tile_electrode.inc"
    include "tile_lightning.inc"
    include "tile_pad_small.inc"
    include "tile_turret_fireball.inc"
    include "debug.inc"
    include "telemetry.inc"
    include "runtime_init.inc"

; --- MAIN PROGRAM FILE ---
main:
; Invalidate and rebuild the public observer record before touching any other
; resident process state. MOS RUN does not reload initialized data.
    call telemetry_init
    call runtime_process_init
; Capture the caller's display mode exactly once, before game_init changes VDP
; state. In-process restarts must not replace this with the gameplay mode.
    call vdu_get_screen_mode
    ld (original_screen_mode),a
; MODE destroys VDP contexts, but the current logical text colours are exposed
; through the MOS screen-pixel result packet. Preserve each value independently
; so a failed VDP query degrades to the explicit white-on-black handoff.
    xor a
    ld (original_text_colours_valid),a
    ld a,128
    call vdu_get_text_colour
    jp nc,@capture_background
    ld (original_text_foreground),a
    ld a,(original_text_colours_valid)
    or %00000001
    ld (original_text_colours_valid),a
@capture_background:
    ld a,129
    call vdu_get_text_colour
    jp nc,@capture_complete
    ld (original_text_background),a
    ld a,(original_text_colours_valid)
    or %00000010
    ld (original_text_colours_valid),a
@capture_complete:
main_loop:
; update the global timestamp
    call timestamp_tick
; do gamestate logic
    call do_game
; state handlers return through the dispatcher; honor an orderly exit request
    ld a,(game_exit_requested)
    or a
    jp nz,@state_exit
; wait for the next vblank mitigate flicker and for loop timing
    call vdu_vblank
; poll keyboard for escape keypress
    ld a, $08 ; code to send to MOS
    rst.lil $08 ; get IX pointer to System Variables
    ld a, (ix + $05) ; get ASCII code of key pressed
    cp 27 ; check if 27 (ascii code for ESC)   
    jp z,@escape
    call telemetry_tick
; escape not pressed so loop
    jp main_loop

@state_exit:
    call telemetry_request_exit
    call telemetry_publish_terminal
    jp main_end
@escape:
    call telemetry_request_exit
    call telemetry_tick
    jp main_end

main_end:
; Release application-owned VDP state. Select the permanent system font before
; deleting the custom-font buffer, and detach sprites before their bitmaps.
    ld hl,65535
    xor a
    call vdu_font_select
    call vdu_sprite_reset
    call vdu_clear_all_buffers
; restore original screen mode
    ld a,(original_screen_mode)
    call vdu_set_screen_mode
; MODE resets the context. Reassert a permanent font and deterministic farewell
; colours explicitly: white text on a freshly cleared black text background.
    call vdu_select_text_cursor
    ld hl,65535
    xor a
    call vdu_font_select
    ld a,c_black+128
    call vdu_colour_text
    ld a,c_white
    call vdu_colour_text
    call vdu_cls
    call vdu_cursor_on
    call printInline
    asciz "Thank you for playing\r\n"
    ld hl,purple_nurples_ascii
    call printString
; Leave subsequent MOS text using the caller's logical colours when the VDP
; reported them. The already-rendered farewell remains white on black.
    ld a,(original_text_colours_valid)
    bit 1,a
    jp z,@restore_foreground
    ld a,(original_text_background)
    or 128
    call vdu_colour_text
@restore_foreground:
    ld a,(original_text_colours_valid)
    bit 0,a
    jp z,@display_restored
    ld a,(original_text_foreground)
    call vdu_colour_text
@display_restored:
    ret
; end main

; needs to be the final include to leave room for the sprite and tile tables
    include "tables.inc"
