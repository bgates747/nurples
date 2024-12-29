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
    include "images.inc"
    include "timer.inc"
    include "vdu.inc"
    include "vdu_fonts.inc"
    include "vdu_plot.inc"
    include "vdu_sprites.inc"

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

    align 256

; --- MAIN PROGRAM FILE ---
main:
; start a new game
    ld hl,game_init
    ld (game_state),hl
main_loop:
; update the global timestamp
    call timestamp_tick
; do gamestate logic
    call do_game
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
    call vdu_cursor_on
    ret
; end main

    include "tables.inc"