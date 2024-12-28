; main game state
game_playing:
; Main gameplay loop

; plot tiles
    call tiles_plot

; set gfx viewport to playing field window
    ld bc,field_left
    ld de,field_top
    ld ix,field_right
    ld iy,field_bottom
    call vdu_set_gfx_viewport

; process movement and collisions
    call do_player
    call do_enemies
    call move_explosions
    call move_tiles

; update UI
    ld bc,0
    ld de,0
    call vdu_set_gfx_origin
    call print_score
    call draw_shields
    call draw_laser
    call draw_fuel
    ret

