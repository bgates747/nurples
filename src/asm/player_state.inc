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
player_ships: db 3 ; integer
player_speed: equ 3*256 ; pixels per frame 16.8 fixed
player_lasers_max: equ 4

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

player_dying:
; DEBUG
; decrement lives
    ld a,(player_ships)
    dec a
    jp z,game_over
    ld (player_ships),a
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
; set gfx origin and viewport to playing field window
    ld bc,origin_left
    ld de,origin_top
    call vdu_set_gfx_origin
    ld bc,field_left
    ld de,field_top
    ld ix,field_right
    ld iy,field_bottom
    call vdu_set_gfx_viewport
    call vdu_clg
; restart the level
    ld hl,cur_level
    dec (hl) ; decrement level so it will be incremented back to the same level
    call next_level
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
; END DEBUG

    ld hl,enemies_despawning
    ld (enemies_state),hl
    ret
; end player_dying

player_dead:
    call player_move_weapons
    ret
; end player_dead


kill_player:
    ld hl,player_dying
    ld (player_state),hl
    ret
; end kill_player

