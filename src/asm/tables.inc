; #### THIS IS THE SPACE ALLOCATED TO THE SPRITE TABLE ####
table_base: equ $
player_weapons_begin: equ table_base
player_lasers_begin: equ player_weapons_begin
player_lasers_end: equ table_record_size*player_lasers_max+player_lasers_begin
player_lasers_end_sprite_id: equ [player_lasers_end-table_base]/table_record_size
player_weapons_end: equ table_record_size*player_lasers_max+player_weapons_begin
player_begin: equ player_weapons_end
player_end: equ table_record_size+player_begin
enemy_sprites_begin: equ player_end
seekers_base: equ enemy_sprites_begin
seeker_min_sprite_id: equ [seekers_base-table_base]/table_record_size
seekers_end: equ table_record_size*seekers_max+seekers_base
seekers_end_sprite_id: equ [seekers_end-table_base]/table_record_size
fireballs_base: equ seekers_end
fireballs_end: equ table_record_size*fireballs_max+fireballs_base
fireballs_end_sprite_id: equ [fireballs_end-table_base]/table_record_size
enemy_sprites_end: equ fireballs_end
explosions_base: equ enemy_sprites_end
explosions_end: equ table_record_size*explosions_max+explosions_base
explosions_end_sprite_id: equ [explosions_end-table_base]/table_record_size
table_end: equ explosions_end

table_num_records: equ [table_end-table_base]/table_record_size
table_num_enemy_records: equ [enemy_sprites_end-enemy_sprites_begin]/table_record_size

; ### THIS IS THE SPACE ALLOCATED TO THE ACTIVE TILES TABLE ###
tile_table_base: equ table_end
tile_table_end: equ 256*tile_table_record_size+tile_table_base
