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
    ld hl,BUF_SHIELD
    call vdu_buff_select
    ld bc,492
    ld de,368
    call vdu_plot_bmp
    call init_bar_shields
    ret
; end draw_player_cockpit

init_bar_shields:
    ld ix,bar_shields
    ld hl,(player_max_shields)
    ld (ix+bar_val),hl
    call bar_update_v
    ret
; end init_bar_shields

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

bar_val: equ 0 ; current value
bar_x0: equ panel_x0
bar_y0: equ panel_y0
bar_x1: equ panel_x1
bar_y1: equ panel_y1
bar_y: equ 15 ; y-plot coordinate

; update a vertical bar display
; inputs: ix points to bar structure
;         bar_val set by caller
; prerequisite: bar_init called before first use
bar_update_v:
; write x-plot position
    ld hl,(ix+bar_x0)
    ld (@x2),hl
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
    ld (@y2),hl
; select the color based on the value thresholds
    ld de,(ix+bar_val)
    ld hl,-16
    add hl,de
    jp nc,@red
    ld hl,-32
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
; set gfx viewport
    ld hl,(ix+bar_x0)
    ld (@x0),hl
    ld hl,(ix+bar_y1)
    ld (@y1),hl
    ld hl,(ix+bar_x1)
	ld (@x1),hl
    ld hl,(ix+bar_y0)
	ld (@y0),hl
; restore bytes that got stomped
    ld a,25 ; plot command
    ld (@cmd1),a
; send the vdu command string
	ld hl,@cmd0
	ld bc,@end-@cmd0
	rst.lil $18
	ret
; set graphics viewport command
@cmd0:   db 24 
@x0: 	dw 0x0000 
@y1: 	dw 0x0000 
@x1: 	dw 0x0000 
@y0: 	dw 0x0000 
; plot bitmap command
@cmd1:   db 25 ; stomped on by @y0 load
@mode:  db plot_bmp+dr_abs_fg ; 0xED stomped on by @x2 load
@x2: 	dw 0x0000
@y2: 	dw 0x0000
@end: db 0x00 ; padding
; end bar_update_v

bar_shields:
    dl 0 ; value
    dl 496 ; x0
    dl 304 ; y0
    dl 496+8 ; x1
    dl 304+64 ; y1
    dl 304-64 ; y-plot coordinate