; main game state
game_playing:
; Main gameplay loop

; Level-start presentation scrolls only the blank starfield. The player actor
; publishes its bounded speed; the global renderer owns the VDP operation and
; never admits terrain through the multi-scanline path. The final seven entry
; frames run at speed one, then GRACE falls through to the ordinary one-line
; terrain pipeline on the following frame.
    call player_entry_starfield_speed
    jp nc,@terrain
    call starfield_warp_scroll
    jp @tiles_ready
@terrain:
; plot tiles through the gameplay-only one-scanline path
    call tiles_plot
    or a
    jp z,@tiles_ready
; level completion is a world result; only the global state selects the phase
    ld a,STATE_NEW_EVENT_LEVEL_COMPLETE
    call telemetry_post_events
    ld hl,game_level_transition
    ld (game_state),hl
    ret
@tiles_ready:

; set gfx viewport to playing field window
    call set_gfx_playing_field

; process movement and collisions
    call do_player
    ld a,(player_event)
    or a
    jp z,@player_ready
; consume the actor outcome before any more work runs in the old playing frame
    xor a
    ld (player_event),a
    ld a,STATE_NEW_EVENT_NO_LIVES
    call telemetry_post_events
    ld hl,game_over
    ld (game_state),hl
    ret
@player_ready:
    call do_enemies
    call move_explosions
    call move_tiles

; update UI
    ld bc,0
    ld de,0
    call vdu_set_gfx_origin
    ; call print_score
    call draw_shields
    call draw_laser
    call draw_fuel
    ret
