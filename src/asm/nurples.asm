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
    include "runtime_init.inc"

; --- MAIN PROGRAM FILE ---
main:
; Establish a new process lifetime. MOS RUN does not reload initialized data.
    call runtime_process_init
; Capture the caller's display mode exactly once, before game_init changes VDP
; state. In-process restarts must not replace this with the gameplay mode.
    call vdu_get_screen_mode
    ld (original_screen_mode),a
main_loop:
; update the global timestamp
    call timestamp_tick
; do gamestate logic
    call do_game
; state handlers return through the dispatcher; honor an orderly exit request
    ld a,(game_exit_requested)
    or a
    jp nz,main_end
; wait for the next vblank mitigate flicker and for loop timing
    call vdu_vblank
; poll keyboard for escape keypress
    ld a, $08 ; code to send to MOS
    rst.lil $08 ; get IX pointer to System Variables
    ld a, (ix + $05) ; get ASCII code of key pressed
    cp 27 ; check if 27 (ascii code for ESC)   
    jp z, main_end ; if pressed, jump to exit
; escape not pressed so loop
    jp main_loop

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
; print thanks for playing message
    call printInline
    asciz "Thank you for playing\r\n"
    ld hl,purple_nurples_ascii
    call printString
    ret
; end main

; needs to be the final include to leave room for the sprite and tile tables
    include "tables.inc"
