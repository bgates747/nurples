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
    include "macros.inc"
    include "functions.inc"
    include "arith24.inc"
    include "maths.inc"
    include "files.inc"
    include "fixed168.inc"
    ; include "fonts.inc"
    ; include "images.inc"
    include "timer.inc"
    include "vdu.inc"
    include "vdu_fonts.inc"
    include "vdu_plot.inc"
    ; include "vdu_sprites.inc"

; ; APPLICATION INCLUDES
;     include "ascii.inc"
;     include "collisions.inc"
;     include "enemies.inc"
;     include "enemy_fireball.inc"
;     include "enemy_seeker.inc"
;     include "explosion.inc"
;     include "fonts_list.inc"
;     include "images_tiles_dg.inc"
;     ; include "images_tiles_xevious.inc"
;     include "images_sprites.inc"
;     include "images_ui.inc"
;     ; include "laser.inc"
;     include "levels.inc"
;     include "levels_tileset_0.inc"
;     ; include "levels_xevious.inc"
;     include "player.inc"
;     include "player_laser.inc"
;     include "player_weapons.inc"
;     include "state.inc"
;     include "targeting.inc"
;     include "tile_table.inc"
;     include "tiles.inc"
;     include "tiles_active.inc"
;     include "tile_pad_small.inc"
;     include "tile_turret_fireball.inc"
;     include "sprites.inc"
;     include "sprites_new.inc"

    align 256

; --- MAIN PROGRAM FILE ---
hello_world: asciz "Welcome to Purple Nurples!"
loading_time: asciz "Loading time:"
loading_complete: asciz "Press any key to continue."

init:
    ret

buffer: 
    ; db 0x0D,0x0E,0x0F
    ; db 0x0A,0x0B,0x0C
    db 0x07,0x08,0x09
    db 0x04,0x05,0x06
    db 0x01,0x02,0x03

main:
    ld ix,buffer
    ld hl,(ix+0)
    push hl
    ld hl,(ix+3)
    push hl
    ld hl,(ix+6)
    push hl

; ; the testing is here
;     pop hl
;     pop hl
;     call printHex24 ; 0x060504
;     call printNewLine
; ; end of testing

; ; the testing is here
;     pop hl
;     inc sp
;     pop hl
;     dec sp
;     call printHex24 ; 0x070605
;     call printNewLine
; ; end of testing

; the testing is here
    pop hl
    dec sp
    pop hl
    inc sp
    call printHex24 ; 0x050403
    call printNewLine
; end of testing

    pop hl

    ; ld a,0xAB
    ; A_TO_HLU
    ; call printHex24
    ; call printNewLine

    jp main_end

; test umul24ss
    call vdu_vblank ; synchronize timer
    ld iy,tmr_test
    ld hl,120 ; 1 second
    call tmr_set
    ld hl,0 ; counter
    push hl ; save counter
@@:
    ld hl,256
    ld de,256
    call umul168
    pop hl ; restore counter
    inc hl
    push hl ; save counter
    call tmr_get
    jp p,@B
    pop hl ; restore counter
    call printDec
    call printNewLine

; test umul24
    call vdu_vblank ; synchronize timer
    ld iy,tmr_test
    ld hl,120 ; 1 second
    call tmr_set
    ld hl,0 ; counter
    push hl ; save counter
@@:
    ld hl,256
    ld de,256
    call udiv168
    pop hl ; restore counter
    inc hl
    push hl ; save counter
    call tmr_get
    jp p,@B
    pop hl ; restore counter
    call printDec
    call printNewLine


main_end:
    ret

    ; include "tables.inc"