; Game state variables
game_state:  dl game_main_menu
game_fault_bad_state: equ 0
game_fault_flags: db 0 ; latched fail-closed diagnostics
game_exit_requested: db 0 ; main-loop handoff; state handlers must return normally

do_game:
    ld hl,(game_state)   ; Load the current game state
    call game_state_pointer_valid
    jp c,@dispatch
    ld hl,game_fault_flags
    set game_fault_bad_state,(hl)
    ld hl,game_init ; publish recovery; do not run it in the corrupt frame
    ld (game_state),hl
    ret
@dispatch:
    CALL_HL                ; Call the corresponding state handler
    ret

; Exact stabilization whitelist for writable legacy global-state pointers.
; inputs/outputs: hl = candidate handler; carry set only for an exact match
; destroys: a,de
game_state_pointer_valid:
    ld de,game_main_menu
    call @matches
    jp z,@valid
    ld de,game_paused
    call @matches
    jp z,@valid
    ld de,game_level_transition
    call @matches
    jp z,@valid
    ld de,game_over
    call @matches
    jp z,@valid
    ld de,game_victory
    call @matches
    jp z,@valid
    ld de,game_loading
    call @matches
    jp z,@valid
    ld de,game_cutscene
    call @matches
    jp z,@valid
    ld de,game_inventory
    call @matches
    jp z,@valid
    ld de,game_map
    call @matches
    jp z,@valid
    ld de,game_exit
    call @matches
    jp z,@valid
    ld de,game_init
    call @matches
    jp z,@valid
    ld de,game_playing
    call @matches
    jp z,@valid
    xor a
    ret
@valid:
    scf
    ret
@matches:
    push hl
    or a
    sbc hl,de
    pop hl
    ret
; end game_state_pointer_valid

; ========================
; Main Game States
; ========================

game_main_menu:
    ; Unimplemented legacy state: remain here deterministically.
    ret



game_paused:
    ; Unimplemented legacy state: remain here deterministically.
    ret

game_level_transition:
    call next_level
    ld hl, game_playing
    ld (game_state), hl
    ret

game_over:
; set text colors
    ld a,16+128 ; darkest blue
    call vdu_colour_text
    ld a,58 ; dark orange
    call vdu_colour_text
; print game over and prompt
    ld c,27 ; x
    ld b,16 ; y
    call vdu_move_cursor
    call printInline
    asciz "GAME OVER!"
    ld c,19 ; x
    ld b,17 ; y
    call vdu_move_cursor
    call printInline
    asciz "Press any key to continue."
    ld a,%00100000 ; wait just a tick before polling for input
    call multiPurposeDelay
    call waitKeypress
; initialize a new game
    ld hl,game_init
    ld (game_state),hl
    ret

game_victory:
    ; Unimplemented legacy state: remain here deterministically.
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
    ; Unimplemented legacy state: remain here deterministically.
    ret

game_inventory:
    ; Unimplemented legacy state: remain here deterministically.
    ret

game_map:
    ; Unimplemented legacy state: remain here deterministically.
    ret

game_exit:
    ; Request orderly shutdown after returning through the state dispatcher.
    call save_game
    ld a,1
    ld (game_exit_requested),a
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
