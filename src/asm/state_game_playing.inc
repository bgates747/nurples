; main game state
game_playing:
; Main gameplay loop

; Level-start presentation scrolls only the blank starfield. The player actor
; publishes its bounded speed; the global renderer owns the VDP operation and
; never admits terrain through the multi-scanline path. The final entry
; speed is one, then GRACE falls through to the ordinary one-line
; terrain pipeline on the following frame.
    call player_entry_starfield_speed
    jp nc,@terrain
    call starfield_warp_scroll
    jp @tiles_ready
@terrain:
; plot tiles through the gameplay-only one-scanline path
    call tiles_plot
@tiles_ready:

; set gfx viewport to playing field window
    call set_gfx_playing_field

; process movement and collisions
    .if GAME_OVER_REVIEW_TRIGGER
    call game_over_review_should_trigger
    jp nc,@normal_player
; Publish the terminal actor state and use the real coordinator transition.
    ld hl,player_dead
    ld (player_state),hl
    xor a
    ld (player_ships),a
    call draw_player_ships
    jp game_route_terminal_player_event
@normal_player:
    .endif
    call game_update_live_world
    ret c
; Complete the last ordinary actor frame before switching phases. A death or
; respawn can delay the handoff even after the last terrain has gone.
    ld a,(tiles_level_complete)
    or a
    ret z
    call player_world_travel_enabled
    ret nc
    ld a,STATE_NEW_EVENT_LEVEL_COMPLETE
    call telemetry_post_events
    ld hl,game_level_transition
    ld (game_state),hl
    ret
; end game_playing

; Same actor ordering in normal play and the warp. Carry denotes terminal
; player outcome; the caller must not overwrite the new Game Over state.
game_update_live_world:
    call do_player
    ld a,(player_event)
    or a
    jp nz,game_route_terminal_player_event
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
    or a
    ret
; end game_update_live_world

game_route_terminal_player_event:
    xor a
    ld (player_event),a
    ld (game_level_transition_active),a
    ld a,STATE_NEW_EVENT_NO_LIVES
    call telemetry_post_events
    ld hl,game_over
    ld (game_state),hl
    scf
    ret
; end game_route_terminal_player_event

    .if GAME_OVER_REVIEW_TRIGGER
game_over_review_should_trigger:
    ld hl,(player_state)
    ld a,GAME_OVER_REVIEW_TRIGGER
    cp 1
    jp z,@immediate
    ld de,player_alive
    or a
    sbc hl,de
    jp nz,@wait
    ld iy,seekers_base
    ld b,seekers_max
@scan:
    bit 7,(iy+sprite_type) ; the sprite must already have been published
    jp nz,@next
    bit sprite_alive,(iy+sprite_collisions)
    jp z,@next
    bit sprite_just_died,(iy+sprite_collisions)
    jp nz,@next
    call seeker_within_playfield
    ret c
@next:
    lea iy,iy+table_record_size
    djnz @scan
    jp @wait
@immediate:
    ld de,player_grace
    or a
    sbc hl,de
    jp nz,@wait
    scf
    ret
@wait:
    xor a
    ret
    .endif
