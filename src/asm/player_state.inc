player_defaults:
    db player               ; type
    dl BUF_SHIP_0L          ; base_bufferId
    dl do_player            ; move_program
    db %01000000            ; collisions bit 6 signals sprite alive
    db 16                   ; dim_x
    db 16                   ; dim_y
    db 1                    ; num_orientations
    db 1                    ; num_animations
    db 0                    ; animation
    db 0                    ; animation_timer
    db 0                    ; move_timer
    db 0                    ; move_step
    db 0                    ; points
    db 0                    ; shield_damage
    db 0                    ; frame

player_state:  dl player_alive
player_ships: db 0
player_ships_max: db 3
player_speed: equ 3*256 ; pixels per frame 16.8 fixed

draw_player_ships:
; activate mfd
    ld ix,mfd_ships
    call mfd_activate
; blank out previously drawn ships
    xor a ; color mode 0
    ld c,c_black
    call vdu_gcol
    ld bc,0
    ld de,0
    ld ix,0+79
    ld iy,0+15
    call vdu_plot_rf
; draw the ships remaining
    ld hl,BUF_SHIP_MFD
    call vdu_buff_select
    ld a,(player_ships) ; loop counter
    ld bc,0 ; starting x-coordinate
@loop:
    dec a ; we don't count the ship we're flying
    ret z 
    push af ; back up loop counter
    push bc ; save x-coordinate
    ld de,0 ; y-coordinate
    call vdu_plot_bmp
    pop bc ; restore x-coordinate
    ld hl,16 ; bump x-coords
    add hl,bc
    push hl
    pop bc ; bc = new x-coordinate
    pop af
    jp @loop    
    ret
; end draw_player_ships

do_player:
    ld hl,(player_state)
    CALL_HL
    ret
; end do_player

; set initial player position
; inputs: none,everything is hardcoded
; outputs: player_x/y set to bottom-left corner of screen
; destroys: pretty much everything
player_init:
    ld a,(player_shields_max)
    ld (player_shields),a
    ld a,(laser_power_max)
    ld (laser_power),a
    ld a,(player_fuel_max)
    ld (player_fuel),a
; set all to zero so they will update
    xor a
    ld (laser_power_last),a
    ld (player_shields_last),a
    ld (player_fuel_last),a
; set player sprite record with defaults
    ld ix,player_begin
    ld hl,player_defaults
    ld bc,0*256
    ld de,sprite_bottom*256
    ld a,1
    call spawn_sprite_from_coords
    jp nc,@draw
    res 7,(ix+sprite_type)
; load sprite frames
    ld a,(ix+sprite_id)
    call vdu_sprite_select
    call vdu_sprite_clear_frames
    ld hl,BUF_SHIP_0L
    call vdu_sprite_add_buff
    ld hl,BUF_SHIP_1C
    call vdu_sprite_add_buff
    ld hl,BUF_SHIP_2R
    call vdu_sprite_add_buff
@draw:
    ld bc,0*256
    ld de,sprite_bottom*256
    call vdu_sprite_move_abs168
    call vdu_sprite_show
    ld hl,player_alive
    ld (player_state),hl
    set sprite_alive,(ix+sprite_collisions)
    ret
; end player_init

player_alive:
    call player_input
    call player_move_weapons
    ret
; end player_alive

kill_player:
; set player collision flag to dead
    ld ix,player_begin
    res sprite_alive,(ix+sprite_collisions)
; change seeker move program to move_enemy_seeker_orbit
    ld iy,seekers_base
    ld hl,move_enemy_seeker_orbit
    ld b,seekers_max
    call change_sprite_group_move_program
; decrement lives
    ld a,(player_ships)
    dec a
    jp z,game_over
    ld (player_ships),a
    call draw_player_ships
; set player state to dying
    ld hl,player_dying
    ld (player_state),hl
    ret
; end kill_player

player_dying:
; spawn explosions at player location
    ld b,16 ; loop counter
@loop_explode:
    push bc ; save loop counter
    call spawn_explosion_player
; move explosions for a few frames
    ld b,16 ; loop counter
@loop_animate:
    push bc
    call @animate
    pop bc
    djnz @loop_animate
    pop bc
    djnz @loop_explode
; set game state to player dead
    ld hl,player_dead
    ld (player_state),hl
    ret
@animate:
    call tiles_plot
    call set_gfx_playing_field
    call player_move_weapons
    call do_enemies
    call move_explosions
    call move_tiles
    call vdu_vblank
    ; call vdu_vblank ; DEBUG
    ; call vdu_vblank ; DEBUG
    ret
; end player_dying

get_ready: asciz "Get Ready!"

player_dead:
; reset the sprite table
    call table_init
    ld hl,table_base
    ld (table_pointer),hl
; initialize sprites and active tiles table
    call sprites_init
    call table_init
    call tile_table_init
; reset player score to what it was at the start of the level
    ld hl,(player_starting_score)
    ld (player_score),hl
; restart the level
    call start_level_countdown
    call init_level
; spawn player sprite at starting position
    call player_init
; set the game state to playing
    ld hl,game_playing
    ld (game_state), hl
; set game playing substates
    ld hl,player_alive
    ld (player_state), hl
    ld hl,enemies_alive
    ld (enemies_state), hl
    ret
; end player_dead