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
    jp nz, game_init ; Start game on selection
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
    ld hl, game_init
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

