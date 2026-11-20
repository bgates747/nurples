; Rally's stock VDP Expand Bitmap byte lookup. Upload one master phase per
; title via AGNB and generate its peer once, before either can be drawn.
; This is not the geometric affine transform API.

    include "title_phase_luts.inc"

title_phase_build_splash:
    ld de,splash_title_lut
    ld hl,title_phase_splash_vdu
    jr title_phase_build

title_phase_build_game_over:
    ld de,game_over_lut
    ld hl,title_phase_game_over_vdu

; DE = 256-byte map; HL = the ordered expand/select/register VDU packet.
; The commands are synchronous within the VDP stream, so after registration
; the scratch lookup buffer can go away; the bitmap owns its separate data.
title_phase_build:
    push hl
    push de
    ld hl,TITLE_PHASE_MAP_BUFFER
    call vdu_clear_buffer
    pop de
    ld hl,TITLE_PHASE_MAP_BUFFER
    ld bc,256
    call vdu_write_block_to_buffer
    pop hl
    ld bc,title_phase_splash_vdu_end-title_phase_splash_vdu
    rst.lil 18h
    ld hl,TITLE_PHASE_MAP_BUFFER
    jp vdu_clear_buffer
; end title_phase_build

    macro TITLE_PHASE_VDU master, destination, width, height
    db 23,0,$A0
    dw destination
    db 72,$10 ; eight-bit lookup using a separate table buffer
    dw master,TITLE_PHASE_MAP_BUFFER
    db 23,27,$20
    dw destination
    db 23,27,$21
    dw width,height
    db 1 ; RGBA2222
    endmacro

title_phase_splash_vdu:
    TITLE_PHASE_VDU BUF_SPLASH_TITLE_0, BUF_SPLASH_TITLE_1, SPLASH_TITLE_WIDTH, SPLASH_TITLE_HEIGHT
title_phase_splash_vdu_end:
title_phase_game_over_vdu:
    TITLE_PHASE_VDU BUF_GAME_OVER_0, BUF_GAME_OVER_1, GAME_OVER_WIDTH, GAME_OVER_HEIGHT
