; gets the next available sprite id for a particular sprite group
; inputs; ix = pointer to sprite group, b = number of records in sprite group
; returns: if new sprite available, a = sprite id, ix pointing to new sprite vars, carry set
;      otherwise, a = 0, carry flag reset, ix pointing to highest sprite vars
; destroys: a,b,c,hl,ix
; affects: bumps table_active_sprites by one
table_get_next_id_new:
    ld c,(ix+sprite_id) ; sprite id counter
@loop:
    ld a,(ix+sprite_type)
    and a
    jr z,@found
    lea ix,ix+table_record_size
    inc c
    djnz @loop
@notfound:
    xor a ; a = 0 and reset carry flag indicating that we didn't find a free sprite
    ret
@found:
; bump number of active sprites
    ld hl,table_active_sprites
    inc (hl)
; return sprite id
    ld a,c
    scf ; sets carry flag indicating we found a free sprite
    ret ; done
; end table_get_next_id_new

; create a new sprite at the same location as an existing sprite
; inputs: hl = pointer to new sprite default settings, table_pointer = existing sprite
;         ix = pointer to sprite group, b = number of records in sprite group
; affects: bumps table_active_sprites by one
spawn_sprite_from_sprite_new: 
    push hl ; back up pointer to new sprite defaults
    call table_get_next_id_new ; if available: a = new sprite id, ix = pointer to new sprite vars
    pop hl ; restore pointer to new sprite defaults
    ret nc ; no carry means no free sprite slots, so we go home
; copy default settings to new sprite
    push ix ; back up pointer to new sprite record
    lea ix,ix+sprite_definitions ; bump pointer to copy to
    push ix ; pointer to default sprite settings
    pop de ; address to copy to
    ld bc,table_record_size-sprite_definitions ; number of bytes to copy
    ldir ; copies defaults to new sprite record
    pop ix ; restore pointer to new sprite record
; copy sprite position, velocity and orientation from existing sprite
    ld iy,(table_pointer) ; get existing sprite record
    ld hl,(iy+sprite_x)
    ld (ix+sprite_x),hl
    ld hl,(iy+sprite_y)
    ld (ix+sprite_y),hl
; return to caller
    set 7,(ix+sprite_type) ; set just spawned flag
    scf ; set carry flag to indicate success
    ret
; end spawn_sprite_from_sprite_new

; change an existing sprite to a new type, keeping its position and velocity the same
; inputs: hl = pointer to new sprite default settings, table_pointer = existing sprite
;         ix = pointer to sprite group, b = number of records in sprite group
alter_sprite_new:
    push hl ; back up pointer to new sprite defaults
    call table_get_next_id_new ; if available: a = new sprite id, ix = pointer to new sprite vars
    dec (hl) ; decrement table_active_sprites by one since we're not actually creating a new sprite
    pop hl ; restore pointer to new sprite defaults
    ret nc ; no carry means no free sprite slots, so we go home
; copy default settings to new sprite
    push ix ; back up pointer to new sprite record
    lea ix,ix+sprite_definitions ; bump pointer to copy to
    push ix ; pointer to default sprite settings
    pop de ; address to copy to
    ld bc,table_record_size-sprite_definitions ; number of bytes to copy
    ldir ; copies defaults to new sprite record
    pop ix ; restore pointer to new sprite record
; copy sprite position, velocity and orientation from existing sprite
    ld iy,(table_pointer) ; get existing sprite record
    ld hl,(iy+sprite_x)
    ld (ix+sprite_x),hl
    ld hl,(iy+sprite_y)
    ld (ix+sprite_y),hl
; return to caller
    scf ; set carry flag to indicate success
; end alter_sprite_new