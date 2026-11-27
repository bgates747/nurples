; Private pose comparison. Only ordinary, already moving Earth formation
; actors collect here. Initialization and second-pass pose+show remain direct.
pose_eligible: db 0
pose_frame_n: db 0
pose_frame_sends: db 0
pose_cursor: dl pose_slots
pose_slots:
    include "pose_templates.inc"
pose_end:
    .if pose_end-pose_slots-480
        dl pose_pool_template_mismatch
    .endif
    include "earth_pose_filter.inc"

pose_publish:
    ld hl,pose_frame_n
    inc (hl)
    .if pose_direct
    ld a,(pose_frame_sends)
    add a,3
    ld (pose_frame_sends),a
    ld a,(iy+sprite_id)
    call vdu_sprite_select
    ld a,(iy+sprite_frame)
    call vdu_sprite_select_frame
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    jp vdu_sprite_move_abs168
    .else
    ; At most one append per air slot per first pass. Never write the buffer
    ; during async TX: submission ends that run, and barriers join it before
    ; processing any further actor that can append.
    ld ix,(pose_cursor)
    ld a,(iy+sprite_id)
    ld (ix+3),a
    ld a,(iy+sprite_frame)
    ld (ix+7),a
    ld bc,(iy+sprite_x)
    ld hl,origin_left*256
    add hl,bc
    ld (ix+10),hl
    ld (ix+10),13 ; drop fraction, restore move opcode
    ld bc,(iy+sprite_y)
    ld hl,origin_top*256
    add hl,bc
    ld a,(ix+12)
    ld (ix+12),hl
    ld (ix+12),a ; drop fraction, preserve X high byte
    lea hl,ix+15
    ld (pose_cursor),hl
    ret
    .endif

pose_submit:
    .if pose_direct
    ret
    .endif
    ld hl,(pose_cursor)
    ld de,pose_slots
    or a
    sbc hl,de
    ret z ; never call RST 18 with BC=0 (zero-terminated mode)
    push hl
    pop bc
    ex de,hl
    ld (pose_cursor),hl
    ld a,(pose_frame_sends)
    inc a
    ld (pose_frame_sends),a
    .if pose_async
    xor a
    ld (pose_released),a
    inc a
    ld (pose_in_flight),a
    ld de,pose_callback
    ld a,025h
    rst.lil 08h
    .else
    rst.lil 18h
    .endif
    ret

pose_barrier:
    .if pose_direct
    ret
    .endif
    push af
    push hl
    push bc
    push de
    call pose_submit
    .if pose_async
    ld a,(pose_in_flight)
    or a
    jr z,@joined
@source:
    ld a,(pose_released)
    or a
    jr z,@source
@wire:
    in0 a,(0c5h)
    bit 6,a
    jr z,@wire
    xor a
    ld (pose_in_flight),a
@joined:
    .endif
    pop de
    pop bc
    pop hl
    pop af
    ret
    .if pose_async
pose_in_flight: db 0
pose_released: db 0
pose_callback:
    ld a,1
    ld (pose_released),a
    ret
    .endif
