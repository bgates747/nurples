; Purple Nurples replacement state-machine integration scaffold.
;
; This file remains intentionally disconnected from the legacy scheduler. It
; supplies game-specific callbacks and immutable configuration to the exact
; generic core exercised by the standalone coordinator fixture.

STATE_NEW_COUNTDOWN_FRAMES:    equ 60*6
STATE_NEW_ACTOR_CLASS_PLAYER:  equ 0
STATE_NEW_ACTOR_NUM_CLASSES:   equ 1

state_new_stage_complete:      db 0

    include "state_core.inc"
    include "state_graph.inc"

; Mark bounded NEW_GAME, LEVEL_ENTER, LEVEL_TRANSITION, or RESPAWN work done.
state_new_mark_stage_complete:
    ld a,1
    ld (state_new_stage_complete),a
    ret

; Global phase callbacks
; ---------------------------------------------------------------------------

state_new_noop:
    ret

state_new_boot_update:
    ld a,STATE_NEW_LOADING
    jp state_new_request

state_new_loading_update:
    call state_new_loading_step
    ld a,(state_new_frame_events)
    and STATE_NEW_EVENT_LOAD_COMPLETE
    ret z
    ld a,STATE_NEW_TITLE
    jp state_new_request

state_new_title_update:
    ld a,(state_new_frame_events)
    and STATE_NEW_EVENT_START
    ret z
    ld a,STATE_NEW_NEW_GAME
    jp state_new_request

state_new_stage_enter:
    xor a
    ld (state_new_stage_complete),a
    ret

state_new_new_game_update:
    call state_new_new_game_step
    ld a,(state_new_stage_complete)
    or a
    ret z
    ld a,STATE_NEW_LEVEL_ENTER
    jp state_new_request

state_new_level_enter_update:
    call state_new_level_enter_step
    ld a,(state_new_stage_complete)
    or a
    ret z
    ld a,STATE_NEW_COUNTDOWN
    jp state_new_request

state_new_countdown_update:
    ld hl,(state_new_state_frames)
    ld de,STATE_NEW_COUNTDOWN_FRAMES-1
    or a
    sbc hl,de
    ret c
    ld a,STATE_NEW_PLAYING
    jp state_new_request

; Actors post PLAYER_DEAD/NO_LIVES; they never assign RESPawn/GAME_OVER.
state_new_playing_update:
    ld a,(state_new_frame_events)
    and STATE_NEW_EVENT_VICTORY
    jp nz,@victory
    ld a,(state_new_frame_events)
    and STATE_NEW_EVENT_PLAYER_DEAD
    jp nz,@player_dead
    ld a,(state_new_frame_events)
    and STATE_NEW_EVENT_LEVEL_COMPLETE
    jp nz,@level_complete
    ld a,(state_new_frame_events)
    and STATE_NEW_EVENT_PAUSE
    ret z
    ld a,STATE_NEW_PAUSED
    jp state_new_request
@victory:
    ld a,STATE_NEW_VICTORY
    jp state_new_request
@player_dead:
    ld a,(state_new_frame_events)
    and STATE_NEW_EVENT_NO_LIVES
    jp nz,@game_over
    ld a,STATE_NEW_RESPAWN
    jp state_new_request
@game_over:
    ld a,STATE_NEW_GAME_OVER
    jp state_new_request
@level_complete:
    ld a,STATE_NEW_LEVEL_TRANSITION
    jp state_new_request

state_new_paused_update:
    ld a,(state_new_frame_events)
    and STATE_NEW_EVENT_PAUSE
    ret z
    ld a,STATE_NEW_PLAYING
    jp state_new_request

state_new_level_transition_update:
    call state_new_level_transition_step
    ld a,(state_new_stage_complete)
    or a
    ret z
    ld a,STATE_NEW_LEVEL_ENTER
    jp state_new_request

state_new_respawn_update:
    call state_new_respawn_step
    ld a,(state_new_stage_complete)
    or a
    ret z
    ld a,STATE_NEW_COUNTDOWN
    jp state_new_request

