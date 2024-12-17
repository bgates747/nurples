move_weapons:
; initialize pointers and loop counter
    ld iy,player_weapons_begin ; set iy to first record in table
    ld b,player_weapons_max ; loop counter
@move_loop:
    ld (table_pointer),iy ; update table pointer
    push bc ; backup loop counter
; check top bit of sprite_type to see if sprite is just spawned
    ld a,(iy+sprite_type)
    bit 7,a
    jp nz,@next_record ; if just spawned, skip to next record
; check sprite_type to see if sprite is active
    and a ; if zero, sprite is dead 
    jp z,@next_record ; ... skip to next record
; otherwise move the sprite
    ld hl,(iy+sprite_move_program) ; load the behavior subroutine address
    CALL_HL ; run the behavior subroutine
    ld a,(iy+sprite_id) ; get spriteId
    call vdu_sprite_select ; select sprite 
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    call vdu_sprite_move_abs168
; fall through to @next_record
@next_record:
    lea iy,iy+table_record_size
    xor a ; clears carry flag
    ld (sprite_screen_edge),a ; clear screen edge collision flag
    pop bc ; get back our loop counter
    djnz @move_loop ; loop until we've checked all the records
; reset all just spawned flags
    ld iy,player_weapons_begin
    ld b,player_weapons_max ; loop counter
@reset_just_spawned:
    push bc ; back up loop counter
    bit 7,(iy+sprite_type) ; check if just spawned
    jp z,@F
    res 7,(iy+sprite_type) ; clear just spawned flag
    ld a,(iy+sprite_id)
    call vdu_sprite_select
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    call vdu_sprite_move_abs168
    call vdu_sprite_show
@@:
    lea iy,iy+table_record_size
    pop bc ; get back loop counter
    djnz @reset_just_spawned
    ret
; end move_weapons