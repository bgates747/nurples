score_x: equ 50
score_y: equ 33
; prints the player's score
player_score_last: dl 0
print_score:
    ld hl,(player_score_last)
    ld de,(player_score)
    ld (player_score_last),de
    xor a ; clear carry
    sbc hl,de
    ret z ; no change to update
    ld c,score_x
    ld b,score_y
    call vdu_move_cursor
    ld hl,(player_score)
    call printDec
    ret
; end print_score


; offsets into control panel element definitions
panel_bufferId: equ 0
panel_x0: equ 3
panel_y0: equ 6
panel_x1: equ 9
panel_y1: equ 12

ctl_panel_l:
    dl BUF_CTL_PANEL_L ; bufferId
    dl 0 ; x0
    dl 256 ; y0
    dl 0+128 ; x1
    dl 256+128 ; y1

ctl_panel_r:
    dl BUF_CTL_PANEL_R ; bufferId
    dl 384 ; x0
    dl 256 ; y0
    dl 384+128 ; x1
    dl 256+128 ; y1

mfd_bezel_l:
    dl BUF_MFD_BEZEL ; bufferId
    dl 24 ; x0
    dl 280 ; y0
    dl 24+96 ; x1
    dl 280+96 ; y1

mfd_bezel_r:
    dl BUF_MFD_BEZEL ; bufferId
    dl 392 ; x0
    dl 280 ; y0
    dl 392+96 ; x1
    dl 280+96 ; y1

draw_player_cockpit:
    ld ix,ctl_panel_l
    call draw_player_ctl_panel
    ld ix,ctl_panel_r
    call draw_player_ctl_panel
    ld ix,mfd_bezel_l
    call draw_player_ctl_panel
    ld ix,mfd_bezel_r
    call draw_player_ctl_panel
    ret
; end draw_player_cockpit

draw_player_ctl_panel:
    ld hl,(ix+panel_bufferId)
    call vdu_buff_select
    ld bc,(ix+panel_x0)
    ld de,(ix+panel_y0)
    call vdu_plot_bmp
    ret
; end draw_player_ctl_panel

mfd_display_l:
    dl BUF_MFD_ARMS ; bufferId
    dl 54 ; x0
    dl 320 ; y0
    dl 54+36 ; x1
    dl 320+18 ; y1

mfd_activate:
    ld bc,(ix+panel_x0)
    ld de,(ix+panel_y0)
    call vdu_set_gfx_origin
    ret
; end mfd_activate

mfd_init:
    ld hl,(ix+panel_bufferId)
    call vdu_buff_select
    ld bc,0
    ld de,0
    call vdu_plot_bmp
    ret
; end mfd_init
