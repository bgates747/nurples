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

ctl_panel_top:
    dl BUF_CTL_PANEL_TOP ; bufferId
    dl 80 ; x0
    dl 0 ; y0
    dl 80+351 ; x1
    dl 0+47 ; y1

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
    ld ix,ctl_panel_top
    call draw_player_ctl_panel
    ld ix,mfd_bezel_l
    call draw_player_ctl_panel
    ld ix,mfd_bg_l
    call draw_player_ctl_panel
    ld ix,mfd_arms_l
    call draw_player_ctl_panel
    ld ix,mfd_bezel_r
    call draw_player_ctl_panel
    ld ix,mfd_bg_r
    call draw_player_ctl_panel

    ld hl,BUF_SHIELD
    call vdu_buff_select
    ld bc,492
    ld de,368
    call vdu_plot_bmp
    ld hl,BUF_POWER
    call vdu_buff_select
    ld bc,4
    ld de,368
    call vdu_plot_bmp
    ld hl,BUF_CREDITS
    call vdu_buff_select
    ld bc,384
    ld de,261
    call vdu_plot_bmp
    ld hl,BUF_JERRY_CAN
    call vdu_buff_select
    ld bc,110
    ld de,258
    call vdu_plot_bmp

    call init_bar_shields
    call init_bar_laser
    call init_bar_fuel

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

mfd_bg_l:
    dl BUF_MFD_BG ; bufferId
    dl 40 ; x0
    dl 296 ; y0
    dl 40+63 ; x1
    dl 296+63 ; y1

mfd_arms_l:
    dl BUF_MFD_ARMS ; bufferId
    dl 54 ; x0
    dl 320 ; y0
    dl 54+35 ; x1
    dl 320+17 ; y1

mfd_bg_r:
    dl BUF_MFD_BG ; bufferId
    dl 408 ; x0
    dl 296 ; y0
    dl 408+63 ; x1
    dl 296+63 ; y1

mfd_activate:
    push ix ; preserve pointer
    ld bc,(ix+panel_x0)
    ld de,(ix+panel_y0)
    call vdu_set_gfx_origin
    ld bc,0
    ld de,0
    ld ix,63
    ld iy,63
    call vdu_set_gfx_viewport
    pop ix ; restore pointer
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

bar_val: equ 0 ; current value
bar_x0: equ panel_x0
bar_y0: equ panel_y0
bar_x1: equ panel_x1
bar_y1: equ panel_y1
bar_y: equ 15 ; y-plot coordinate
bar_x: equ 15 ; x-plot coordinate

bar_yellow: equ 33
bar_red: equ 17

; update a vertical bar display
; inputs: ix points to bar structure
;         bar_val set by caller
; prerequisite: bar_init called before first use
bar_update_v:
; set gfx viewport
    push ix ; preserve pointer
    ld bc,(ix+bar_x0)
    ld de,(ix+bar_y0)
    ld iy,(ix+bar_y1)
    ld ix,(ix+bar_x1)
    call vdu_set_gfx_viewport
    pop ix ; restore pointer
; select the color based on the value thresholds
    ld de,(ix+bar_val)
    ld hl,-bar_red
    add hl,de
    jp nc,@red
    ld hl,-bar_yellow
    add hl,de
    jp nc,@yellow
    ld hl,BUF_BAR_V_2 ; green
    jp @select
@yellow:
    ld hl,BUF_BAR_V_1 ; yellow
    jp @select
@red:
    ld hl,BUF_BAR_V_0 ; red
@select:
    call vdu_buff_select
; get x-plot position
    ld bc,(ix+bar_x0)
; compute y-plot position
    ld de,(ix+bar_val)
    ld hl,64
    xor a ; clear carry
    sbc hl,de
    ld de,(ix+bar_y)
    add hl,de
    ld a,l
    and %11111100 ; round to nearest 4 pixels
    ld l,a
    ex de,hl
    call vdu_plot_bmp
    ret
; end bar_update_v

; update a horizontal bar display
; inputs: ix points to bar structure
;         bar_val set by caller
; prerequisite: bar_init called before first use
bar_update_h:
; set gfx viewport
    push ix ; preserve pointer
    ld bc,(ix+bar_x0)
    ld de,(ix+bar_y0)
    ld iy,(ix+bar_y1)
    ld ix,(ix+bar_x1)
    call vdu_set_gfx_viewport
    pop ix ; restore pointer
; select the color based on the value thresholds
    ld de,(ix+bar_val)
    ld hl,-bar_red
    add hl,de
    jp nc,@red
    ld hl,-bar_yellow
    add hl,de
    jp nc,@yellow
    ld hl,BUF_BAR_H_2 ; green
    jp @select
@yellow:
    ld hl,BUF_BAR_H_1 ; yellow
    jp @select
@red:
    ld hl,BUF_BAR_H_0 ; red
@select:
    call vdu_buff_select
; compute x-plot position
    ld hl,(ix+bar_val)
    ld bc,(ix+bar_x)
    add hl,bc
    ld a,l
    and %11111100 ; round to nearest 4 pixels
    ld l,a
    push hl
    pop bc
; get y-plot position
    ld de,(ix+bar_y0)
    call vdu_plot_bmp
    ret
; end bar_update_h