state_new_game_over_update:
    ld a,(state_new_frame_events)
    and STATE_NEW_EVENT_START
    ret z
    ld a,STATE_NEW_NEW_GAME
    jp state_new_request

state_new_victory_update:
    ld a,(state_new_frame_events)
    and STATE_NEW_EVENT_START
    ret z
    ld a,STATE_NEW_NEW_GAME
    jp state_new_request

state_new_fault_update:
    ld a,STATE_NEW_EXIT
    jp state_new_request

; Bounded lifecycle integration seams. Each callback performs at most one
; setup/reset step and returns. Loading posts LOAD_COMPLETE on its final step;
; the other final steps call state_new_mark_stage_complete. No out-of-band
; scheduler job is required.
state_new_loading_step:
    ; TODO integration: load one bounded resource batch. The final batch posts
    ; STATE_NEW_EVENT_LOAD_COMPLETE for the next coordinator tick.
    ret

state_new_new_game_step:
    ; TODO integration: advance one session-reset step.
    ret

state_new_level_enter_step:
    ; TODO integration: advance one level-construction step.
    ret

state_new_level_transition_step:
    ; TODO integration: advance one level-teardown/selection step.
    ret

state_new_respawn_step:
    ; TODO integration: advance one life-reset step.
    ret

; Every descriptor has enter/update/render/exit even when a shared no-op is
; sufficient. This removes null callback checks and arbitrary null jumps.
state_new_descriptors:
    dl state_new_noop,state_new_boot_update,state_new_noop,state_new_noop
    dl state_new_noop,state_new_loading_update,state_new_noop,state_new_noop
    dl state_new_noop,state_new_title_update,state_new_noop,state_new_noop
    dl state_new_stage_enter,state_new_new_game_update,state_new_noop,state_new_noop
    dl state_new_stage_enter,state_new_level_enter_update,state_new_noop,state_new_noop
    dl state_new_noop,state_new_countdown_update,state_new_noop,state_new_noop
    dl state_new_noop,state_new_playing_update,state_new_noop,state_new_noop
    dl state_new_noop,state_new_paused_update,state_new_noop,state_new_noop
    dl state_new_stage_enter,state_new_level_transition_update,state_new_noop,state_new_noop
    dl state_new_stage_enter,state_new_respawn_update,state_new_noop,state_new_noop
    dl state_new_noop,state_new_game_over_update,state_new_noop,state_new_noop
    dl state_new_noop,state_new_victory_update,state_new_noop,state_new_noop
    dl state_new_noop,state_new_noop,state_new_noop,state_new_noop
    dl state_new_noop,state_new_fault_update,state_new_noop,state_new_noop
state_new_descriptors_end:

; Actor controllers always reconcile, but only selected phases run local UPDATE
; or RENDER callbacks. No policy bit authorizes the coordinator to mutate an
; actor-local state.
state_new_policy_table:
    db STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_INPUT+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_ACTOR_RENDER+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_INPUT+STATE_NEW_POLICY_ACTOR_RENDER+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_INPUT+STATE_NEW_POLICY_ACTOR_UPDATE+STATE_NEW_POLICY_ACTOR_RENDER+STATE_NEW_POLICY_WORLD_UPDATE+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_INPUT+STATE_NEW_POLICY_ACTOR_RENDER+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_ACTOR_RENDER+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_ACTOR_RENDER+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_INPUT+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_INPUT+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db 0
    db STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
state_new_policy_table_end:

; Reference player actor
; ---------------------------------------------------------------------------
;
; This inert example demonstrates the intended ownership rule. It is not wired
; to player_begin. Future player collision code requests PLAYER_DYING locally;
; the dying handler posts a global event only after its nonblocking sequence is
; complete. Global RESPAWN then causes this actor's own reconcile callback to
; choose PLAYER_READY. No global handler writes this controller's state.

STATE_NEW_PLAYER_DYING_FRAMES: equ 60*2

