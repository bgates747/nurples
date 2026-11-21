; VOX-001 isolated campaign audition. Music owns 0-4, laser/blast 5-7,
; hollow scoring ping 8-10 and one interruptible speech channel 11.
vox_filename: db "voice.agnb",0
vox_pings: dl 0
vox_warning: db 0 ; 0 none, 2 depleted, 3 critical (low byte of buffer ID)
vox_quiet_crater: db 0

vox_reset:
    xor a
    ld (vox_warning),a
    ld (vox_quiet_crater),a
    ld hl,0
    ld (vox_pings),hl
    ret

; Entered only after a real, gated player contact. A = new (clamped)
; shields; the old value has not yet been overwritten. Preserve caller ABI.
vox_contact:
    push af
    push hl
    call mus_queue_blast ; one explosion on contact, even if the enemy survives
    cp bar_yellow
    jr nc,@done
    cp bar_red
    ld a,(player_shields)
    jr nc,@yellow
    cp bar_red
    jr c,@done
    ld a,3
    jr @queue
@yellow:
    cp bar_yellow
    jr c,@done
    ld a,(vox_warning)
    cp 3
    jr z,@done
    ld a,2
@queue:
    ld (vox_warning),a
@done:
    pop hl
    pop af
    ret

; An electrode craters synchronously, by either laser or player contact.
; The contact already queued its blast; cratering must not add a score ping.
vox_contact_electrode:
    push af
    ld a,1
    ld (vox_quiet_crater),a
    pop af
    call kill_electrode
    push af
    xor a
    ld (vox_quiet_crater),a
    pop af
    ret
vox_crater_sound:
    push af
    ld a,(vox_quiet_crater)
    or a
    jr nz,@quiet
    pop af
    jp vox_queue_ping
@quiet:
    pop af
    ret

vox_queue_ping:
    push hl
    ld hl,(vox_pings)
    inc hl
    ld (vox_pings),hl
    pop hl
    ret

vox_open:
    ld hl,@cmd
    ld bc,@end-@cmd
    rst.lil 18h
    ret
@cmd: db 23,0,85h,8,8, 23,0,85h,9,8, 23,0,85h,10,8, 23,0,85h,11,8
@end:
vox_close:
    ld hl,@cmd
    ld bc,@end-@cmd
    rst.lil 18h
    ret
@cmd: db 23,0,85h,11,9, 23,0,85h,10,9, 23,0,85h,9,9, 23,0,85h,8,9
@end:

; Called on Game Over entry only. The initial LOADTUNE has no speech.
vox_game_over:
    ld a,1
vox_say:
    ld (@id),a
    ld hl,@cmd
    ld bc,@end-@cmd
    rst.lil 18h
    ret
; Abort the old note, select explicit sample buffer FB01..FB03, then play
; its natural duration. Never change master volume or music envelopes.
@cmd: db 23,0,85h,11,12,0,0,0
    db 23,0,85h,11,4,8
@id: db 1,0FBh
    db 23,0,85h,11,0,100,0,0,0,0
@end:

vox_frame:
    ld hl,(vox_pings)
    ld de,0
    or a
    sbc hl,de
    jr z,@speech
    dec hl
    ld (vox_pings),hl
    ld hl,vox_hull
    ld bc,vox_hull_end-vox_hull
    rst.lil 18h
    jr vox_frame
@speech:
    ld a,(vox_warning)
    or a
    ret z
    ld hl,vox_warning
    ld (hl),0
    jp vox_say
vox_hull:
    incbin "hull.vdu"
vox_hull_end:
