; Write 6 bytes directly into the outgoing state array (ID, frame, X, Y).
; Share with Golem/native; retain fractional coordinate carry before dropping
; the fraction and leave signed 16-bit screen X/Y in little-endian order.
pose_publish:
    ld ix,(pose_cursor)
    ld hl,origin_left*256
    ld bc,(iy+sprite_x)
    add hl,bc
    ld (ix+1),hl
    ld a,(ix+3)
    ld hl,origin_top*256
    ld bc,(iy+sprite_y)
    add hl,bc
    ld (ix+3),hl
    ld (ix+3),a
    ld a,(iy+sprite_id)
    ld (ix+0),a
    ld a,(iy+sprite_frame)
    ld (ix+1),a
    lea hl,ix+6
    ld (pose_cursor),hl
    ld hl,pose_frame_n
    inc (hl)
    ret
