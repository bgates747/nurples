enemies_state: dl enemies_alive

do_enemies: 
    ld hl,(enemies_state)
    CALL_HL
    ret

enemies_alive:
    ld iy,enemy_sprites_begin
    ld b,table_num_enemy_records
    call move_sprite_group
    ret

enemies_despawning:
    ret

enemies_dead:
    ret

enemies_respawning:
    ret