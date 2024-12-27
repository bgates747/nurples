    align 256
; ====== COLLISION DETECTION ======
; --- constants ---
; sprite_collisions flags
; collides_with_player: equ %00000001
; collides_with_laser: equ %00000010
sprite_alive: equ 6; set when sprite is alive
sprite_just_died: equ 7; set when sprite just died
collide_top: equ %00001000
collide_bottom: equ %00000100
collide_left: equ %00000010
collide_right: equ %00000001

collision_table_pointer: dl 0x000000 ; pointer to the sprite in the group being checked for collision

collision_draw_hitboxes:
; draw hitbox for ix collider
    push ix
    push iy
    xor a ; color mode 0
    ld c,c_red
    call vdu_gcol
    ld bc,0
    ld c,(ix+sprite_x+1)
    ld b,(ix+sprite_x+2)
    ld de,0
    ld e,(ix+sprite_y+1)
    ld d,(ix+sprite_y+2)
    ld hl,0 ; clear hlu and h
    ld l,(ix+sprite_dim_x)
    add hl,bc ; hl = x1
    push hl ; save x1
    ld hl,0 ; clear hlu and h
    ld l,(ix+sprite_dim_y)
    add hl,de ; hl = y1
    pop ix ; ix = x1 (was hl)
    push hl
    pop iy ; iy = y1
    call vdu_plot_rf
    pop iy
    pop ix

; draw hitbox for iy collider
    push ix
    push iy
    xor a ; color mode 0
    ld c,c_green
    call vdu_gcol
    ld bc,0
    ld c,(iy+sprite_x+1)
    ld b,(iy+sprite_x+2)
    ld de,0
    ld e,(iy+sprite_y+1)
    ld d,(iy+sprite_y+2)
    ld hl,0 ; clear hlu and h
    ld l,(iy+sprite_dim_x)
    add hl,bc ; hl = x1
    push hl ; save x1
    ld hl,0 ; clear hlu and h
    ld l,(iy+sprite_dim_y)
    add hl,de ; hl = y1
    pop ix ; ix = x1 (was hl)
    push hl
    pop iy ; iy = y1
    call vdu_plot_rf
    pop iy
    pop ix
    ret
; end collision_draw_hitboxes

; check_collision:
; Detects collision using bounding box method.
; Inputs:
;   IX -> Sprite/Tile table for Collider A
;   IY -> Sprite/Tile table for Collider B
; Outputs:
;   a = 0, zero flag set, positive sign, carry reset -> Collision detected
;   a = -1, zero flag reset, negative sign, carry set -> No collision detected
; Destroys: HL, DE, AF
check_collision:
check_collision_box:
; Test 1: (ay + ah) - by < 0
    ld hl,0                     ; Clear hlu
    ld l,(ix+sprite_y+1)        ; Load ay integer part low byte
    ld h,(ix+sprite_y+2)        ; Load ay integer part high byte
    ld de,0                     ; Clear deu and d
    ld e,(ix+sprite_dim_y)      ; Load ah
    add hl,de                   ; (ay + ah)
    ld e,(iy+sprite_y+1)        ; Load by integer part low byte
    ld d,(iy+sprite_y+2)        ; Load by integer part high byte
    xor a                       ; Clear carry
    sbc hl,de                   ; (ay + ah) - by
    jp m,@not_collision         ; No collision if negative

; Test 2: ay - (by + bh) < 0
    ld hl,0                     ; Clear hlu
    ld l,(iy+sprite_y+1)        ; Load by integer part low byte
    ld h,(iy+sprite_y+2)        ; Load by integer part high byte
    ld de,0                     ; Clear deu and d
    ld e,(iy+sprite_dim_y)      ; Load bh
    add hl,de                   ; (by + bh)
    ld e,(ix+sprite_y+1)        ; Load ay integer part low byte
    ld d,(ix+sprite_y+2)        ; Load ay integer part high byte
    xor a                       ; Clear carry
    sbc hl,de                   ; (by + bh) - ay
    jp m,@not_collision         ; No collision if negative

