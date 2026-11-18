; Game state variables
game_state:  dl game_main_menu
game_fault_bad_state: equ 0
game_fault_presentation: equ 1
game_fault_session_init: equ 2
game_fault_flags: db 0 ; latched fail-closed diagnostics
game_exit_requested: db 0 ; main-loop handoff; state handlers must return normally
game_level_transition_active: db 0

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
    ld de,game_attract
    call @matches
    jp z,@valid
    ld de,game_start_config
    call @matches
    jp z,@valid
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
    ld a,(game_level_transition_active)
    or a
    jp nz,@step
; Publish the non-hostile actor phase before any transition-world update.
    ld iy,player_begin
    call player_victory_init
    ld hl,player_victory
    ld (player_state),hl
    call seeker_group_enter_orbit
    call level_transition_init
    ld a,1
    ld (game_level_transition_active),a
@step:
    call level_transition_step
    jp c,@complete
    ld a,(level_transition_frame_policy)
    and LEVEL_TRANSITION_POLICY_TERRAIN
    jp z,@starfield
; The existing gameplay-only path remains exactly one line per frame. With the
; level-complete latch set it scrolls pixels without reading another map row.
    call tiles_plot
    jp @world
@starfield:
    ld a,(level_transition_frame_speed)
    call starfield_warp_scroll
@world:
    call do_player
    call do_enemies
    call move_explosions
    call move_tiles
    ld bc,0
    ld de,0
    call vdu_set_gfx_origin
    call draw_shields
    call draw_laser
    call draw_fuel
    ret
@complete:
    xor a
    ld (game_level_transition_active),a
; The loop has settled at centre and the warp has returned to speed one.
; Admit the next level without clearing the resident world or score.
    call next_level
    ld iy,player_begin
    set sprite_alive,(iy+sprite_collisions)
    ld hl,player_alive
    ld (player_state),hl
    call seeker_group_enter_seek
    ld hl,game_playing
    ld (game_state),hl
    ret

game_over:
; Retain the established global pointer while replacing its blocking legacy
; body with the bounded hardware-overlay lifecycle.
    jp game_over_bounded

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
