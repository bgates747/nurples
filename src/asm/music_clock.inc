; One invocation owns PRT1; changing track, pausing, and starting a new
; game do not touch its vector. IRQs only publish deadlines. Foreground
; issues at most one already prepared packet per service, then decodes one.

; Complete VDU boundaries during rendering and the VBlank wait. Preserve
; the caller including the MOS IX pointer, VBlank countdown and flags.
lp_poll:
    push af
    ld a,(lp_music_off)
    or a
    jr nz,@return
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
@return:
    pop af
    ret

; Track changes discard the departing packet and any silent deadlines.
; AY has already silenced/reset its voices. Prime but do NOT submit tick 1.
lp_prime_track:
    ld a,(lp_music_off)
    ld (lp_track_pending),a
    or a
    ret nz ; defer even decoder initialization until the user enables music
    ld hl,(lp_selected_track)
    call pt3_init
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
lp_restore_audible:
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

lp_music_off: db 0 ; per MOS run, not reset by mus_stop/new game
lp_m_gate: db 0
lp_track_pending: db 0
lp_selected_track: dl 0

; Main loop only, never in an ISR.
lp_sample_m:
    ld a,(lp_started)
    or a
    ret z
    ld a,(lp_owned)
    or a
    ret z
    xor a
    bit music_key_bit,(ix+music_key_byte)
    jr z,@sample
    inc a
@sample:
    ld iy,lp_m_gate
    call input_gate_update
    ret nc
    ld a,(lp_music_off)
    xor 1
    ld (lp_music_off),a
    jr z,@on
    xor a
    out0 (083h),a ; stop counting AND disable PRT1 IRQ; retain vector ownership
    in0 a,(083h) ; acknowledge any latched expiry
    jp ay_silence
@on:
    call lp_rebase
    ld a,(game_pause_active)
    ld (mus_paused),a
    ld hl,(lp_irqs)
    ld (lp_pause_start),hl
    ld a,(mus_active)
    or a
    jr z,@restart
    ld a,(lp_track_pending)
    or a
    jr z,@same
    call lp_prime_track
    jr @restart
@same:
    ld a,(mus_paused)
    ld hl,mus_in_warp
    or (hl)
    call z,lp_restore_audible
@restart:
    ld a,05fh ; reload a fresh 20 ms interval, no catch-up
    out0 (083h),a
    ret

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
