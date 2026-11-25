; Fixture only. sysvar_time is updated by 2 per serviced VBlank IRQ.
pace_raw: dl 0
pace_frames: dl 0
pace_empty: dl 0
pace_prev: dl 0
pace_count_this: db 0
pace_read_clock:
    ld hl,(000000h) ; operand bound to MOS sysvars at process entry
    ret
pace_init:
    MOSCALL mos_sysvars
    ld (pace_read_clock+1),ix
    ld hl,0
    ld (pace_raw),hl
    ld (pace_frames),hl
    ld (pace_empty),hl
    call pace_read_clock
    ld (pace_prev),hl
    ret

; Classify by the state that actually executes, not its successor.
pace_classify:
    ld hl,(game_state)
    ld de,game_playing
    or a
    sbc hl,de
    jr z,@yes
    ld hl,(game_state)
    ld de,game_level_transition
    or a
    sbc hl,de
    ld a,0
    jr nz,@save
@yes:
    ld a,1
@save:
    ld (pace_count_this),a
    ret

; Called immediately after every outer wait, including pause/menu frames.
pace_sample:
    call pace_read_clock
    ld de,(pace_prev)
    ld (pace_prev),hl
    or a
    sbc hl,de ; modulo 24 bits: safe across MOS low-word/24-bit wrap
    ld a,(pace_count_this)
    or a
    ret z
    ld a,(game_pause_active)
    or a
    ret nz
    ld de,0
    sbc hl,de
    jr nz,@elapsed
    ld de,(pace_empty)
    inc de
    ld (pace_empty),de
@elapsed:
    ld de,(pace_raw)
    add hl,de
    ld (pace_raw),hl
    ld hl,(pace_frames)
    inc hl
    ld (pace_frames),hl
    ret

pace_report:
    ; Convert a copy of the 24-bit MOS count to VBlanks for decimal output.
    ld hl,(pace_raw)
    ld (pace_ticks),hl
    ld ix,pace_ticks
    srl (ix+2)
    rr (ix+1)
    rr (ix+0)
    call printInline
    asciz "Skipped / VBlanks: "
    ld hl,(pace_ticks)
    ld de,(pace_frames)
    or a
    sbc hl,de
    ld de,(pace_empty)
    add hl,de ; zero-tick waits consume no opportunity
    call printDec
    call printInline
    asciz " / "
    ld hl,(pace_ticks)
    call printDec
    call printNewLine
    call printInline
    asciz "Updates / zero-tick waits: "
    ld hl,(pace_frames)
    call printDec
    call printInline
    asciz " / "
    ld hl,(pace_empty)
    call printDec
    jp printNewLine
pace_ticks: dl 0