state_new_player_actor:
    db STATE_NEW_PLAYER_INACTIVE
    db STATE_NEW_NONE
    db STATE_NEW_NONE
    db STATE_NEW_NONE
    db STATE_NEW_ACTOR_CLASS_PLAYER
    dl 0 ; owner pointer: set to player sprite/model during integration
    dl 0 ; last observed global transition serial
    dl 0 ; local state frames
    db 0 ; errors
    db 0 ; runtime flags
state_new_player_actor_end:

; Called before every player-local state handler.
state_new_player_reconcile:
    ld a,(state_new_current)
    cp STATE_NEW_PLAYING
    jp z,@playing
    cp STATE_NEW_PAUSED
    jp z,@keep
    cp STATE_NEW_NEW_GAME
    jp z,@ready
    cp STATE_NEW_LEVEL_ENTER
    jp z,@ready
    cp STATE_NEW_COUNTDOWN
    jp z,@ready
    cp STATE_NEW_LEVEL_TRANSITION
    jp z,@ready
    cp STATE_NEW_RESPAWN
    jp z,@ready
    ld a,STATE_NEW_PLAYER_INACTIVE
    ret
@playing:
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    cp STATE_NEW_PLAYER_INACTIVE
    jp z,@active
    cp STATE_NEW_PLAYER_READY
    jp z,@active
@keep:
    ld a,STATE_NEW_NONE
    ret
@ready:
    ld a,STATE_NEW_PLAYER_READY
    ret
@active:
    ld a,STATE_NEW_PLAYER_ACTIVE
    ret

state_new_player_active_update:
    ; TODO integration seam: snapshot input, express intent, and update the
    ; player model once. On lethal damage:
    ;   ld a,STATE_NEW_PLAYER_DYING
    ;   jp state_new_actor_request
    ret

state_new_player_dying_update:
    ld hl,(iy+STATE_NEW_ACTOR_STATE_FRAMES)
    ld de,STATE_NEW_PLAYER_DYING_FRAMES-1
    or a
    sbc hl,de
    ret c
    ld a,STATE_NEW_EVENT_PLAYER_DEAD
    ; OR STATE_NEW_EVENT_NO_LIVES here when the player owns no spare lives.
    call state_new_post_events
    ld a,STATE_NEW_PLAYER_DEAD
    jp state_new_actor_request

state_new_player_descriptors:
    dl state_new_noop,state_new_noop,state_new_noop,state_new_noop
    dl state_new_noop,state_new_noop,state_new_noop,state_new_noop
    dl state_new_noop,state_new_player_active_update,state_new_noop,state_new_noop
    dl state_new_noop,state_new_player_dying_update,state_new_noop,state_new_noop
    dl state_new_noop,state_new_noop,state_new_noop,state_new_noop
state_new_player_descriptors_end:

; Immutable actor-class registry. Add a class record here before any controller
; can select its callbacks; class IDs are bounds-checked on every actor tick.
state_new_actor_classes:
    db STATE_NEW_PLAYER_COUNT
    dl state_new_player_descriptors
    dl state_new_player_reconcile
state_new_actor_classes_end:

; ez80asm has no ASSERT directive. A selected undefined symbol is the supported
; fail-build idiom for immutable table/cardinality invariants.
    .if [state_new_descriptors_end-state_new_descriptors]-[STATE_NEW_COUNT*STATE_NEW_DESC_SIZE]
        dl state_new_global_descriptor_span_mismatch
    .endif
    .if [state_new_policy_table_end-state_new_policy_table]-STATE_NEW_COUNT
        dl state_new_policy_span_mismatch
    .endif
    .if [state_new_player_actor_end-state_new_player_actor]-STATE_NEW_ACTOR_SIZE
        dl state_new_player_actor_size_mismatch
    .endif
    .if [state_new_player_descriptors_end-state_new_player_descriptors]-[STATE_NEW_PLAYER_COUNT*STATE_NEW_DESC_SIZE]
        dl state_new_player_descriptor_span_mismatch
    .endif
    .if [state_new_actor_classes_end-state_new_actor_classes]-[STATE_NEW_ACTOR_NUM_CLASSES*STATE_NEW_ACTOR_CLASS_SIZE]
        dl state_new_actor_class_span_mismatch
    .endif
