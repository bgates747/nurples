; EDL1 diagnostic only. The MOS epoch is read atomically by the same ADL
; load as the scheduler. No IRQ changes, timer reprogramming or VDU traffic.
; Tag zero is padding, not "other". The first edge is a due opportunity.
dl_active: db 0
dl_parent: db 1
dl_last: dl 0
dl_used: db 0
dl_tags: db 0,0,0
dl_flags: db 0 ; bit 0: pending at claim; bit 1: attribution unavailable

; Finish pace_classify (A = pace_count_this). Preserve the caller's state.
dl_begin:
    push af
    ld (dl_active),a
    or a
    jr z,@done
    push hl
    ld hl,0
    ld (dl_tags),hl
    xor a
    ld (dl_used),a
    ld (dl_flags),a
    ld hl,(pace_prev)
    ld (dl_last),hl
    pop hl
    ld a,1
    ld (dl_parent),a
    ld a,2 ; previous claim to frame classification, including previous sample overhead
    call dl_observe
@done:
    pop af
    ret

; Transition between nonnested foreground parents (other, tiles, actors,
; wait). Leaf probes below these never change the parent.
dl_set_parent:
    push af
    ld a,(dl_parent)
    call dl_observe
    pop af
    ld (dl_parent),a
    ret

; A is the phase that just ran. Call sites preserve AF including MOS
; return status. Most calls see no edge; the common path needs no writes.
dl_observe:
    push bc
    ld b,a
    ld a,(dl_active)
    or a
    jr z,@done
    push hl
    push de
    call pace_read_clock
    call dl_record
    pop de
    pop hl
@done:
    pop bc
    ret

; B = phase; HL = observed epoch. Modulo-24 subtraction even at wrap.
dl_record:
    ld de,(dl_last)
    or a
    sbc hl,de
    ret z
    push hl
    add hl,de
    ld (dl_last),hl
    pop hl
    bit 0,l
    jr nz,@invalid
@next:
    ld a,(dl_used)
    cp 3
    ret nc ; header/row still keep the full skip count
    push hl
    ld hl,dl_tags
    ld de,0
    ld e,a
    add hl,de
    ld (hl),b
    pop hl
    ld a,(dl_used)
    inc a
    ld (dl_used),a
    dec hl
    dec hl
    ld de,0
    or a
    sbc hl,de
    jr nz,@next
    ret
@invalid:
    ld a,2
    ld (dl_flags),a
    ret

; Do NOT read the live clock here. A later IRQ belongs to the next update.
; pace_sample immediately reloads claim_epoch for its own accounting.
dl_end:
    push af
    ld a,(dl_active)
    or a
    jr z,@done
    push bc
    push de
    push hl
    ld hl,(claim_epoch)
    ld b,10
    call dl_record
    ld a,(claim_pending)
    ld hl,dl_flags
    or (hl)
    ld (hl),a
    xor a
    ld (dl_active),a
    pop hl
    pop de
    pop bc
@done:
    pop af
    ret
