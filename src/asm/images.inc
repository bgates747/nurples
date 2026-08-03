; Purple Nurples AGNB startup integration
; =======================================
;
; UI and gameplay bitmaps retain their established explicit VDP buffer IDs,
; now carried in two canonical AGNB 0.1 containers. The loose VDU font loader
; remains separate.

ui_agnb_filename:   db "ui.agnb",0
game_agnb_filename: db "game.agnb",0

; The 160x106 logo travels from just inside the lower edge to an 8-pixel top
; margin. Keep these expressions unambiguous because ez80asm evaluates complex
; arithmetic left-to-right.
logo_y_start:    equ 224*256
logo_y_end:      equ 8*256
logo_y_distance: equ -216*256

logo_xpos: dl 0
logo_ypos: dl 0
logo_yvel: dl 0

game_load_index:             dl 0
game_load_remaining:         dl 0
game_load_callback_overflow: db 0


; Print one completion breadcrumb at a safe record boundary. The canonical
; loader invokes this only after the bitmap has been fully created.
img_load_agnb_breadcrumb:
    ld a,'.'
    rst.lil 10h
    xor a
    ret


; Initialize exact 16.8 accumulated displacement for all game.agnb records.
img_load_init:
    ld hl,0
    ld (logo_xpos),hl
    ld (game_load_index),hl
    ld a,l
    ld (game_load_callback_overflow),a

    ld hl,logo_y_start
    ld (logo_ypos),hl

    ld hl,game_num_images
    ld (game_load_remaining),hl

    ld hl,logo_y_distance
    ld de,game_num_images*256
    call sdiv168 ; DE = signed 16.8 distance per completed bitmap
    ld (logo_yvel),de
    ret


; Draw the splash and advance its marching logo after one complete game image.
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

; Clear before repainting because screen mode 8 is single-buffered.
    call vdu_cls
    ld hl,BUF_SPLASH_BG
    call vdu_buff_select
    ld bc,0
    ld de,0
    call vdu_plot_bmp

    call move_logo

; Show deterministic record progress without retaining 350 pathname strings.
    call printInline
    asciz "Loading images: "
    ld hl,(game_load_index)
    call printDec
    call printInline
    asciz "/"
    ld hl,game_num_images
    call printDec

    xor a
    ret


move_logo:
; Snap the last callback to the exact endpoint so fixed-point division
; remainder cannot accumulate into a visible shortfall.
    ld hl,(game_load_remaining)
    ld de,0
    or a
    sbc hl,de
    jr nz,@accumulate
    ld hl,logo_y_end
    jr @position_ready

@accumulate:
    ld hl,(logo_ypos)
    ld de,(logo_yvel)
    add hl,de

@position_ready:
    ld (logo_ypos),hl
    ld hl,BUF_SPLASH_LOGO
    call vdu_buff_select
    ld bc,(logo_xpos)
    ld de,(logo_ypos)
    call vdu_plot_bmp168
    ret


; Load the bootstrap graphics while reporting one dot per finalized bitmap.
; outputs: canonical AGNB status in A/Z
load_ui_images:
    ld de,ui_agnb_filename
    ld hl,img_load_agnb_breadcrumb
    jp agnb_load_images_with_callback


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
    xor a
    ret

@count_error:
    ld a,agnb_error_metadata
    ld (agnb_last_error),a
    or a
    ret
