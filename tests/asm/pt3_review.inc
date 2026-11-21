; MUS-001 private fixture. Called in the foreground; no timer or ISR.
mus_active: db 0
mus_paused: db 0
mus_laser: db 0
mus_blast: db 0
mus_ticks: dl 0

mus_process_init:
    xor a
    ld (mus_active),a
    ld (mus_paused),a
    ld (mus_laser),a
    ld (mus_blast),a
    ld hl,vdu_buf
    ld (vdu_ptr),hl
    ret

; Preserve the game's pause-poll result (especially carry). Only send
; silence once on entry; do not call ay_update while paused: it advances
; the library's software envelope even without a PT3 decoder tick.
mus_pause_edge:
    push af
    ld a,(mus_active)
    or a
    jr z,@done
    ld a,(game_pause_active)
    ld hl,mus_paused
    cp (hl)
    jr z,@done
    ld (hl),a
    or a
    jr z,@restore
    call ay_silence
    jr @done
@restore:
    ld hl,ay_nw_vol
    ld de,ay_sh_vol
    ld c,0
@volume:
    ld b,2
    call ay_cmd
    ld a,(hl)
    ld (de),a
    call vdu_put
    inc hl
    inc de
    inc c
    ld a,c
    cp AY_VOICES
    jr nz,@volume
    call vdu_flush
@done:
    pop af
    ret

mus_frame:
    ld hl,(game_state)
    ld de,game_playing
    or a
    sbc hl,de
    jr z,@playing
    ld hl,(game_state)
    ld de,game_level_transition
    or a
    sbc hl,de
    jp nz,mus_stop
@playing:
    ld a,(mus_active)
    or a
    jr nz,@tick
    ld hl,(player_state)
    ld de,player_grace
    or a
    sbc hl,de
    ret nz
    call ay_init
    ; Half-level AY amplitude curve for music only. Keep one chip and do
    ; not use ay_volume (VDP master volume, which would also reduce SFX).
    ld hl,ay_amptab2
    ld (ay_amps),hl
    ld hl,mus_song
    call pt3_init
    ld hl,mus_fx_setup
    ld bc,mus_fx_setup_end-mus_fx_setup
    rst.lil 18h
    ld hl,0
    ld (mus_ticks),hl
    ld a,1
    ld (mus_active),a
@tick:
    call pt3_play
    call ay_update
    ld hl,(mus_ticks)
    inc hl
    ld (mus_ticks),hl
    ld a,(mus_laser)
    or a
    jr z,@blast
    xor a
    ld (mus_laser),a
    ld hl,mus_fx_laser
    ld bc,mus_fx_laser_end-mus_fx_laser
    rst.lil 18h
@blast:
    ld a,(mus_blast)
    or a
    ret z
    xor a
    ld (mus_blast),a
    ld hl,mus_fx_blast
    ld bc,mus_fx_blast_end-mus_fx_blast
    rst.lil 18h
    ret

mus_stop:
    ld a,(mus_active)
    or a
    ret z
    call ay_shutdown
    ld hl,mus_fx_stop
    ld bc,mus_fx_stop_end-mus_fx_stop
    rst.lil 18h
    jp mus_process_init

    include "libsupport.asm"
    include "pt3play.asm"
    include "pt3tab.inc"
    include "ayvdp.asm"
    include "mus_fx.inc"
mus_song:
    incbin "terminator.pt3"
mus_song_end:
