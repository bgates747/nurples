; AgonArcade Nurples soundboard, default two-layer rocket and double klaxon.
; Private lanes: rocket hiss 12, rocket sweep 13, warning 14. Speech stays
; on 11. Only the foreground submits VDU commands; the timer ISR never does.
cues_launches: db 0 ; at most four fireball slots can launch in one actor pass
cues_pre_alert: db 0
cues_alert_start: dl 0
; Preserve the original 160 ms pitch sweep. Stock VDP plays the whole train
; itself; no per-pulse polling, no interruption of the 50 Hz soundtrack.
cues_klaxon_gate_ms: equ 8*160
cues_klaxon_tail_ms: equ 4*44
; MOS time advances twice per VBlank (120 counts/s in the game's 60 Hz
; mode). Allow one VBlank of phase uncertainty. No dependency on music IRQs.
cues_alert_ticks: equ [[cues_klaxon_gate_ms+cues_klaxon_tail_ms]*120+999]/1000+2

cues_queue_launch:
    push af
    push hl
    ld hl,cues_launches
    inc (hl)
    pop hl
    pop af
    ret

cues_play_launches:
    ld hl,cues_launches
    ld a,(hl)
    or a
    ret z
    dec (hl)
    ld hl,cues_rocket
    ld bc,cues_rocket_end-cues_rocket
    rst.lil 18h
    jr cues_play_launches

; vox_contact retains the most urgent unsaid band in vox_warning. An upgrade
; during the whoop changes those words, not the start time. Repeated polls,
; same-band hits and quiet frames produce no extra VDU transactions.
cues_service_warning:
    ld a,(vox_warning)
    or a
    ret z
    ld a,(cues_pre_alert)
    or a
    jr nz,@waiting
    ld hl,cues_interrupt_speech
    ld bc,cues_klaxon_end-cues_interrupt_speech
    rst.lil 18h
    MOSCALL mos_sysvars
    ld hl,(ix+sysvar_time)
    ld (cues_alert_start),hl
    ld a,1
    ld (cues_pre_alert),a
    ret
@waiting:
    MOSCALL mos_sysvars
    ld hl,(ix+sysvar_time)
    ld de,(cues_alert_start)
    or a
    sbc hl,de ; unsigned 24-bit elapsed, including wrap
    ld de,cues_alert_ticks
    or a
    sbc hl,de
    ret c
    xor a
    ld (cues_pre_alert),a
    ld hl,vox_warning
    ld a,(hl)
    ld (hl),0
    jp vox_say

cues_cancel_warning:
    xor a
    ld (cues_pre_alert),a
    ld (vox_warning),a
    ; Reselecting the waveform aborts an active note without starting one.
    ld hl,cues_abort
    ld bc,cues_klaxon-cues_abort
    rst.lil 18h
    ret

cues_abort:
    db 23,0,85h,14,4,1
cues_interrupt_speech:
    db 23,0,85h,11,4,1

    macro KLAXON_PULSE body_level
    db 127,4,0, body_level,146,0, 0,10,0 ; accent, body, trough (160 ms)
    endmacro

cues_klaxon:
    db 23,0,133,14,4,1
    db 23,0,133,14,6,2,23 ; 7 articulated pulses × 3 phases + 2 last phases
    KLAXON_PULSE 75
    KLAXON_PULSE 64
    KLAXON_PULSE 75
    KLAXON_PULSE 64
    KLAXON_PULSE 75
    KLAXON_PULSE 64
    KLAXON_PULSE 75
    db 127,4,0,64,156,0   ; last pulse flows directly into release
    db 0,1,0               ; no sustain phases, one release to silence
    dw cues_klaxon_tail_ms
    db 23,0,133,14,7,1,2,5,10,0,32,0,8,0,224,255,8,0
    db 23,0,133,14,0,80    ; twice the R06 note level, this channel only
    dw 470,cues_klaxon_gate_ms
cues_klaxon_end:
cues_rocket:
    db 23,0,133,12,4,4
    db 23,0,133,12,6,1,100,0,28,0,113,44,1
    db 23,0,133,12,7,0
    db 23,0,133,13,4,5
    db 23,0,133,13,6,1,48,0,80,0,89,74,0
    db 23,0,133,13,7,1,1,4,10,0,2,0,20,0
    db 23,0,133,12,0,39,0,0,220,0
    db 23,0,133,13,0,28,82,0,205,0
cues_rocket_end:
