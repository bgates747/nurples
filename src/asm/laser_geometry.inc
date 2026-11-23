; A vertical beam never changes X after launch. Bound its muzzle to its own
; bitmap footprint, not the 16-pixel ship's travel range. HL = muzzle X,
; IX = allocated laser. Returns HL = clamped 16.8 X (field_left = 0).
laser_bound_muzzle_x:
    ld de,0
    or a
    sbc hl,de
    jp p,@right
    ld hl,0
    ret
@right:
    push hl
    ld hl,[field_right+1]*256
    ld d,(ix+sprite_dim_x)
    or a
    sbc hl,de
    ex de,hl
    pop hl
    or a
    sbc hl,de
    jp nc,@clamp
    add hl,de
    ret
@clamp:
    ex de,hl
    ret

; IX = laser, IY = target. Return Z for a shared raster pixel, NZ otherwise.
; Ramming still uses the original contact-inclusive box test.
laser_hits_pixels:
    ld hl,(ix+sprite_x)
    ld de,(iy+sprite_x)
    ld b,(ix+sprite_dim_x)
    ld c,(iy+sprite_dim_x)
    call laser_axis_overlap
    jp nc,@miss
    ld hl,(ix+sprite_y)
    ld de,(iy+sprite_y)
    ld b,(ix+sprite_dim_y)
    ld c,(iy+sprite_dim_y)
    call laser_axis_overlap
    jp nc,@miss
    xor a
    ret
@miss:
    ld a,1
    or a
    ret

; Positive extents B,C in pixels. For delta = floor(beam) - floor(target),
; the two spans intersect iff -B < delta < C. Carry means overlap.
laser_axis_overlap:
    ld l,0
    ld e,0
    or a
    sbc hl,de
    ld de,0
    jp m,@beam_first
    ld d,c
    or a
    sbc hl,de
    ret
@beam_first:
    ld d,b
    dec de ; equality at the exclusive end is a miss
    add hl,de
    ret
