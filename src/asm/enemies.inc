move_enemies:
; initialize pointers and loop counter
    ld iy,enemy_sprites_begin ; set iy to first record in table
    ld b,table_num_enemy_records ; loop counter
    call move_sprite_group
    ret
; end move_enemies