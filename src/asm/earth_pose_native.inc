; Private, unattended, freshly booted MOS CLI only. Mode and geometry have
; been acknowledged by game_prepare_playfield before this client's startup.
; No firmware modifications. No timer or asynchronous output in the client.
native_asset: equ 02b00h
native_args:  equ 02b01h
pose_eligible: db 0
pose_frame_n: db 0
pose_frame_sends: db 0
pose_cursor: dl pose_slots
pose_packet:
    db 23,0,160
    dw native_args
    db 5,0c2h
    dw 0
native_len: dw 0
native_n: db 0
pose_slots: ds 6*32+17
    include "earth_pose_filter.inc"
    include "earth_pose_packed.inc"
pose_submit:
    ld hl,(pose_cursor)
    ld de,pose_slots
    or a
    sbc hl,de
    ret z
    ld (pose_cursor),de
    push hl
    pop bc
    add hl,de
    ex de,hl               ; DE = end of populated state
    inc bc
    ld a,c
    ld (native_len),a
    push bc
    ld hl,native_run
    ld bc,17
    ldir                   ; append one RUN with the handle actually returned
    pop bc
    ld hl,28
    add hl,bc
    push hl
    pop bc                 ; packet = 11 header + 1 count + 6N + 17 RUN
    ld hl,pose_packet
    rst.lil 18h
    ld hl,pose_frame_sends
    inc (hl)
    ld hl,(native_sends)
    inc hl
    ld (native_sends),hl
    ld hl,0
    ld a,(native_n)
    ld l,a
    ld de,(native_poses)
    add hl,de
    ld (native_poses),hl
    xor a
    ld (native_n),a
    ret
pose_barrier:
    push af
    push bc
    push de
    push hl
    call pose_submit
    pop hl
    pop de
    pop bc
    pop af
    ret

; Published by the keyboard packet interrupt only, after all fields copied.
; Ordinary keys return to standard MOS processing unchanged. This private
; process is the sole user callback under a fresh unattended MOS invocation.
native_callback:
    push af
    push hl
    push de
    push bc
    push ix
    push de
    pop ix
    ld a,(ix+0)
    cp 'V'
    jr nz,@done
    ld a,(ix+1)
    cp 'D'
    jr nz,@done
    ld a,(ix+2)
    cp 'P'
    jr nz,@done
    ld a,(ix+3)
    cp 'L'
    jr nz,@done
    ld a,(ix+4)
    cp 1
    jr nz,@neutral
    ld a,(native_token)
    cp (ix+6)
    jr nz,@neutral
    ld a,(ix+7)
    or a
    jr nz,@neutral
    ld a,(ix+8)
    ld (native_value),a
    ld a,(ix+9)
    ld (native_value+1),a
    ld a,(ix+5)
    ld (native_got),a
@neutral:
    xor a
    ld (ix+0),a
    ld (ix+1),a
    ld (ix+2),a
    ld (ix+3),a
@done:
    pop ix
    pop bc
    pop de
    pop hl
    pop af
    ret

; HL=packet, BC=length, D=unique small token, E=expected reply kind.
; Carry set only on matched success. Bound by both MOS ticks and a finite
; instruction budget if the clock is not progressing.
native_query:
    push de
    xor a
    ld (native_got),a
    ld (native_value),a
    ld (native_value+1),a
    ld a,d
    ld (native_token),a
    rst.lil 18h
    MOSCALL mos_sysvars
    ld hl,(ix+sysvar_time)
    ld (native_start),hl
    ld bc,0ffffffh
@spin:
    ld a,(native_got)
    or a
    jr nz,@arrived
    ld hl,(ix+sysvar_time)
    ld de,(native_start)
    or a
    sbc hl,de
    ld de,360              ; three seconds @120 Hz
    or a
    sbc hl,de
    jr nc,@fail
    dec bc
    ld hl,0
    or a
    sbc hl,bc
    jr nz,@spin
@fail:
    pop de
    jr native_capture_failure
@arrived:
    pop de
    cp e
    jr nz,@bad
    ld hl,(native_value)
    scf
    ret
@bad:
native_capture_failure:
    ld a,(native_failed)
    or a
    ret nz
    ld a,1
    ld (native_failed),a
    ld a,(native_token)
    ld (native_bad_token),a
    ld a,(native_got)
    ld (native_bad_kind),a
    ld hl,(native_value)
    ld (native_bad_value),hl
    or a
    ret

