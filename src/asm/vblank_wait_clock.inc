; Wall time of the outer vdu_vblank only, including interrupts and lp_poll.
; PRT2 in single-pass /256 mode, no IRQ, no dependency on the music timer.
wait_clock_ready: db 0
wait_clock_saved: db 0
wait_clock_ms: dl 0
wait_clock_frac: dl 0
wait_clock_invalid: db 0
claim_reset: db 0
claim_epoch: dl 0
claim_pending: db 0
claim_immediate: dl 0
claim_fresh_wait: db 0

wait_clock_init:
    xor a
    ld (wait_clock_ready),a
    ld (wait_clock_invalid),a
    ld (claim_pending),a
    ld (claim_reset),a
    ld hl,0
    ld (claim_immediate),hl
    ld (wait_clock_ms),hl
    ld (wait_clock_frac),hl
    in0 a,(092h)
    and 030h ; TMR2 must already use system clock
    ret nz
    in0 a,(086h)
    ld (wait_clock_saved),a
    and 041h ; leave an enabled or IRQ-reserved timer alone
    ret nz
    ld a,1
    ld (wait_clock_ready),a
    ret

; Count a bypass only once the following update really starts (not ESC or P).
claim_count_start:
    ld a,(claim_pending)
    or a
    ret z
    xor a
    ld (claim_pending),a
    ld hl,(claim_immediate)
    inc hl
    ld (claim_immediate),hl
    ret

wait_clock_frame:
    xor a
    ld (claim_pending),a
    ld (claim_fresh_wait),a
    ld a,(claim_reset)
    or a
    jr nz,@ordinary
    ld a,(pace_count_this)
    or a
    jr z,@ordinary
    ld a,(game_pause_active)
    or a
    jr nz,@ordinary
    ; Check the successor too: do not release Game Over on an old play edge.
    ld hl,(game_state)
    ld de,game_playing
    or a
    sbc hl,de
    jr z,@eligible
    ld hl,(game_state)
    ld de,game_level_transition
    or a
    sbc hl,de
    jr nz,@ordinary
@eligible:
    call pace_read_clock
    ld de,(pace_prev) ; last sampled/consumed boundary, not wait-entry time
    or a
    sbc hl,de
    jr z,@wait
    add hl,de ; restore exact observed epoch, including modulo-24 wrap
    ld (claim_epoch),hl
    ld a,1
    ld (claim_pending),a
    ret
@ordinary:
    xor a
    ld (claim_reset),a
    inc a
    ld (claim_fresh_wait),a
    ld a,(pace_count_this)
    or a
    jp z,claim_ordinary_wait
    ld a,(game_pause_active)
    or a
    jp nz,claim_ordinary_wait
@wait:
    ld a,(wait_clock_ready)
    or a
    jp z,claim_wait
    ld a,0ffh
    out0 (087h),a
    out0 (088h),a
    ld a,00fh ; single-pass, /256, load + start
    out0 (086h),a
    call claim_wait
    ld de,0
    in0 e,(087h) ; low read latches high
    in0 d,(088h)
    in0 a,(086h)
    bit 7,a
    jr z,@stop
    ld a,1 ; never report a truncated wait as a valid total
    ld (wait_clock_invalid),a
@stop:
    ld a,(wait_clock_saved)
    and 07ch
    out0 (086h),a
    ld hl,65535
    or a
    sbc hl,de
    ld de,(wait_clock_frac)
    add hl,de
    ld de,72000 ; 72 ticks/ms; carry whole seconds without per-frame division
    or a
    sbc hl,de
    jr nc,@second
    add hl,de
    jr @save
@second:
    push hl
    ld hl,(wait_clock_ms)
    ld de,1000
    add hl,de
    ld (wait_clock_ms),hl
    pop hl
@save:
    ld (wait_clock_frac),hl
    ret

; Keep the bounded wait and the music service cadence. In particular, never
; take a fresh baseline here: an edge during timer setup is already sufficient.
claim_wait:
    ld a,(claim_fresh_wait)
    or a
    jr nz,claim_ordinary_wait
    ld de,(pace_prev)
    ld bc,0
@poll:
    call pace_read_clock
    or a
    sbc hl,de
    jr nz,@seen
    dec c
    jr nz,@poll
    call lp_poll
    djnz @poll
    ld hl,vdu_timing_fault_flags
    set 0,(hl)
    call pace_read_clock
    jr @store
@seen:
    add hl,de
@store:
    ld (claim_epoch),hl
    ret
claim_ordinary_wait:
    call vdu_vblank
    call pace_read_clock
    ld (claim_epoch),hl
    ret

wait_clock_report:
    call printInline
    asciz "Immediate play/warp starts: "
    ld hl,(claim_immediate)
    call printDec
    call printNewLine
    ld a,(wait_clock_ready)
    or a
    jr z,@invalid
    ld a,(wait_clock_invalid)
    or a
    jr nz,@invalid
    call printInline
    asciz "VBlank wait total (ms): "
    ld hl,(wait_clock_frac)
    ld de,72
    call udiv24
    ld hl,(wait_clock_ms)
    add hl,de
    call printDec
    jp printNewLine
@invalid:
    call printInline
    asciz "VBlank wait timing unavailable"
    jp printNewLine
