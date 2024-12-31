
player_shields: db 64
; player_shields_max: db 64
player_shields_max: db 8 ; DEBUG
player_shields_last: db 0
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
    ld l,(iy+sprite_shield_damage)
    ld a,(player_shields)
    add a,l
    jp p,@have_shields
    xor a ; set to zero
@have_shields:
    ld (player_shields),a
    or a ; check for zero
    jp z,kill_player
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