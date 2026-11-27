; Stock Golem module. Six bytes per actor are written directly into the
; final outgoing packet. Only the two CALLs are appended at each boundary.
pose_eligible: db 0
pose_frame_n: db 0
pose_frame_sends: db 0
pose_cursor: dl pose_slots
pose_install:
    ld hl,pose_boot
    ld bc,pose_boot_end-pose_boot
    rst.lil 18h
    ret
pose_boot: incbin "pose_install.bin"
pose_boot_end:
    include "pose_package.inc"
pose_packet:
    db 23,0,160
    dw pose_state_id
    db 5,0c2h
pose_dest: dw 0
pose_size: dw 0
pose_slots: ds 32*6+15
    include "earth_pose_filter.inc"
    include "earth_pose_packed.inc"
pose_submit:
    ld hl,(pose_cursor)
    ld de,pose_slots
    or a
    sbc hl,de
    ret z
    ; HL = byte length; suffix of the static input span receives the bytes.
    ld a,l                ; max 192; high length byte stays zero
    ld (pose_size),a
    ld (pose_cursor),de
    ex de,hl
    ld hl,pose_input_tail
    or a
    sbc hl,de
    ld a,l
    ld (pose_dest),a
    ld a,h
    ld (pose_dest+1),a
    ; One descriptor per possible positive length (6,12,...,192).
    ; Two 3-byte entries per descriptor; input byte count is its end offset.
    ld hl,pose_descriptors-6
    add hl,de
    ld bc,(hl)            ; entry buffer, high padding zero
    inc hl
    inc hl
    inc hl
    ld hl,(hl)            ; 24-bit offset of first populated output record
    push hl
    ld ix,pose_slots
    add ix,de
    ld (ix+0),23
    ld (ix+1),0
    ld (ix+2),160
    ld (ix+3),bc
    ld (ix+5),1
    ld (ix+6),23
    ld (ix+7),0
    ld (ix+8),160
    ld bc,pose_state_id
    ld (ix+9),bc
    ld (ix+11),11
    pop hl
    ld (ix+12),hl
    ld hl,26
    add hl,de
    push hl
    pop bc
    ld hl,pose_packet
    rst.lil 18h
    ld hl,pose_frame_sends
    inc (hl)
    ret
pose_barrier:
    push af
    push bc
    push de
    push hl
    push ix
    call pose_submit
    pop ix
    pop hl
    pop de
    pop bc
    pop af
    ret
