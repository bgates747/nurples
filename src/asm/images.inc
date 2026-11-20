; Purple Nurples AGNB startup integration
; =======================================
;
; UI and gameplay bitmaps retain their established explicit VDP buffer IDs,
; now carried in two canonical AGNB 0.1 containers. The loose VDU font loader
; remains separate.

ui_agnb_filename:   db "ui.agnb",0
game_agnb_filename: db "game.agnb",0

; Author's three-line mockup anchor in physical Mode 20 pixels.
SPLASH_TITLE_X: equ 28
SPLASH_TITLE_Y: equ 50
SPLASH_TITLE_PERIOD: equ 32 ; MOS clock centiseconds; checked at record boundaries

game_load_index:             dl 0
game_load_remaining:         dl 0
game_load_callback_overflow: db 0
splash_title_frame:          db 0
splash_title_tick:           dl 0


; Print one completion breadcrumb at a safe record boundary. The canonical
; loader invokes this only after the bitmap has been fully created.
img_load_agnb_breadcrumb:
    ld a,'.'
    rst.lil 10h
    xor a
    ret


; Paint the single-buffered background once, after the UI container is ready.
img_load_init:
    ld hl,0
    ld (game_load_index),hl
    ld a,l
    ld (game_load_callback_overflow),a
    ld (splash_title_frame),a

    ld hl,game_num_images
    ld (game_load_remaining),hl

    ld hl,BUF_SPLASH_BG
    call vdu_buff_select
    ld bc,0
    ld de,0
    call vdu_plot_bmp
    call splash_draw_title
    MOSCALL mos_sysvars
    ld hl,(ix+sysvar_time)
    ld (splash_title_tick),hl
    jp splash_print_progress


; Update the progress after one complete game image. Never clear the screen.
; The callback result is ignored by AGNB, so an impossible extra callback is
; latched for validation after the loader returns.
img_load_agnb_progress:
    ld hl,(game_load_remaining)
    ld de,0
    or a
    sbc hl,de
    jr nz,@record_expected
    ld a,1
    ld (game_load_callback_overflow),a
    xor a
    ret

@record_expected:
    dec hl
    ld (game_load_remaining),hl
    ld de,(game_load_index)
    inc de
    ld (game_load_index),de

    call splash_update_title
    call splash_print_progress
    xor a
    ret

; printDec uses a fixed field width, so writing at the same cursor position
; replaces the entire status line without clearing any background pixels.
splash_print_progress:
    ld bc,0
    call vdu_move_cursor
    call printInline
    asciz "Loading images: "
    ld hl,(game_load_index)
    call printDec
    call printInline
    asciz "/"
    ld hl,game_num_images
    jp printDec

; Use the same wrapping 24-bit MOS clock as the game timers. Repaint at most
; once per period; don't wait for the clock or flip in the loader callback.
splash_update_title:
    MOSCALL mos_sysvars
    ld hl,(ix+sysvar_time)
    ld de,(splash_title_tick)
    or a
    sbc hl,de
    ld de,SPLASH_TITLE_PERIOD
    or a
    sbc hl,de
    ret c
    ld hl,(ix+sysvar_time)
    ld (splash_title_tick),hl
    ld a,(splash_title_frame)
    xor 1
    ld (splash_title_frame),a
; Equal binary-alpha footprints allow direct redraw without erasing the old title.
splash_draw_title:
    ld a,(splash_title_frame)
    or a
    ld hl,BUF_SPLASH_TITLE_0
    jr z,@select
    ld hl,BUF_SPLASH_TITLE_1
@select:
    call vdu_buff_select
    ld bc,SPLASH_TITLE_X
    ld de,SPLASH_TITLE_Y
    jp vdu_plot_bmp


; Load the bootstrap graphics while reporting one dot per finalized bitmap.
; outputs: canonical AGNB status in A/Z
load_ui_images:
    ld de,ui_agnb_filename
    ld hl,img_load_agnb_breadcrumb
    call agnb_load_images_with_callback
    ret nz
    call title_phase_build_splash
    xor a
    ret


; Load all backgrounds, sprites, and DG tiles through one persistent handle.
; The postcondition guards both catalog drift and callback overrun.
; outputs: A=0/Z on exact success; AGNB metadata error/NZ otherwise
load_game_images:
    call img_load_init
    ld de,game_agnb_filename
    ld hl,img_load_agnb_progress
    call agnb_load_images_with_callback
    ret nz

    ld hl,(game_load_remaining)
    ld de,0
    or a
    sbc hl,de
    jr nz,@count_error
    ld a,(game_load_callback_overflow)
    or a
    jr nz,@count_error
    call title_phase_build_game_over
    xor a
    ret

@count_error:
    ld a,agnb_error_metadata
    ld (agnb_last_error),a
    or a
    ret

    include "title_phase_vdp.inc"
