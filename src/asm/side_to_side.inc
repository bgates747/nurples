; AIR-001 motion component. IY is a validated allocated actor; coordinates
; are bitmap top-left, not center. No custom slots or global scratch.
; The future pad/VDP adapter consumes A after construction and after each tick.
; Do not install as a legacy callback that ignores A: partial is NOT dead.
side_visible: equ 0
side_hidden:  equ 1 ; still moving, but must not publish this footprint
side_exited:  equ 2 ; retire through the pool's normal deactivation path

; Post-allocation constructors. Position must already be set by the allocator.
; BC is packed VX,VY; no spawn timer, pool, sprite ID or speed chosen here.
air_size_small: equ 0x1010 ; D=width, E=height
air_size_medium: equ 0x2010
air_size_large: equ 0x1c1c
side_spawn_small:
    ld de,air_size_small
    jp side_spawn_inward
side_spawn_medium:
    ld de,air_size_medium
    jp side_spawn_inward
side_spawn_large:
    ld de,air_size_large
side_spawn_inward:
    ld (iy+sprite_dim_x),d
    ld (iy+sprite_dim_y),e
    call side_to_side_init
    ; Compare twice top-left X with twice the midpoint of the legal X range.
    ; This also works for fractional positions and odd sprite widths.
    ld hl,(iy+sprite_x)
    add hl,hl
    call side_right_limit_de
    or a
    sbc hl,de
    push af
    ld hl,(iy+sprite_xvel)
    bit 7,(iy+sprite_xvel+2)
    call nz,neg_hlu ; widen first: abs(packed -128) is representable here
    pop af
    jp m,@set
    call neg_hlu
@set:
    ld (iy+sprite_xvel),hl
    jp side_visibility

; Generic signed initialization (no launch-direction policy). Clamp initial
; placement before the first publication, without consuming a movement tick.
side_to_side_init:
    ld a,b
    call side_unpack
    ld (iy+sprite_xvel),hl
    ld a,c
    call side_unpack
    ld (iy+sprite_yvel),hl
    ld hl,(iy+sprite_x)
    call side_bound_x
    jp side_visibility

side_unpack:
    ld hl,0
    bit 7,a
    jp z,@positive
    dec hl
@positive:
    ld l,a
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
    ret

; One logical tick. Output A = side_visible/side_hidden/side_exited.
side_to_side:
    ld hl,(iy+sprite_x)
    ld de,(iy+sprite_xvel)
    add hl,de
    call side_bound_x
    ld hl,(iy+sprite_y)
    ld de,(iy+sprite_yvel)
    add hl,de
    ld (iy+sprite_y),hl
    jp side_visibility

; In this game's field-relative coordinate system field_left = 0.
side_right_limit_de:
    push hl
    ld hl,[field_right+1]*256
    ld de,0
    ld d,(iy+sprite_dim_x)
    or a
    sbc hl,de
    ex de,hl
    pop hl
    ret

side_bound_x:
    ld de,0
    or a
    sbc hl,de
    jp m,@left
    jp z,@left
    call side_right_limit_de
    or a
    sbc hl,de
    jp nc,@right
    add hl,de
    ld (iy+sprite_x),hl
    ret
@right:
    ld (iy+sprite_x),de
    bit 7,(iy+sprite_xvel+2)
    ret nz ; already heading into the field
    jp @reverse
@left:
    ld (iy+sprite_x),de ; DE=0
    bit 7,(iy+sprite_xvel+2)
    ret z
@reverse:
    ld hl,(iy+sprite_xvel)
    call neg_hlu
    ld (iy+sprite_xvel),hl
    ret

side_visibility:
    ld hl,(iy+sprite_y)
    ld l,0 ; raster uses floor(Y), including negative fractional positions
    ld de,[field_bottom+1]*256
    or a
    sbc hl,de
    jp p,@exited
    add hl,de
    ld de,0
    ld d,(iy+sprite_dim_y)
    bit 7,(iy+sprite_y+2) ; field_top = 0
    jp nz,@above
    add hl,de ; first raster row below sprite
    ld de,[field_bottom+2]*256
    or a
    sbc hl,de
    ld a,side_visible
    ret c
    jp @partial
@above:
    add hl,de
    ld de,0
    or a
    sbc hl,de
    jp m,@exited
    jp z,@exited
@partial:
    ld a,side_hidden
    ret
@exited:
    ld a,side_exited
    ret
