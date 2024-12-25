    align 256

; Game state variables
game_state:  dl game_main_menu

do_game:
    ld hl,(game_state)   ; Load the current game state
    CALL_HL                ; Call the corresponding state handler
    ret

; ========================
; Main Game States
; ========================

game_main_menu:
    ; Handle main menu options
    ; Example: Check input for "Start Game"
    call check_input_menu
    jp nz, game_initialize ; Start game on selection
    ret

game_initialize:
; reset the sprite table
    call table_init
    ld hl,table_base
    ld (table_pointer),hl
; initialize sprites and active tiles table
    call sprites_init
    call table_init
    call tile_table_init
; initialize the first level
    ld a,tiles_dg
    call init_tileset
; initialize player
    ld hl,(player_shields_max)
    ld (player_shields),hl
    ld (player_score_last),hl ; anything but zero so print_score will update
    ld hl,(laser_power_max)
    ld (laser_power),hl
    ld a,(player_fuel_max)
    ld (player_fuel),a
    ld hl,0
    ld (player_score),hl
    ld (player_shields_last),hl ; set to zero so draw_shields will update
    ld (laser_power_last),hl ; set to zero so draw_laser will update
    xor a
    ld (player_fuel_last),a ; set to zero so draw_fuel will update    
    ld a,3
    ld (player_ships),a
    call player_init
; set the game state to playing
    ld hl,game_playing
    ld (game_state), hl
; done initializing
    ret

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

game_paused:
    ; Pause menu logic
    call check_input_pause
    jp nz, game_playing ; Resume game
    jp z, game_main_menu ; Exit to main menu
    ret

game_level_transition:
    ; Handle level transitions
    call load_next_level
    ld hl, game_playing
    ld (game_state), hl
    ret

game_over:
    ; Display game over screen
    call display_game_over
    jp nz, game_main_menu ; Return to main menu on input
    ret

game_victory:
    ; Display victory screen
    call display_victory
    jp nz, game_main_menu ; Return to main menu on input
    ret

; ========================
; Supporting States
; ========================

game_loading:
    ; Show loading screen while preparing assets
    call load_resources
    ld hl, game_initialize
    ld (game_state), hl
    ret

game_cutscene:
    ; Play a non-interactive cutscene
    call play_cutscene
    jp nz, game_playing ; Return to gameplay after cutscene
    ret

game_inventory:
    ; Open inventory or shop
    call display_inventory
    jp nz, game_playing ; Return to gameplay after inventory close
    ret

game_map:
    ; Display map or quest
    call display_map
    jp nz, game_playing ; Return to gameplay after map close
    ret

game_exit:
    ; Handle exit logic
    call save_game
    jp exit_game         ; Safely shut down
    ret

; ========================
; Sub-state branching (Example for player state)
; ========================

player_state:  dl player_alive ; Player state variable

do_player:
    ld hl,(player_state)
    CALL_HL                ; Call the corresponding player state
    ret

player_alive:
    call player_input     ; Handle player input
    call player_move_weapons    ; Handle laser movement and collision detection
    ret

player_dead:
    call player_move_weapons    ; Handle laser movement and collision detection
    ret

do_enemies: 
    ld hl,(enemies_state)
    CALL_HL
    ret
enemies_state: dl enemies_alive

enemies_dead:
    ret

enemies_respawning:
    ret

enemies_respawn:
    ret 

enemies_alive:
    call move_enemies
    ret

check_input_menu:
    ret

update_game_world:
    ret

check_game_events:
    ret

check_input_pause:
    ret

load_next_level:
    ret

display_game_over:
    ret

display_victory:
    ret

load_resources:
    ret

play_cutscene:
    ret

display_inventory:
    ret

display_map:
    ret

save_game:
    ret

exit_game:
    ret

