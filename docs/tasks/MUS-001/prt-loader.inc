; Isolated FAE audition, not included by production. PRT1 programming follows
; AgonArcade RALLY-25/prt.inc. 18.432 MHz / 256 / 1440 = 50 Hz.
lp_reload: equ 1440
lp_record: db "LPR1",lp_music
lp_resettable:
lp_status: db 0 ; bit0 completed, bit1 timer busy, bit2 packet overflow, bit3 vector restored
lp_elapsed: dl 0 ; MOS clock units across identical UI/game/voice load boundaries
lp_irqs: dl 0
lp_sent: dl 0 ; serviced tracker periods, including empty command batches
lp_missed: dl 0 ; extra deadlines observed together; never burst-replay
lp_late: dl 0 ; worst lateness in /256 PRT counts (72 per ms)
lp_appends: dl 0
lp_bytes: dl 0
lp_record_end:
lp_owned: db 0
lp_capture: db 0
lp_handoff: db 0
lp_seen: dl 0
lp_size: dl 0
lp_reset_end:
lp_start_time: dl 0
lp_old_ctl: db 0
lp_old_iss: db 0
lp_old_vector: dl 0
lp_packet: ds 512
lp_filename: asciz "LOADPRT.BIN"

lp_begin:
    ld hl,lp_resettable
    ld de,lp_resettable+1
    ld bc,lp_reset_end-lp_resettable-1
    ld (hl),0
    ldir
    MOSCALL mos_sysvars
    ld hl,(ix+sysvar_time)
    ld (lp_start_time),hl
    .if lp_music
    in0 a,(083h)
    ld (lp_old_ctl),a
    and 041h
    jr z,@idle
    ld a,2
    ld (lp_status),a
    ret
@idle:
    call mus_audio_open
    ld hl,mus_loadtune
    call pt3_init
    ld a,1
    ld (mus_active),a
    ld (mus_title_active),a
    call lp_prepare
    in0 a,(092h)
    ld (lp_old_iss),a
    and 0f3h
    out0 (092h),a
    ld hl,lp_irq
    ld e,0ch
    MOSCALL mos_setintvector
    ld (lp_old_vector),hl
    ld hl,lp_reload
    out0 (084h),l
    out0 (085h),h
    ld a,1
    ld (lp_owned),a
    ld a,05fh ; IRQ, continuous, /256, load and enable
    out0 (083h),a
    .endif
    ret

lp_irq:
    push af
    push hl
    in0 a,(083h)
    ld hl,(lp_irqs)
    inc hl
    ld (lp_irqs),hl
    pop hl
    pop af
    ei
    reti.l

lp_stop_timer:
    ld a,(lp_owned)
    or a
    ret z
    xor a
    out0 (083h),a
    ld (lp_owned),a
    ld a,(lp_old_iss)
    out0 (092h),a
    ld hl,(lp_old_vector)
    ld e,0ch
    MOSCALL mos_setintvector
    ld de,lp_irq
    or a
    sbc hl,de
    jr nz,@done
    ld hl,lp_status
    set 3,(hl)
@done:
    ld a,(lp_old_ctl)
    and 07ch
    out0 (083h),a
    ret

; AY flush can happen mid-command at the library watermark. Collect *all*
; flush fragments in RAM; publish only the complete tick or none on overflow.
lp_flush:
    ld a,(lp_capture)
    or a
    jr nz,@capture
    rst.lil 18h
    ret
@capture:
    push hl
    ld hl,(lp_size)
    ld de,lp_packet
    add hl,de
    push hl
    add hl,bc
    ld de,lp_packet+513
    or a
    sbc hl,de
    pop de
    pop hl
    jr nc,@overflow
    push bc
    ldir
    pop bc
    ld hl,(lp_size)
    add hl,bc
    ld (lp_size),hl
    ret
@overflow:
    ld hl,lp_status
    set 2,(hl)
    ret

lp_prepare:
    ld hl,0
    ld (lp_size),hl
    ld a,1
    ld (lp_capture),a
    call pt3_play
    call ay_update
    xor a
    ld (lp_capture),a
    ret
lp_send:
    ld a,(lp_status)
    bit 2,a
    ret nz
    ld hl,(lp_size)
    ld bc,0
    or a
    sbc hl,bc
    ret z ; RST 18 with zero length means zero-terminated string!
    ld bc,(lp_size)
    ld hl,lp_packet
    rst.lil 18h
    ret

; Only called outside an outstanding VDU transaction. No waiting for time.
lp_pump:
    ld a,(lp_owned)
    or a
    ret z
@snapshot:
    ld bc,(lp_irqs)
    ld hl,(lp_seen)
    or a
    sbc hl,bc
    ret z
    ld de,0
    in0 e,(084h) ; low read latches high
    in0 d,(085h)
    ld hl,(lp_irqs)
    or a
    sbc hl,bc
    jr nz,@snapshot
    ld hl,lp_reload
    or a
    sbc hl,de
    push hl
    ld hl,(lp_seen)
    ld (lp_seen),bc
    push bc
    pop de
    ex de,hl
    or a
    sbc hl,de
    dec hl ; elapsed periods minus the tick we are about to play
    push hl
    ld de,(lp_missed)
    add hl,de
    ld (lp_missed),hl
    pop bc
    pop hl
    ld de,lp_reload
@extra:
    ld a,b
    or c
    jr z,@compare
    add hl,de
    dec bc
    jr @extra
@compare:
    ld de,(lp_late)
    or a
    sbc hl,de
    jr c,@send
    add hl,de
    ld (lp_late),hl
@send:
    call lp_send
    ld hl,(lp_sent)
    inc hl
    ld (lp_sent),hl
    jp lp_prepare

; BC = exact SD read length, DE = source, HL = explicit VDP buffer id.
lp_append:
    push ix
    push iy
@more:
    push hl
    push de
    push bc
    call lp_pump
    pop hl ; remaining
    ld bc,512
    or a
    sbc hl,bc
    jr nc,@piece
    add hl,bc
    push hl
    pop bc
    ld hl,0
@piece:
    pop de ; source
    ex (sp),hl ; HL=buffer id; stack top = remaining after this piece
    push hl
    push de
    push bc
    call vdu_load_buffer
    pop bc
    pop hl
    add hl,bc
    ex de,hl ; next source
    ld hl,(lp_bytes)
    add hl,bc
    ld (lp_bytes),hl
    ld hl,(lp_appends)
    inc hl
    ld (lp_appends),hl
    pop hl
    pop bc
    ld a,b
    or c
    jr nz,@more
    call lp_pump
    pop iy
    pop ix
    ret

lp_finish:
    .if lp_music
    call lp_pump
    ld a,(lp_owned)
    ld (lp_handoff),a
    call lp_stop_timer
    .endif
    MOSCALL mos_sysvars
    ld hl,(ix+sysvar_time)
    ld de,(lp_start_time)
    or a
    sbc hl,de
    ld (lp_elapsed),hl
    ld hl,lp_status
    set 0,(hl)
    ret

; First presentation frame consumes the *already decoded* next tick.
lp_frame:
    ld a,(lp_handoff)
    or a
    jr z,@ordinary
    xor a
    ld (lp_handoff),a
    jp lp_send
@ordinary:
    call pt3_play
    jp ay_update

lp_save:
    ld hl,lp_filename
    ld de,lp_record
    ld bc,lp_record_end-lp_record
    MOSCALL mos_save
    or a
    ret z
    call printInline
    asciz "Timing report write failed\r\n"
    ret
