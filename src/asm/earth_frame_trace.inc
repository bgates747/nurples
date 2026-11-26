; Included only in the unattended frame-trace comparison.
trace_cursor: dl trace_rows
trace_flags: db 0
trace_sd: dl 0
trace_length: dl 0
trace_reset:
    ld hl,trace_rows
    ld (trace_cursor),hl
    xor a
    ld (trace_truncated),a
    ld (bench_complete),a
    ret
trace_begin_frame:
    xor a
    ld (trace_flags),a
    ld hl,0
    ld (trace_sd),hl
    ret
trace_mark_read:
    ld a,(trace_flags)
    or a
    jr z,@first
    or 16 ; unexpected multiple refills: not a single-read latency
@first:
    and 16
    or 1
    ld (trace_flags),a
    ret

; Called with the already computed nonnegative SD sample in HL. Preserve it
; for the unmodified jit_read_max comparison. No second timer sample.
trace_valid_sd:
    push hl
    ld de,65535
    or a
    sbc hl,de
    ld a,(trace_flags)
    jr c,@fits
    jr z,@fits
    or 4
    pop hl
    push hl
    ex de,hl
    jr @store
@fits:
    pop hl
    push hl
@store:
    or 2
    ld (trace_flags),a
    ld (trace_sd),hl
    pop hl
    ret

; HL is this measured update's raw MOS delta; it must survive for pace_raw.
trace_append:
    push hl
    ld hl,(trace_cursor)
    ld de,trace_rows_end
    or a
    sbc hl,de
    jr c,@room
    ld a,1
    ld (trace_truncated),a
    pop hl
    ret
@room:
    pop hl
    push hl
    ld a,(trace_flags)
    bit 0,l
    jr z,@even
    or 8
@even:
    ld de,512
    or a
    sbc hl,de
    jr c,@small
    or 32
    ld c,255
    jr @record
@small:
    add hl,de
    srl h
    rr l
    ld c,l
@record:
    ld hl,(trace_cursor)
    ld de,(trace_sd)
    ld (hl),e
    inc hl
    ld (hl),d
    inc hl
    ld (hl),c
    inc hl
    ld (hl),a
    inc hl
    ld (trace_cursor),hl
    pop hl
    ret
