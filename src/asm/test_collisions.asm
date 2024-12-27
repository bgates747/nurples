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
    ; include "images_tiles_xevious.inc"
    include "images_sprites.inc"
    include "images_ui.inc"
    include "levels.inc"
    include "levels_tileset_0.inc"
    ; include "levels_xevious.inc"
    include "player.inc"
    include "player_cockpit.inc"
    include "player_laser.inc"
    include "state.inc"
    include "targeting.inc"
    include "tile_table.inc"
    include "tiles.inc"
    include "tiles_active.inc"
    include "tile_crater.inc"
    include "tile_electrode.inc"
    include "tile_lightning.inc"
    include "tile_pad_small.inc"
    include "tile_turret_fireball.inc"
    include "sprites.inc"
    include "debug.inc"

    align 256

; --- MAIN PROGRAM FILE ---
hello_world: asciz "Welcome to Purple Nurples!"
loading_time: asciz "Loading time:"
loading_complete: asciz "Press any key to continue."

init:
    xor a
    call vdu_set_scaling
    call vdu_cursor_off
    call vdu_clg

    ld ix,table_base
    ld hl,0
    ld (ix+sprite_x),hl
    ld (ix+sprite_y),hl
    ld a,16
    ld (ix+sprite_dim_x),a
    ld (ix+sprite_dim_y),a

    ld iy,table_base+table_record_size
    ld hl,32*256
    ld (iy+sprite_x),hl
    ld (iy+sprite_y),hl
    ld a,16
    ld (iy+sprite_dim_x),a
    ld (iy+sprite_dim_y),a

    call collision_draw_hitboxes

; done with init
    ret

main:

main_loop:
; wait for the next vblank mitigate flicker and for loop timing
    call vdu_vblank

    call waitKeypress

    ld ix,table_base
    ld iy,table_base+table_record_size
    ld bc,(ix+sprite_x)
    ld de,(ix+sprite_y)

    cp 'a'
    jp z,@left
    cp 'd'
    jp z,@right
    cp 'w'
    jp z,@up
    cp 's'
    jp z,@down
    cp '\e' ; esc  
    jp z,main_end
    jp main_loop

@left:
    ; call printInline
    ; asciz "left\r\n"
    ld hl,-256
    add hl,bc
    ld (ix+sprite_x),hl
    jp @draw
@right:
    ; call printInline
    ; asciz "right\r\n"
    ld hl,256
    add hl,bc
    ld (ix+sprite_x),hl
    jp @draw
@up:
    ld hl,-256
    add hl,de
    ld (ix+sprite_y),hl
    ; call printInline
    ; asciz "up\r\n"
    jp @draw
@down:
    ld hl,256
    add hl,de
    ld (ix+sprite_y),hl
    ; call printInline
    ; asciz "down\r\n"

@draw:
    call vdu_cls
    call collision_draw_hitboxes
    call check_collision
    call dumpFlags
    jp main_loop

main_end:
    call vdu_cursor_on
    ret

    include "tables.inc"