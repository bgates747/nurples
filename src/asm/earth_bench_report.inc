; Unattended alignment fixture. Write only after main_end stopped the clock,
; closed the stream, and pace_report calculated pace_ticks.
bench_complete: db 0
bench_name: asciz "EARTHAB.BIN"
bench_record:
    db "EAB1"
bench_offset: dl 0
bench_vblanks: dl 0
bench_updates: dl 0
bench_zero: dl 0
bench_skips: dl 0
bench_claims: dl 0
bench_ms: dl 0
bench_sd: dl 0
bench_batch: dl 0
bench_wait_valid: db 0
bench_record_end:
bench_save:
    ld a,(bench_complete)
    or a
    jp z,@fail
    ld a,(jit_error)
    or a
    jp nz,@fail
    ld hl,(pace_ticks)
    ld (bench_vblanks),hl
    ld de,(pace_frames)
    ld (bench_updates),de
    or a
    sbc hl,de
    ld de,(pace_empty)
    ld (bench_zero),de
    add hl,de
    ld (bench_skips),hl
    ld hl,(claim_immediate)
    ld (bench_claims),hl
    ld hl,(wait_clock_frac)
    ld de,72
    call udiv24
    ld de,(wait_clock_ms)
    add hl,de
    ld (bench_ms),hl
    ld a,(wait_clock_invalid)
    xor 1
    ld hl,wait_clock_ready
    and (hl)
    ld (bench_wait_valid),a
    ld hl,(jit_read_max)
    ld (bench_sd),hl
    ld hl,(jit_send_max)
    ld (bench_batch),hl
    ld hl,agnb_fil
    ld de,bench_name
    ld c,fa_write+fa_create_new
    FFSCALL ffs_fopen
    or a
    jr nz,@fail
    ld hl,agnb_fil
    ld de,bench_record
    ld bc,bench_record_end-bench_record
    FFSCALL ffs_fwrite
    or a
    jr nz,@close
    ld hl,bench_record_end-bench_record
    sbc hl,bc
@close:
    push af
    ld hl,agnb_fil
    FFSCALL ffs_fclose
    or a
    jr nz,@close_failed
    pop af
    jr nz,@fail
    call printInline
    asciz "EARTH AB SAVED\r\n"
    ret
@close_failed:
    pop af
@fail:
    call printInline
    asciz "EARTH AB FAILED\r\n"
    ret
