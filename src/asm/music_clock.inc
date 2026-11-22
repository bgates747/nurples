; One invocation owns PRT1; changing track, pausing, and starting a new
; game do not touch its vector. IRQs only publish deadlines. Foreground
; issues at most one already prepared packet per service, then decodes one.

; Complete VDU boundaries during rendering and the VBlank wait. Preserve
; the caller including the MOS IX pointer, VBlank countdown and flags.
lp_poll:
    push af
    push bc
    push de
    push hl
    push ix
    push iy
    call lp_pump
    pop iy
    pop ix
    pop hl
    pop de
    pop bc
    pop af
    ret

; Track changes discard the departing packet and any silent deadlines.
; AY has already silenced/reset its voices. Prime but do NOT submit tick 1.
lp_prime_track:
    ld hl,lp_audible
    ld b,AY_VOICES
    xor a
@clear:
    ld (hl),a
    inc hl
    djnz @clear
    call lp_prepare
lp_rebase:
    ld hl,(lp_irqs)
    ld (lp_seen),hl
    ret

; ay_silence zeroed the VDP volumes and AY volume shadows. Restore the
; volume of the last *heard* tick, but put the shadows back at the state
; of the pending packet (AY generates deltas against that future state).
lp_resume:
    ld hl,(lp_irqs)
    ld de,(lp_pause_start)
    or a
    sbc hl,de
    ld de,(lp_paused_irqs)
    add hl,de
    ld (lp_paused_irqs),hl
    ld hl,lp_audible
    ld c,0
@voice:
    ld b,2
    call ay_cmd
    ld a,(hl)
    call vdu_put
    inc hl
    inc c
    ld a,c
    cp AY_VOICES
    jr nz,@voice
    call vdu_flush
    ld hl,ay_nw_vol
    ld de,ay_sh_vol
    ld bc,AY_VOICES
    ldir
    jp lp_rebase

lp_note_tick:
    ld hl,ay_nw_vol
    ld de,lp_audible
    ld bc,AY_VOICES
    ldir
    ld a,(lp_status)
    bit 0,a
    ld hl,lp_phase_ticks
    jr z,@increment
    ld hl,(mus_ticks)
    inc hl
    ld (mus_ticks),hl
    ld hl,(game_state)
    ld de,game_start_config
    or a
    sbc hl,de
    ld hl,lp_phase_ticks+3
    jr z,@increment
    ld hl,(game_state)
    ld de,game_attract
    or a
    sbc hl,de
    ld hl,lp_phase_ticks+6
    jr z,@increment
    ld hl,(game_state)
    ld de,game_over
    or a
    sbc hl,de
    ld hl,lp_phase_ticks+15
    jr z,@increment
    ld a,(cur_level)
    or a
    ld hl,lp_phase_ticks+9
    jr z,@increment
    ld hl,lp_phase_ticks+12
@increment:
    push hl
    ld hl,(hl)
    inc hl
    ex de,hl
    pop hl
    ld (hl),de
    ret
