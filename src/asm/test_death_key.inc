; Test builds only. D = bit 2 of MOS keymap byte 6, independent of input device.
death_review_gate: db INPUT_GATE_WAIT_RELEASE

; Same outward contract as the ordinary pre-dispatch poll: carry means pause.
game_pause_poll:
    call ordinary_pause_poll
    jp c,@paused
    ld hl,(game_state)
    ld de,game_playing
    or a
    sbc hl,de
    jp nz,@ineligible
    ld hl,(player_state)
    ld de,player_alive
    or a
    sbc hl,de
    jp nz,@ineligible
    MOSCALL mos_getkbmap
    xor a
    bit 2,(ix+6)
    jp z,@sample
    inc a
@sample:
    ld iy,death_review_gate
    call input_gate_update
    ret nc
    xor a
    ld (player_shields),a ; match the ordinary lethal-hit HUD
    ld hl,player_death_pending
    ld (hl),1 ; exercise the same end-of-world-frame handoff as a lethal hit
    or a ; dispatch the game; never turn the D event into a pause
    ret
@ineligible:
    ld iy,death_review_gate
    jp input_gate_reset
@paused:
    ld iy,death_review_gate
    call input_gate_reset
    scf
    ret
