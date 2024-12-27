score_x: equ 50
score_y: equ 33
; prints the player's score
player_score_last: dl 0 ; integer

; inputs: iy set to sprite/tile record being tallied
; outputs: hl,player_score = updated score
; destroys: hl,de
update_score:
    ld de,0 ; clear deu and d
    ld e,(iy+sprite_points)
    ld hl,(player_score)
    add hl,de
    ld (player_score),hl
    ret
; end update_score

; inputs: none
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

player_shields: dl 64 ; integer
; max player shields,binary
; can increase with power-ups (todo)
player_shields_max: dl 64 ; integer
player_shields_last: dl 0 ; integer
bar_shields:
    dl 0 ; value
    dl 496 ; x0
    dl 304 ; y0
    dl 496+7 ; x1
    dl 304+63 ; y1
    dl 304-64 ; y-plot coordinate

init_bar_shields:
    ld ix,bar_shields
    ld hl,(player_shields_max)
    ld (ix+bar_val),hl
    call bar_update_v
    ret
; end init_bar_shields

; inputs: iy set to sprite/tile record being tallied
; outputs: hl,player_shields = updated shields
; destroys: hl,de
update_shields:
    ld de,0 ; clear deu and d
    ld e,(iy+sprite_shield_damage)
    ld hl,(player_shields)
    add hl,de
    ld (player_shields),hl
    ret
; end update_shields

draw_shields:
    ld hl,player_shields_last
    ld a,(player_shields)
    cp (hl)
    ret z ; no change to update
    ld (hl),a
    ld ix,bar_shields
    ld (ix+bar_val),a ; TODO: this is redundant
    call bar_update_v
    ret

bar_laser:
    dl 0 ; value
    dl 8 ; x0
    dl 304 ; y0
    dl 8+7 ; x1
    dl 304+63 ; y1
    dl 304-64 ; y-plot coordinate

init_bar_laser:
    ld a,(charge_laser_timer_max)
    ld (charge_laser_timer),a
    ld ix,bar_laser
    ld hl,(laser_power_max)
    ld (ix+bar_val),hl
    call bar_update_v
    ret
; end init_bar_laser

laser_power: dl 64 ; integer
laser_power_max: dl 64 ; integer
charge_laser_timer: db 0
charge_laser_timer_max: db 6
laser_power_last: dl 0 ; integer
charge_laser:
; check laser timer
    ld hl,charge_laser_timer
    dec (hl)
    ret nz
    ld a,(charge_laser_timer_max)
    ld (hl),a
    ld hl,laser_power
    inc (hl)
    ld a,64
    cp (hl)
    ret nc
    ld (hl),a
    ret
; end charge_laser
draw_laser:
    call charge_laser
    ld hl,laser_power_last
    ld a,(laser_power)
    cp (hl)
    ret z ; no change to update
    ld (hl),a
    ld ix,bar_laser
    ld (ix+bar_val),a ; TODO: this is redundant
    call bar_update_v
    ret


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

; ; DEBUG
;     PUSH_ALL
;     call vdu_home_cursor
;     ld hl,(ix+bar_val)
;     call printDec
;     call printNewLine
;     POP_ALL
;     PUSH_ALL
;     push bc
;     pop hl
;     call printDec
;     call printNewLine
;     POP_ALL
;     PUSH_ALL
;     ex de,hl
;     call printDec
;     call printNewLine
;     POP_ALL
; ; END DEBUG

    call vdu_plot_bmp
    ret
; end bar_update_h

bar_fuel:
    dl 0 ; value
    dl 48 ; x0
    dl 264 ; y0
    dl 48+63 ; x1
    dl 264+7 ; y1
    dl 48-64 ; x-plot coordinate

init_bar_fuel:
    ld a,(discharge_fuel_timer_max)
    ld (discharge_fuel_timer),a
    ld ix,bar_fuel
    ld a,(player_fuel_max)
    ld (ix+bar_val),a
    call bar_update_h
    ret
; end init_bar_fuel

player_fuel: db 64 ; integer
player_fuel_last: db 0 ; integer
player_fuel_max: db 64 ; integer
discharge_fuel_timer: db 0
discharge_fuel_timer_max: db 64 ; 
discharge_fuel:
; check fuel timer
    ld hl,discharge_fuel_timer
    dec (hl)
    ret nz ; not time to discharge fuel so return
; reset fuel discharge timer
    ld a,(discharge_fuel_timer_max)
    ld (hl),a
; discharge fuel
    ld hl,player_fuel
    dec (hl)
    jp z,@no_fuel
    jp m,@no_fuel
    ret ; fuel not zero so return
@no_fuel:
    ld (hl),0 ; TODO: implement out-of-fuel handler
    ret
; end discharge_fuel

draw_fuel:
    call discharge_fuel
    ld hl,player_fuel_last
    ld a,(player_fuel)
    cp (hl)
    ret z ; no change to update
    ld (hl),a
    ld ix,bar_fuel
    ld (ix+bar_val),a ; TODO: this is redundant
    call bar_update_h
    ret
; end draw_fuel