; Test 3: (ax + aw) - bx < 0
    ld hl,0                     ; Clear hlu
    ld l,(ix+sprite_x+1)        ; Load ax integer part low byte
    ld h,(ix+sprite_x+2)        ; Load ax integer part high byte
    ld de,0                     ; Clear deu and d
    ld e,(ix+sprite_dim_x)      ; Load aw
    add hl,de                   ; (ax + aw)
    ld e,(iy+sprite_x+1)        ; Load bx integer part low byte
    ld d,(iy+sprite_x+2)        ; Load bx integer part high byte
    xor a                       ; Clear carry
    sbc hl,de                   ; (ax + aw) - bx
    jp m,@not_collision         ; No collision if negative

; Test 4: ax - (bx + bw) < 0
    ld hl,0                     ; Clear hlu
    ld l,(iy+sprite_x+1)        ; Load bx integer part low byte
    ld h,(iy+sprite_x+2)        ; Load bx integer part high byte
    ld de,0                     ; Clear deu and d
    ld e,(iy+sprite_dim_x)      ; Load bw
    add hl,de                   ; (bx + bw)
    ld e,(ix+sprite_x+1)        ; Load ax integer part low byte
    ld d,(ix+sprite_x+2)        ; Load ax integer part high byte
    xor a                       ; Clear carry
    sbc hl,de                   ; (bx + bw) - ax
    jp m,@not_collision         ; No collision if negative

; If all tests pass, collision is detected
@is_collision:
    xor a                       ; Clear accumulator and flags (a = 0)
    ret                         ; Return with zero flag set

; No collision detected
@not_collision:
    dec a                       ; Set a to -1
    ret                         ; Return with carry set
; End check_collision_box


; this routine detects collisions between two collidable objects,
; which can include sprites, active tiles, or any screen coordinate
; which has been set up with a sprite table structure that includes dim_x
; representing the radius of the collidable object
; inputs: ix and iy point to the colliding objects' sprite/tile table records
; outputs: uh.l = distance - sum of radii
;          zero flag set if collision detected
;          dx168 and dy168 are also populated with the signed displacements
; destroys: everything except indexes
; check_collision:
check_collision_cartesian:
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
    ret z ; return collision
    ret p ; return no collision
    xor a ; return collision
    ret
; end check_collision_cartesian

; check collisions of a sprite or tile against a sprite group
; inputs: iy = pointer to sprite or tile record being checked
;         ix = pointer to sprite group, b = number of records in sprite group
check_sprite_group_collisions:
@collision_loop:
    ld (collision_table_pointer),ix ; update table pointer
    push bc ; backup loop counter
; check sprite_collisions sprite_alive flag to see if sprite is active
    bit sprite_alive,(ix+sprite_collisions)
    jp z,@next_record ; not alive so skip to next record
; sprite alive so check for collision
    call check_collision
    jp nz,@next_record ; no collision
; set sprite_just_died flag for each entity and return to caller
    pop bc ; dummy pop to balance stack
    set sprite_just_died,(ix+sprite_collisions)
    set sprite_just_died,(iy+sprite_collisions)
    ret
@next_record:
    pop bc ; get back our loop counter
    lea ix,ix+table_record_size
    djnz @collision_loop ; loop until we've checked all the records
; return to caller
    xor a
    inc a ; resets sign flag and a to non-zero indicating no collision
    ret
; end check_sprite_group_collisions

check_enemy_collision_laser: ; check for collisions with laser
    ld ix,player_lasers_begin
    ld b,player_lasers_max
    call check_sprite_group_collisions
    ret
; end check_enemy_collision_laser

check_enemy_collision_player: ; check for collisions with player
    ld ix,player_begin
    call check_collision
    ret nz ; no collision so nothing to do
    set sprite_just_died,(ix+sprite_collisions)
    set sprite_just_died,(iy+sprite_collisions)
    xor a ; signal collision detected
    ret
; end check_enemy_collision_player