
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