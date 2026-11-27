; Private EPT1 observer, not included by EPS1 fixtures or the shipping game.
; Vectors select one kind of scope for a whole sample frame: no nested clocks.
pc_countdown: db 17
pc_next: db 1
pc_active: db 0
pc_ordinal: dl 0
pc_ptr: dl pc_events
pc_publish: jp pose_publish
pc_submit: jp pc_submit_body
pc_send: jp pc_plain_send
pc_plain_send:
    rst.lil 18h
    ret

pc_frame:
    push af
    push hl
    ld a,(pc_active)
    or a
    jr z,@classify
    ld hl,pose_publish
    ld (pc_publish+1),hl
    ld hl,pc_submit_body
    ld (pc_submit+1),hl
    ld hl,pc_plain_send
    ld (pc_send+1),hl
    xor a
    ld (pc_active),a
@classify:
    ld a,(pace_count_this)
    or a
    jr z,@done
    ld a,(game_pause_active)
    or a
    jr nz,@done
    ld hl,pc_countdown
    dec (hl)
    jr nz,@done
    ld (hl),61
    ld a,(lp_owned)
    or a
    jr nz,@sample
    ld a,1
    ld (pc_bad),a
    jr @done
@sample:
    ld hl,(pace_frames)
    inc hl
    ld (pc_ordinal),hl
    call pc_empty_pair
    ld a,(pc_next)
    ld (pc_active),a
    cp 2
    jr z,@submit
    jr nc,@send
    ld hl,pc_watch_publish
    ld (pc_publish+1),hl
    ld a,2
    jr @rotate
@submit:
    ld hl,pc_watch_submit
    ld (pc_submit+1),hl
    ld a,3
    jr @rotate
@send:
    ld hl,pc_watch_send
    ld (pc_send+1),hl
    ld a,1
@rotate:
    ld (pc_next),a
@done:
    pop hl
    pop af
    ret

pc_watch_publish:
    push af
    ld a,1
    call pc_stamp
    pop af
    call pose_publish
    push af
    ld a,2
    jr pc_finish
pc_watch_submit:
    push af
    push bc
    push hl
    pop bc              ; pre-header payload bytes from pose_submit
    ld a,3
    call pc_stamp
    pop bc
    pop af
    call pc_submit_body
    push af
    ld a,4
    jr pc_finish
pc_watch_send:
    push af
    ld a,5
    call pc_stamp
    pop af
    rst.lil 18h
    push af
    ld a,6
pc_finish:
    call pc_stamp
    pop af
    ret
pc_empty_pair:
    push af
    ld a,7
    call pc_stamp
    pop af
    call pc_nothing
    push af
    ld a,8
    jr pc_finish
pc_nothing:
    ret

; A=event, BC=actual wire length on send entry. Leave all operand/return
; registers untouched. PRT1 remains owned by the normal 50 Hz music player.
pc_stamp:
    push ix
    push hl
    push de
    push bc
    ld ix,(pc_ptr)
    ld hl,pc_end
    ld de,(pc_ptr)
    or a
    sbc hl,de
    jr z,@full
    ld (ix+0),a
    ld hl,(pc_ordinal)
    ld (ix+1),l
    ld (ix+2),h
    ld (ix+8),c
    ld (ix+9),b
    call jit_clock ; BC=stable reload epoch, DE=latched downcounter
    ld (ix+3),bc
    ld (ix+6),e
    ld (ix+7),d
    lea hl,ix+10
    ld (pc_ptr),hl
    jr @done
@full:
    ld a,2
    ld (pc_bad),a
@done:
    pop bc
    pop de
    pop hl
    pop ix
    ret
