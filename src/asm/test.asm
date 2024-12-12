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
    ; include "fonts.inc"
    ; include "images.inc"
    include "timer.inc"
    include "vdu.inc"
    ; include "vdu_fonts.inc"
    ; include "vdu_plot.inc"
    ; include "vdu_sprites.inc"

; ; APPLICATION INCLUDES
;     include "collisions.inc"
;     include "enemies.inc"
;     include "enemy_fireball.inc"
;     include "enemy_seeker.inc"
;     include "fonts_list.inc"
;     include "images_tiles_dg.inc"
;     ; include "images_tiles_xevious.inc"
;     include "images_sprites.inc"
;     include "images_ui.inc"
;     include "laser.inc"
;     include "levels.inc"
;     include "levels_tileset_0.inc"
;     ; include "levels_xevious.inc"
;     include "player.inc"
;     include "sprites.inc"
;     include "state.inc"
;     include "targeting.inc"
    
;     include "tiles.inc"
;     include "tile_pad_small.inc"
;     include "tile_turret_fireball.inc"

    include "test.inc"

    align 256

; --- MAIN PROGRAM FILE ---

init:

    ret

main:
    call printNewLine
    call orientation_to_player
    call targeting_computer


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
