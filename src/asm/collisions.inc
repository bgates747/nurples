; ====== COLLISION DETECTION ======
; --- constants ---
; sprite_collisions flags
collides_with_player: equ 0
collides_with_laser: equ 1
sprite_alive: equ 6 ; set when sprite is alive
sprite_just_died: equ 7 ; set when sprite just died

; TODO: make these bit numbers instead of masks
collide_top: equ %00001000
collide_bottom: equ %00000100
collide_left: equ %00000010
collide_right: equ %00000001
; END TODO

; this routine detects collisions between two collidable objects,
; which can include sprites, active tiles, or any screen coordinate
; which has been set up with a sprite table structure that includes dim_x
; representing the radius of the collidable object
; inputs: ix and iy point to the colliding objects' sprite table records
; outputs: uh.l = distance - sum of radii
;          zero flag set or sign flag negative if collision detected
;          dx168 and dy168 are also populated with the signed displacements
; destroys: everything except indexes
check_collision:
; compute dx = x1-x0
    ld hl,(ix+sprite_x)
    ld de,(iy+sprite_x)
    xor a ; clear carry
    sbc hl,de
    ld (dx168),hl ; dx to scratch
; compute dy = y1-y0
    ld hl,(ix+sprite_y)
    ld de,(iy+sprite_y)
    xor a ; clear carry
    sbc hl,de ; hl = dy
    ld (dy168),hl ; dy to scratch
; compute euclidean distance between centers
    call distance168 ; uh.l = distance
; check for overlap by summing half the radii
    ld a,(ix+sprite_dim_x)
    srl a ; divide by 2
    ld e,(iy+sprite_dim_x)
    srl e ; divide by 2
    add a,e
    ld de,0 ; clear deu and e
    ld d,a ; ud.e = sum of radii
    or a ; clear carry
    sbc hl,de ; uh.l = distance - sum of radii
    ret
; end check_collision
