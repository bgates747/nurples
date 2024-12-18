panel_bufferId: equ 0
panel_x0: equ 3
panel_y0: equ 6
panel_x1: equ 9
panel_y1: equ 12

ctl_panel_l:
    dl BUF_CTL_PANEL_L ; bufferId
    dl 0 ; x0
    dl 320 ; y0
    dl 0+128 ; x1
    dl 320+64 ; y1

ctl_panel_r:
    dl BUF_CTL_PANEL_R ; bufferId
    dl 384 ; x0
    dl 320 ; y0
    dl 384+128 ; x1
    dl 320+64 ; y1

mfd_bezel_l:
    dl BUF_MFD_BEZEL ; bufferId
    dl 64 ; x0
    dl 326 ; y0
    dl 64+59 ; x1
    dl 326+52 ; y1

mfd_bezel_r:
    dl BUF_MFD_BEZEL ; bufferId
    dl 389 ; x0
    dl 326 ; y0
    dl 389+59 ; x1
    dl 326+52 ; y1

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
    dl 72 ; x0
    dl 333 ; y0
    dl 72+43 ; x1
    dl 333+38 ; y1

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