native_startup:
    xor a
    ld (native_owned),a
    ld (native_failed),a
    ld hl,0
    ld (native_sends),hl
    ld (native_poses),hl
    ld hl,native_callback
    ld c,0
    MOSCALL mos_setkbvector
    ld a,1
    ld (native_hook),a
    ld hl,native_discover
    ld bc,3
    ld de,1                ; token 0, kind IDENT=1
    call native_query
    ret nc
    ld a,l
    and 5
    cp 5
    jr nz,@bad
    ld hl,native_profile
    ld bc,13
    ld de,0104h            ; token 1, kind VALUE=4
    call native_query
    ret nc
    ld de,8
    or a
    sbc hl,de
    jr nz,@bad             ; require qualified sprite grammar
    ld hl,native_slot
    ld bc,13
    ld de,0204h
    call native_query
    ret nc
    ld a,h
    or l
    jr nz,@bad             ; never replace another module
    ld hl,native_buffers
    ld bc,native_buffers_end-native_buffers
    rst.lil 18h
    ld hl,native_load
    ld bc,19
    ld de,0302h
    call native_query
    ret nc
    ld a,h
    or l
    jr z,@bad
    ld a,l
    ld (native_handle),a
    ld (native_finish_handle),a
    ld (native_unload_handle),a
    ld a,h
    ld (native_handle+1),a
    ld (native_finish_handle+1),a
    ld (native_unload_handle+1),a
    ld a,1
    ld (native_owned),a
    scf
    ret
@bad:
    jp native_capture_failure

native_shutdown:
    ld a,(native_hook)
    or a
    ret z
    ld a,(native_owned)
    or a
    jr z,@unhook
    ld hl,native_check_counts
    ld bc,17
    rst.lil 18h
    ld hl,native_finish
    ld bc,17
    ld de,0403h
    call native_query
    jr c,@unload
    xor a
    ld (bench_complete),a   ; silent RUN refusals cannot look like a speedup
    ; On a timeout, first fence the original request, rather than issuing
    ; UNLOAD on an uncertain stream. If unavailable, flag the failed job
    ; for the known bench reset; never guess that module cleanup succeeded.
    call vdu_query_mode
    jr nc,@remove
@unload:
    ld hl,native_unload
    ld bc,13
    ld de,0503h
    call native_query
    jr c,@unhook
    xor a
    ld (bench_complete),a
@unhook:
    ; Ordered normal query consumes preceding VDP input before removing
    ; the callback, including on a failure with a late private response.
    call vdu_query_mode
    jr c,@remove
    xor a
    ld (bench_complete),a
    call native_capture_failure
@remove:
    ld hl,0
    ld c,0
    MOSCALL mos_setkbvector ; never return to MOS with application ISR installed
    xor a
    ld (native_hook),a
    ld (native_owned),a
    ret

native_diagnostic:
    ld a,(native_failed)
    or a
    ret z
    call printInline
    asciz "Native pose token/kind/value (hex): "
    ld a,(native_bad_token)
    call printHex8
    call printInline
    asciz "/"
    ld a,(native_bad_kind)
    call printHex8
    call printInline
    asciz "/"
    ld a,(native_bad_value+1)
    call printHex8
    ld a,(native_bad_value)
    call printHex8
    jp printNewLine

native_owned: db 0
native_hook: db 0
native_got: db 0
native_token: db 0
native_value: dl 0
native_failed: db 0
native_bad_token: db 0
native_bad_kind: db 0
native_bad_value: dl 0
native_start: dl 0
native_discover: db 23,0,0d9h
native_profile: db 23,0,0d8h,8,0,1,4,1,0,1,0,24,0
native_slot: db 23,0,0d8h,8,0,1,4,2,0,1,0,2,0
native_load: db 23,0,0d8h,14,0,1,1,3,0,1,0
    dw native_asset,native_blob_end-native_blob,0,0
native_run: db 23,0,0d8h,12,0,1,2,0,0,0,0
native_handle: dw 0
    dw 0,native_args
native_finish: db 23,0,0d8h,12,0,1,2,4,0,1,0
native_finish_handle: dw 0
    dw 1,native_args
native_unload: db 23,0,0d8h,8,0,1,3,5,0,1,0
native_unload_handle: dw 0
native_check_counts:
    db 23,0,160
    dw native_args
    db 5,0c2h
    dw 0,6
native_sends: dl 0
native_poses: dl 0
native_buffers:
    db 23,0,160
    dw native_args
    db 2,23,0,160
    dw native_args
    db 0
    dw 193
    ds 193
    db 23,0,160
    dw native_asset
    db 2,23,0,160
    dw native_asset
    db 0
    dw native_blob_end-native_blob
native_blob: incbin "pose.vdpl"
native_blob_end:
native_buffers_end:
