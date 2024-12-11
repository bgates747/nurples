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
    include "fonts_list.inc"
    include "images_tiles_dg.inc"
    ; include "images_tiles_xevious.inc"
    include "images_sprites.inc"
    include "images_ui.inc"
    include "laser.inc"
    include "levels.inc"
    include "levels_tileset_0.inc"
    ; include "levels_xevious.inc"
    include "player.inc"
    include "sprites.inc"
    include "state.inc"
    include "targeting.inc"
    
    include "tiles.inc"
    include "tile_pad_small.inc"
    include "tile_turret_fireball.inc"

    align 256

; --- MAIN PROGRAM FILE ---

init:

    ret

main:
    ld b,32 ; loop counter
@loop:
    push bc
    ld h,b
    ld l,8
    mlt hl
    ld h,l
    ld l,0
    call print_s168_hl
    ld de,3*256
    call polar_to_cartesian
    push bc
    push de
    call print_s168_bc
    call print_s168_de
    pop de
    pop bc

    call cartesian_to_polar_sm
    call print_s168_hl
    call print_s168_de
    call printNewLine
    pop bc
    djnz @loop
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
    call vdu_home_cursor
    ; LIST_FIELD sprite_move_program,3 ; DEBUG
    ; LIST_FIELD sprite_type,1 ; DEBUG

    ld ix,table_base
    call dump_sprite_record
    call printNewLine
    call printNewLine

    lea ix,ix+table_bytes_per_record
    call dump_sprite_record
    call printNewLine

    ; call waitKeypress
    POP_ALL
    RET

DEBUG_WAITKEYPRESS:
    PUSH_ALL
    call waitKeypress
    POP_ALL
    RET