; Purple Nurples canonical legal global-transition graph.
; The including translation unit must include state_ids.inc first.

; Explicit global transition graph. Inputs: b=current, c=destination.
; Returns carry set only for a legal edge.
state_new_transition_allowed:
; Validate the source before applying coordinator-wide EXIT/FAULT shortcuts.
; EXIT is terminal and has no outgoing edge. FAULT may only recover to EXIT;
; same-state requests are accepted earlier as harmless request-layer no-ops.
    ld a,b
    cp STATE_NEW_COUNT
    jp nc,@denied
    cp STATE_NEW_EXIT
    jp z,@denied
    cp STATE_NEW_FAULT
    jp z,@from_fault
    ld a,c
    cp STATE_NEW_COUNT
    jp nc,@denied
    cp STATE_NEW_EXIT
    jp z,@allowed
    cp STATE_NEW_FAULT
    jp z,@allowed
    ld a,b
    cp STATE_NEW_BOOT
    jp z,@from_boot
    cp STATE_NEW_LOADING
    jp z,@from_loading
    cp STATE_NEW_TITLE
    jp z,@from_title
    cp STATE_NEW_NEW_GAME
    jp z,@from_new_game
    cp STATE_NEW_LEVEL_ENTER
    jp z,@from_level_enter
    cp STATE_NEW_COUNTDOWN
    jp z,@from_countdown
    cp STATE_NEW_PLAYING
    jp z,@from_playing
    cp STATE_NEW_PAUSED
    jp z,@from_paused
    cp STATE_NEW_LEVEL_TRANSITION
    jp z,@from_level_transition
    cp STATE_NEW_RESPAWN
    jp z,@from_respawn
    cp STATE_NEW_GAME_OVER
    jp z,@from_game_over
    cp STATE_NEW_VICTORY
    jp z,@from_victory
    jp @denied
@from_boot:
    ld a,c
    cp STATE_NEW_LOADING
    jp z,@allowed
    jp @denied
@from_loading:
    ld a,c
    cp STATE_NEW_TITLE
    jp z,@allowed
    jp @denied
@from_title:
    ld a,c
    cp STATE_NEW_NEW_GAME
    jp z,@allowed
    jp @denied
@from_new_game:
    ld a,c
    cp STATE_NEW_LEVEL_ENTER
    jp z,@allowed
    jp @denied
@from_level_enter:
    ld a,c
    cp STATE_NEW_COUNTDOWN
    jp z,@allowed
    jp @denied
@from_countdown:
    ld a,c
    cp STATE_NEW_PLAYING
    jp z,@allowed
    jp @denied
@from_playing:
    ld a,c
    cp STATE_NEW_PAUSED
    jp z,@allowed
    cp STATE_NEW_LEVEL_TRANSITION
    jp z,@allowed
    cp STATE_NEW_RESPAWN
    jp z,@allowed
    cp STATE_NEW_GAME_OVER
    jp z,@allowed
    cp STATE_NEW_VICTORY
    jp z,@allowed
    jp @denied
@from_paused:
    ld a,c
    cp STATE_NEW_PLAYING
    jp z,@allowed
    cp STATE_NEW_TITLE
    jp z,@allowed
    jp @denied
@from_level_transition:
    ld a,c
    cp STATE_NEW_LEVEL_ENTER
    jp z,@allowed
    cp STATE_NEW_GAME_OVER
    jp z,@allowed
    cp STATE_NEW_VICTORY
    jp z,@allowed
    jp @denied
@from_respawn:
    ld a,c
    cp STATE_NEW_COUNTDOWN
    jp z,@allowed
    cp STATE_NEW_GAME_OVER
    jp z,@allowed
    jp @denied
@from_game_over:
    ld a,c
    cp STATE_NEW_NEW_GAME
    jp z,@allowed
    cp STATE_NEW_TITLE
    jp z,@allowed
    jp @denied
@from_victory:
    ld a,c
    cp STATE_NEW_NEW_GAME
    jp z,@allowed
    cp STATE_NEW_TITLE
    jp z,@allowed
    jp @denied
@from_fault:
    ld a,c
    cp STATE_NEW_EXIT
    jp z,@allowed
@denied:
    or a
    ret
@allowed:
    scf
    ret
