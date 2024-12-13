; #### THIS IS THE SPACE ALLOCATED TO THE SPRITE TABLE ####
    align 256

table_base: equ $
player_weapons_begin: equ table_base ; blkb table_record_size*player_weapons_max,0
player_weapons_end: equ table_record_size*player_weapons_max+player_weapons_begin
player_begin: equ player_weapons_end
player_end: equ table_record_size+player_begin
enemy_sprites_begin: equ player_end
seekers_base: equ enemy_sprites_begin ; blkb table_record_size*seekers_max,0
enemy_sprites_end: equ table_record_size*seekers_max+seekers_base
table_end: equ enemy_sprites_end

table_num_records: equ table_end-table_base/table_record_size

; ### THIS IS THE SPACE ALLOCATED TO THE ACTIVE TILES TABLE ###
tile_table_base: equ table_end
tile_table_end: equ 256*tile_table_record_size+tile_table_base