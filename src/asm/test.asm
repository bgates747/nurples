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
    include "tile_pad_small.inc"
    include "tile_turret_fireball.inc"
    include "sprites.inc"

    align 256

; --- MAIN PROGRAM FILE ---
hello_world: asciz "Welcome to Purple Nurples!"
loading_time: asciz "Loading time:"
loading_complete: asciz "Press any key to continue."

init:
    ret

main:
    call printNewLine

    ld iy,tmr_test
    ld hl,120 ; 1 second
    call tmr_set
    ld hl,0 ; counter
    call vdu_vblank
@loop:
    inc hl ; increment counter
    push hl ; save counter

    ld ix,x_id
    call rand_8
    ld (ix+sprite_x+1),a
    call rand_8
    ld (ix+sprite_y+1),hl

    ld iy,y_id
    call rand_8
    ld (iy+sprite_x),a
    call rand_8
    ld (iy+sprite_y),a

    call check_collision_box

    ld iy,tmr_test
    call tmr_get
    pop hl ; restore counter
    jp p,@loop

    call printDec
    call printNewLine

main_end:
    call vdu_cursor_on
    ret

DEBUG_PRINT:
    PUSH_ALL
    call printNewLine
    call printNewLine
    POP_ALL
    PUSH_ALL
    call dumpFlags
    POP_ALL
    PUSH_ALL
    call dumpRegistersHex
    ; call waitKeypress
    POP_ALL
    ret

DEBUG_PRINT_TILE_TABLE:
    PUSH_ALL
    call printNewLine
    ld ix,tile_stack
    ld ix,(ix)
    call dump_tile_record
    call printNewLine
    POP_ALL
    ret
; end DEBUG_PRINT_TILE_TABLE

DEBUG_PRINT_TABLE:
    PUSH_ALL
    call printNewLine
    call dump_sprite_record
    call printNewLine
    call printNewLine

    push iy
    pop ix
    call dump_sprite_record
    call printNewLine
    call printNewLine
    POP_ALL
    RET

DEBUG_WAITKEYPRESS:
    PUSH_ALL
    call waitKeypress
    POP_ALL
    RET

DEBUG_PRINT_FIELDS:
    ; PUSH_ALL
    ld bc,0
    ld c,a
    ld ix,table_base
    add ix,bc
    ld b,table_num_records
@@:
    push ix
    pop hl
    push bc ; save loop counter
    ld a,1 ; print one byte
    call dumpMemoryHex
    lea ix,ix+table_record_size
    pop bc ; restore loop counter
    djnz @b
    ; POP_ALL
    ret

DEBUG_PRINT_TILE_STACK:
    PUSH_ALL
    call printNewLine
    call printNewLine
    ld hl,(tile_stack_pointer)
    call printHexUHL
    call printNewLine
    ld a,(num_active_tiles)
    call printHexA
    call printNewLine
    ld ix,tile_stack
    ld b,8
@loop:
    push bc
    ld hl,(ix)
    call printHexUHL
    call printNewLine
    lea ix,ix+3
    pop bc
    djnz @loop
    POP_ALL
    ret

DEBUG_DUMP_PLAYER_RECORD:
    PUSH_ALL
    call printNewLine
    CALL dump_player_record
    call printNewLine
    POP_ALL
    RET

x_id:                db 0 ; 1 bytes unique spriteId, zero-based
x_x:                 dl                 0 ; 3 bytes 16.8 fractional x position in pixels
x_y:                 dl                 0 ; 3 bytes 16.8 fractional y position in pixels
x_xvel:              dl                 0 ; 3 bytes x-component velocity, 16.8 fixed, pixels
x_yvel:              dl                 0 ; 3 bytes y-component velocity, 16.8 fixed, pixels
x_vel:               dl             0 ; 3 bytes velocity px/frame (16.8 fixed)
x_heading:           dl                 0 ; 3 bytes sprite movement direction deg256 16.8 fixed
x_orientation:       dl                 0 ; 3 bytes orientation bits
x_type:              db                 0 ; 1 bytes not currently used
x_base_bufferId:     dl       0 ; 3 bytes bitmap bufferId
x_move_program:      dl                 0 ; 3 bytes not currently used
x_collisions:        db                 0 ; 1 bytes see collisions.inc constants for bit definitions
x_dim_x:             db                16 ; 1 bytes sprite width in pixels
x_dim_y:             db                16 ; 1 bytes sprite height in pixels
x_num_orientations:  db                 0 ; 1 bytes number of orientations for this sprite
x_num_animations:    db                 0 ; 1 bytes number of animations for this sprite
x_animation:         db                 0 ; 1 bytes current animation index, zero-based
x_animation_timer:   db                 0 ; 1 bytes when hits zero, draw next animation
x_move_timer:        db                 0 ; 1 bytes when zero, go to next move program, or step
x_move_step:         db                 0 ; 1 bytes stage in a move program sequence, varies
x_points:            db                 0 ; 1 bytes points awarded for killing this sprite type
x_shield_damage:     db                 0 ; 1 bytes shield points deducted for collision




y_id:                db 1 ; 1 bytes unique spriteId, zero-based
y_x:                 dl                   0 ; 3 bytes 16.8 fractional x position in pixels
y_y:                 dl                   0 ; 3 bytes 16.8 fractional y position in pixels
y_xvel:              dl                   0 ; 3 bytes x-component velocity, 16.8 fixed, pixels
y_yvel:              dl              0 ; 3 bytes y-component velocity, 16.8 fixed, pixels
y_vel:               dl              0 ; 3 bytes velocity px/frame (16.8 fixed)
y_heading:           dl                   0 ; 3 bytes sprite movement direction deg256 16.8 fixed
y_orientation:       dl                   0 ; 3 bytes orientation bits
y_type:              db                   0 ; 1 bytes not currently used
y_base_bufferId:     dl         0 ; 3 bytes bitmap bufferId
y_move_program:      dl                   0 ; 3 bytes not currently used
y_collisions:        db                   0 ; 1 bytes see collisions.inc constants for bit definitions
y_dim_x:             db                  16 ; 1 bytes sprite width in pixels
y_dim_y:             db                  16 ; 1 bytes sprite height in pixels
y_num_orientations:  db                   0 ; 1 bytes number of orientations for this sprite
y_num_animations:    db                   0 ; 1 bytes number of animations for this sprite
y_animation:         db                   0 ; 1 bytes current animation index, zero-based
y_animation_timer:   db                   0 ; 1 bytes when hits zero, draw next animation
y_move_timer:        db                   0 ; 1 bytes when zero, go to next move program, or step
y_move_step:         db                   0 ; 1 bytes stage in a move program sequence, varies
y_points:            db                   0 ; 1 bytes points awarded for killing this sprite type
y_shield_damage:     db                   0 ; 1 bytes shield points deducted for collision

    include "tables.inc"