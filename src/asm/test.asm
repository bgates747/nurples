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
    ; include "laser.inc"
    include "levels.inc"
    include "levels_tileset_0.inc"
    ; include "levels_xevious.inc"
    include "player.inc"
    include "player_laser.inc"
    include "player_weapons.inc"
    include "state.inc"
    include "targeting.inc"
    include "tile_table.inc"
    include "tiles.inc"
    include "tiles_active.inc"
    include "tile_pad_small.inc"
    include "tile_turret_fireball.inc"
    include "sprites.inc"
    include "sprites_new.inc"

    align 256

; --- MAIN PROGRAM FILE ---
hello_world: asciz "Welcome to Purple Nurples!"
loading_time: asciz "Loading time:"
loading_complete: asciz "Press any key to continue."

init:
    ret

main:
    call printNewLine
; test umul24ss
    call vdu_vblank ; synchronize timer
    ld iy,tmr_test
    ld hl,120 ; 1 second
    call tmr_set
    ld hl,0 ; counter
    push hl ; save counter
@umul24ss_loop:
    ld hl,256
    ld de,256
    call umul24ss
    pop hl ; restore counter
    inc hl
    push hl ; save counter
    call tmr_get
    jp p,@umul24ss_loop
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
@umul24_loop:
    ld hl,256
    ld de,256
    call umul24
    pop hl ; restore counter
    inc hl
    push hl ; save counter
    call tmr_get
    jp p,@umul24_loop
    pop hl ; restore counter
    call printDec
    call printNewLine


main_end:
    ret

;------------------------------------------------------------------------
; DEPRECATED BY umul24 in fixed168.inc
;------------------------------------------------------------------------
; umul24:	HL = HL*DE (unsigned)
; Preserves AF, BC, DE
; Uses a fast multiply routine.
;------------------------------------------------------------------------
umul24ss:
	push	DE 
	push	BC
	push	AF	
	push	HL
	pop		BC
    ld	 	a, 24 ; No. of bits to process 
    ld	 	hl, 0 ; Result
umul24_lpss:
	add	hl,hl
	ex	de,hl
	add	hl,hl
	ex	de,hl
	jr	nc,umul24_ncss
	add	hl,bc
umul24_ncss: 
	dec	a
	jr	nz,umul24_lpss
	pop	af
	pop	bc
	pop	de
	ret


    include "tables.inc"