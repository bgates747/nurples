; Campaign audio, called in the foreground; no timer or ISR.
    .if mus_campaign
mus_in_warp: db 0
mus_title_active: db 0
mus_blast_count: dl 0
    .endif
mus_active: db 0
mus_paused: db 0
mus_laser: db 0
mus_blast: db 0
mus_ticks: dl 0

mus_process_init:
    .if mus_campaign
    call vox_reset
    .endif
    xor a
    .if mus_campaign
    ld (mus_in_warp),a
    ld (mus_title_active),a
    ld hl,0
    ld (mus_blast_count),hl
    .endif
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
    .if mus_campaign
    ld a,(mus_in_warp)
    or a
    jr nz,@done ; pause/unpause must not restore the departed level's voices
    .endif
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
    .if mus_campaign
    ; Controller choice and attract are one continuous wait; Game Over
    ; starts a new presentation after play. Only an accepted Start or exit
    ; tears down these channels, not arbitrary input or the Y/N choice.
    ld hl,(game_state)
    ld de,game_start_config
    or a
    sbc hl,de
    jp z,mus_presentation
    ld hl,(game_state)
    ld de,game_attract
    or a
    sbc hl,de
    jp z,mus_presentation
    ld hl,(game_state)
    ld de,game_over
    or a
    sbc hl,de
    jp z,mus_presentation
    .endif
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
    .if mus_campaign
    ld a,(mus_active)
    or a
    ret z
    ld a,(mus_in_warp)
    or a
    jp nz,mus_effects
    inc a
    ld (mus_in_warp),a
    call ay_silence ; once, music only; combat effects still run every frame
    jp mus_effects
    .endif
@playing:
    .if mus_campaign
    ld a,(cur_level)
    cp mus_level_count
    jp nc,mus_stop
    ld a,(mus_in_warp)
    or a
    jr z,@not_arrival
    xor a
    ld (mus_in_warp),a
    call ay_reset
    jr @start_track ; next_level already selected the map, even on wraparound
@not_arrival:
    .endif
    ld a,(mus_active)
    or a
    jr nz,mus_tick
    ld hl,(player_state)
    ld de,player_grace
    or a
    sbc hl,de
    ret nz
    .if mus_campaign
    call mus_audio_open
    .else
    call ay_init
    ; Half-level AY amplitude curve for music only. Keep one chip and do
    ; not use ay_volume (VDP master volume, which would also reduce SFX).
    ld hl,ay_amptab2
    ld (ay_amps),hl
    .endif
    .if mus_campaign
@start_track:
    ld a,(cur_level)
    ld de,0
    ld d,a
    ld e,3
    mlt de
    ld hl,mus_tracks
    add hl,de
    ld hl,(hl)
    call pt3_init
    .else
    ld hl,mus_song
    call pt3_init
    ld hl,mus_fx_setup
    ld bc,mus_fx_setup_end-mus_fx_setup
    rst.lil 18h
    .endif
    ld hl,0
    ld (mus_ticks),hl
    ld a,1
    ld (mus_active),a
mus_tick:
    call pt3_play
    call ay_update
    ld hl,(mus_ticks)
    inc hl
    ld (mus_ticks),hl
mus_effects:
    .if mus_campaign
    call vox_frame
    .endif
    ld a,(mus_laser)
    or a
    jr z,@blast
    xor a
    ld (mus_laser),a
    ld hl,mus_fx_laser
    ld bc,mus_fx_laser_end-mus_fx_laser
    rst.lil 18h
@blast:
    .if mus_campaign
    ld hl,(mus_blast_count)
    ld de,0
    or a
    sbc hl,de
    ret z
    dec hl
    ld (mus_blast_count),hl
    ld hl,mus_fx_blast
    ld bc,mus_fx_blast_end-mus_fx_blast
    rst.lil 18h
    jr @blast ; two kills in one frame still issue two complete retriggers
    .else
    ld a,(mus_blast)
    or a
    ret z
    xor a
    ld (mus_blast),a
    ld hl,mus_fx_blast
    ld bc,mus_fx_blast_end-mus_fx_blast
    rst.lil 18h
    ret
    .endif

    .if mus_campaign
mus_queue_blast:
    push hl
    ld hl,(mus_blast_count)
    inc hl
    ld (mus_blast_count),hl
    pop hl
    ret

mus_audio_open:
    call vox_open
    call ay_init
    ld hl,ay_amptab2
    ld (ay_amps),hl
    ld hl,mus_fx_setup
    ld bc,mus_fx_setup_end-mus_fx_setup
    rst.lil 18h
    ret

mus_presentation:
    ld a,(mus_title_active)
    or a
    jp nz,mus_tick
    ld a,(mus_active)
    or a
    jr nz,@replace
    call mus_audio_open
    jr @load
@replace:
    call ay_reset ; silence departing music, not combat effect channels
@load:
    ld hl,(game_state)
    ld de,game_over
    or a
    sbc hl,de
    call z,vox_game_over
    ld hl,mus_loadtune
    call pt3_init
    ld hl,0
    ld (mus_ticks),hl
    xor a
    ld (mus_in_warp),a
    inc a
    ld (mus_active),a
    ld (mus_title_active),a
    jp mus_tick
    .endif

mus_stop:
    ld a,(mus_active)
    or a
    ret z
    .if mus_campaign
    call vox_close
    .endif
    call ay_shutdown
    ld hl,mus_fx_stop
    ld bc,mus_fx_stop_end-mus_fx_stop
    rst.lil 18h
    jp mus_process_init

    include "audio/agonAMP/libsupport.asm"
    include "audio/agonAMP/pt3play.asm"
    include "audio/agonAMP/pt3tab.inc"
    include "audio/agonAMP/ayvdp.asm"
    include "audio/mus_fx.inc"
    .if mus_campaign
    include "announcer.inc"
    .endif
    include "mus_tracks.inc"
