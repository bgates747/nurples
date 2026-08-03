; Purple Nurples state-machine scaffold
;
; This file is intentionally not included by nurples.asm. It is a standalone,
; namespaced migration target for the legacy state.inc/player_state.inc design.
; It requires ADL mode and has no dependency on the project CALL_HL macro.
;
; Scheduler contract
; ------------------
; 1. Snapshot input and post edge-triggered events.
; 2. Call state_new_tick once. It updates one global phase and commits at most
;    one requested global transition.
; 3. Tick every actor controller. Each actor consults the global phase through
;    its own reconcile callback before its local handler can run. The global
;    machine never assigns an actor-local state.
; 4. Update remaining world systems when state_new_policy permits it.
; 5. Render the global phase and actors, then present/wait for VBlank once.
;
; Handler contract
; ----------------
; - ENTER, UPDATE, RENDER, and EXIT callbacks are bounded and return.
; - No callback waits for input, calls VBlank, or performs a multi-frame loop.
; - UPDATE callbacks request a transition and return. They never write current
;   state or jump directly into another state handler.
; - Global callbacks may destroy all registers. Actor callbacks receive IY as
;   the actor-controller pointer; the dispatcher restores IY after callbacks.
; - All stored code pointers are 24-bit `dl` values and all IDs are bytes.
; - The including translation unit must include state_ids.inc first.

; ---------------------------------------------------------------------------
; Global game phases
; ---------------------------------------------------------------------------

; One outer-loop tick is expected per rendered frame. State age is the number
; of completed updates before the current callback, so a duration of N expires
; when age reaches N-1.
STATE_NEW_COUNTDOWN_FRAMES:    equ 60*6

; Events and scheduler policy bits are defined in state_ids.inc.

; Dispatcher phases restrict when a transition may be requested.
STATE_NEW_PHASE_IDLE:          equ 0
STATE_NEW_PHASE_ENTER:         equ 1
STATE_NEW_PHASE_UPDATE:        equ 2
STATE_NEW_PHASE_RENDER:        equ 3
STATE_NEW_PHASE_EXIT:          equ 4

; Latched global error bits.
STATE_NEW_ERROR_INVALID_TARGET:equ 0
STATE_NEW_ERROR_ILLEGAL_EDGE:  equ 1
STATE_NEW_ERROR_CONFLICT:      equ 2
STATE_NEW_ERROR_BAD_PHASE:     equ 3
STATE_NEW_ERROR_INVALID_STATE: equ 4
STATE_NEW_ERROR_REENTRANT:     equ 5
STATE_NEW_ERROR_UNINITIALIZED: equ 6

; Descriptor fields are mandatory 24-bit callbacks.
STATE_NEW_DESC_ENTER:          equ 0
STATE_NEW_DESC_UPDATE:         equ 3
STATE_NEW_DESC_RENDER:         equ 6
STATE_NEW_DESC_EXIT:           equ 9
STATE_NEW_DESC_SIZE:           equ 12

state_new_current:             db STATE_NEW_NONE
state_new_previous:            db STATE_NEW_NONE
state_new_pending:             db STATE_NEW_NONE
state_new_candidate:           db STATE_NEW_NONE
state_new_phase:               db STATE_NEW_PHASE_IDLE
state_new_policy:              db 0
state_new_initialized:         db 0
state_new_in_tick:             db 0
state_new_errors:              db 0
state_new_posted_events:       db 0
state_new_frame_events:        db 0
state_new_last_transition_events: db 0
state_new_stage_complete:      db 0
state_new_state_frames:        dl 0
state_new_total_frames:        dl 0
state_new_transition_serial:   dl 0

; A real CALL followed by this tail jump supplies the handler's return address
; without consuming BC as the legacy CALL_HL macro does.
state_new_call_hl:
    jp (hl)

; Initialize the coordinator in BOOT. The first tick advances to LOADING.
; Inputs: none
; Destroys: af, hl, de, ix and anything destroyed by the BOOT enter callback
state_new_init:
    xor a
    ld (state_new_policy),a
    ld (state_new_errors),a
    ld (state_new_posted_events),a
    ld (state_new_frame_events),a
    ld (state_new_last_transition_events),a
    ld (state_new_stage_complete),a
    ld (state_new_in_tick),a
    ld hl,0
    ld (state_new_state_frames),hl
    ld (state_new_total_frames),hl
    ld (state_new_transition_serial),hl
    ld a,STATE_NEW_NONE
    ld (state_new_previous),a
    ld (state_new_pending),a
    ld (state_new_candidate),a
    ld a,STATE_NEW_BOOT
    ld (state_new_current),a
    call state_new_load_policy
    ld a,1
    ld (state_new_initialized),a
    ld a,STATE_NEW_PHASE_ENTER
    ld (state_new_phase),a
    ld a,STATE_NEW_BOOT
    call state_new_call_enter
    xor a
    ld (state_new_phase),a
    ret

; OR event bits from A into the pending event latch.
; Safe for input code, actors, and other subsystems between global ticks.
state_new_post_events:
    ld hl,state_new_posted_events
    or (hl)
    ld (hl),a
    ret

; Mark bounded NEW_GAME, LEVEL_ENTER, LEVEL_TRANSITION, or RESPAWN work done.
state_new_mark_stage_complete:
    ld a,1
    ld (state_new_stage_complete),a
    ret

; Queue a global transition. First request wins; a duplicate is harmless.
; Inputs: a = destination state ID
; Returns: carry set if accepted/duplicate, carry clear if rejected
; Destroys: af, bc, hl
state_new_request:
    ld c,a
    cp STATE_NEW_COUNT
    jp nc,state_new_error_invalid_target
    ld a,(state_new_phase)
    cp STATE_NEW_PHASE_UPDATE
    jp nz,state_new_error_bad_phase
    ld a,(state_new_pending)
    cp STATE_NEW_NONE
    jp z,@no_pending
    cp c
    jp z,@accepted
    jp state_new_error_conflict
@no_pending:
    ld a,(state_new_current)
    cp c
    jp z,@accepted
    ld b,a
    call state_new_transition_allowed
    jp nc,state_new_error_illegal_edge
    ld a,c
    ld (state_new_pending),a
@accepted:
    scf
    ret

; Explicit global transition graph. Inputs: b=current, c=destination.
; Returns carry set only for a legal edge.
state_new_transition_allowed:
    ld a,c
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
    cp STATE_NEW_FAULT
    jp z,@from_fault
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

; Run one bounded global update and commit at most one transition.
state_new_tick:
    ld a,(state_new_initialized)
    or a
    jp z,state_new_error_uninitialized
    ld a,(state_new_in_tick)
    or a
    jp nz,state_new_error_reentrant
    inc a
    ld (state_new_in_tick),a
; Snapshot posted events. Events posted later remain queued for the next tick.
    ld a,(state_new_posted_events)
    ld (state_new_frame_events),a
    xor a
    ld (state_new_posted_events),a
; Validate before indexing the descriptor table.
    ld a,(state_new_current)
    cp STATE_NEW_COUNT
    jp nc,@invalid_current
    ld a,STATE_NEW_PHASE_UPDATE
    ld (state_new_phase),a
; QUIT is a coordinator-wide priority event, independent of local phase logic.
    ld a,(state_new_frame_events)
    and STATE_NEW_EVENT_QUIT
    jp z,@update
    ld a,STATE_NEW_EXIT
    call state_new_request
    jp @after_update
@update:
    ld a,(state_new_current)
    call state_new_call_update
@after_update:
    ld hl,(state_new_state_frames)
    inc hl
    ld (state_new_state_frames),hl
    ld hl,(state_new_total_frames)
    inc hl
    ld (state_new_total_frames),hl
    call state_new_apply_pending
    xor a
    ld (state_new_phase),a
    ld (state_new_in_tick),a
    scf
    ret
@invalid_current:
    ld hl,state_new_errors
    set STATE_NEW_ERROR_INVALID_STATE,(hl)
    call state_new_force_fault
    xor a
    ld (state_new_phase),a
    ld (state_new_in_tick),a
    or a
    ret

; Render the current global phase after actor/world updates.
state_new_render:
    ld a,(state_new_initialized)
    or a
    jp z,state_new_error_uninitialized
    ld a,(state_new_current)
    cp STATE_NEW_COUNT
    jp nc,@invalid_current
    ld a,(state_new_policy)
    and STATE_NEW_POLICY_STATE_RENDER
    ret z
    ld a,STATE_NEW_PHASE_RENDER
    ld (state_new_phase),a
    ld a,(state_new_current)
    call state_new_call_render
    xor a
    ld (state_new_phase),a
    ret
@invalid_current:
    ld hl,state_new_errors
    set STATE_NEW_ERROR_INVALID_STATE,(hl)
    call state_new_force_fault
    xor a
    ld (state_new_phase),a
    or a
    ret

; Returns Z when the coordinator has reached its terminal EXIT phase.
state_new_is_terminal:
    ld a,(state_new_current)
    cp STATE_NEW_EXIT
    ret

; Commit pending as EXIT(old) -> swap/reset/publish -> ENTER(new).
state_new_apply_pending:
    ld a,(state_new_pending)
    cp STATE_NEW_NONE
    ret z
    cp STATE_NEW_COUNT
    jp c,@target_in_range
    ld hl,state_new_errors
    set STATE_NEW_ERROR_INVALID_TARGET,(hl)
    ld a,STATE_NEW_NONE
    ld (state_new_pending),a
    jp state_new_force_fault
@target_in_range:
    ld c,a
    ld a,(state_new_current)
    cp STATE_NEW_COUNT
    jp nc,@invalid_current
    cp c
    jp z,@discard_same
    ld b,a
    call state_new_transition_allowed
    jp c,@valid
    ld hl,state_new_errors
    set STATE_NEW_ERROR_ILLEGAL_EDGE,(hl)
    ld a,STATE_NEW_NONE
    ld (state_new_pending),a
    jp state_new_force_fault
@invalid_current:
    ld hl,state_new_errors
    set STATE_NEW_ERROR_INVALID_STATE,(hl)
    ld a,STATE_NEW_NONE
    ld (state_new_pending),a
    jp state_new_force_fault
@discard_same:
    ld a,STATE_NEW_NONE
    ld (state_new_pending),a
    ret
@valid:
    ld a,c
    ld (state_new_candidate),a
    ld a,STATE_NEW_NONE
    ld (state_new_pending),a
    ld a,STATE_NEW_PHASE_EXIT
    ld (state_new_phase),a
    ld a,(state_new_current)
    ld (state_new_previous),a
    push af ; preserve expected old ID across arbitrary EXIT callback
    ld a,(state_new_candidate)
    push af ; preserve expected next ID across arbitrary EXIT callback
    ld a,(state_new_current)
    call state_new_call_exit
    pop af
    ld c,a ; expected next ID
    pop af
    ld b,a ; expected old ID
; Revalidate immediately after EXIT before indexing policy/callback tables.
    ld a,(state_new_current)
    cp b
    jp nz,@exit_corrupt_current
    cp STATE_NEW_COUNT
    jp nc,@exit_corrupt_current
    ld a,(state_new_candidate)
    cp c
    jp nz,@exit_corrupt_target
    cp STATE_NEW_COUNT
    jp nc,@exit_corrupt_target
    call state_new_transition_allowed
    jp nc,@exit_corrupt_edge
    ld a,b
    ld (state_new_previous),a
    ld a,c
    ld (state_new_current),a
    ld a,(state_new_frame_events)
    ld (state_new_last_transition_events),a
    ld hl,0
    ld (state_new_state_frames),hl
    ld hl,(state_new_transition_serial)
    inc hl
    ld (state_new_transition_serial),hl
    ld a,(state_new_current)
    call state_new_load_policy
    ld a,STATE_NEW_PHASE_ENTER
    ld (state_new_phase),a
    ld a,(state_new_current)
    call state_new_call_enter
    ret
@exit_corrupt_current:
    ld hl,state_new_errors
    set STATE_NEW_ERROR_INVALID_STATE,(hl)
    jp state_new_force_fault
@exit_corrupt_target:
    ld hl,state_new_errors
    set STATE_NEW_ERROR_INVALID_TARGET,(hl)
    jp state_new_force_fault
@exit_corrupt_edge:
    ld hl,state_new_errors
    set STATE_NEW_ERROR_ILLEGAL_EDGE,(hl)
    jp state_new_force_fault

; Recover from a corrupt global ID without dereferencing it.
state_new_force_fault:
    ld a,(state_new_current)
    ld (state_new_previous),a
    ld a,STATE_NEW_NONE
    ld (state_new_pending),a
    ld (state_new_candidate),a
    ld a,STATE_NEW_FAULT
    ld (state_new_current),a
    ld hl,0
    ld (state_new_state_frames),hl
    ld hl,(state_new_transition_serial)
    inc hl
    ld (state_new_transition_serial),hl
    ld a,STATE_NEW_FAULT
    call state_new_load_policy
    ld a,STATE_NEW_PHASE_ENTER
    ld (state_new_phase),a
    ld a,STATE_NEW_FAULT
    call state_new_call_enter
    ret

; Inputs: a = valid state ID. Publishes its scheduler/actor policy.
state_new_load_policy:
    ld hl,state_new_policy_table
    ld de,0
    ld e,a
    add hl,de
    ld a,(hl)
    ld (state_new_policy),a
    ret

; Inputs: a = valid state ID, c = descriptor callback offset.
state_new_call_callback:
    ld de,0
    ld d,a
    ld e,STATE_NEW_DESC_SIZE
    mlt de
    ld ix,state_new_descriptors
    add ix,de
    ld de,0
    ld e,c
    add ix,de
    ld hl,(ix)
    jp state_new_call_hl

state_new_call_enter:
    ld c,STATE_NEW_DESC_ENTER
    jp state_new_call_callback

state_new_call_update:
    ld c,STATE_NEW_DESC_UPDATE
    jp state_new_call_callback

state_new_call_render:
    ld c,STATE_NEW_DESC_RENDER
    jp state_new_call_callback

state_new_call_exit:
    ld c,STATE_NEW_DESC_EXIT
    jp state_new_call_callback

state_new_error_invalid_target:
    ld hl,state_new_errors
    set STATE_NEW_ERROR_INVALID_TARGET,(hl)
    or a
    ret

state_new_error_illegal_edge:
    ld hl,state_new_errors
    set STATE_NEW_ERROR_ILLEGAL_EDGE,(hl)
    or a
    ret

state_new_error_conflict:
    ld hl,state_new_errors
    set STATE_NEW_ERROR_CONFLICT,(hl)
    or a
    ret

state_new_error_bad_phase:
    ld hl,state_new_errors
    set STATE_NEW_ERROR_BAD_PHASE,(hl)
    or a
    ret

state_new_error_invalid_state:
    ld hl,state_new_errors
    set STATE_NEW_ERROR_INVALID_STATE,(hl)
    or a
    ret

state_new_error_reentrant:
    ld hl,state_new_errors
    set STATE_NEW_ERROR_REENTRANT,(hl)
    or a
    ret

state_new_error_uninitialized:
    ld hl,state_new_errors
    set STATE_NEW_ERROR_UNINITIALIZED,(hl)
    or a
    ret

; ---------------------------------------------------------------------------
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

; ---------------------------------------------------------------------------
; Actor-owned mini state machines
; ---------------------------------------------------------------------------
;
; An actor controller is separate from its sprite/tile data. OWNER is a 24-bit
; pointer to that data. A validated class ID selects an immutable class record
; containing the descriptor table and RECONCILE callback; writable actor data
; never contains a code pointer. RECONCILE runs before every local handler and
; returns either:
;   A = STATE_NEW_NONE       keep the current local state
;   A = valid local state ID adjust this actor to the global phase
;
; The dispatcher performs the returned adjustment. The global coordinator and
; master scheduler never write ACTOR_CURRENT or ACTOR_PENDING.

STATE_NEW_ACTOR_CURRENT:       equ 0
STATE_NEW_ACTOR_PREVIOUS:      equ 1
STATE_NEW_ACTOR_PENDING:       equ 2
STATE_NEW_ACTOR_CANDIDATE:     equ 3
STATE_NEW_ACTOR_CLASS:         equ 4
STATE_NEW_ACTOR_OWNER:         equ 5  ; dl
STATE_NEW_ACTOR_GAME_SERIAL:   equ 8  ; dl
STATE_NEW_ACTOR_STATE_FRAMES:  equ 11 ; dl
STATE_NEW_ACTOR_ERRORS:        equ 14
STATE_NEW_ACTOR_RUNTIME:       equ 15
STATE_NEW_ACTOR_SIZE:          equ 16

STATE_NEW_ACTOR_CLASS_COUNT:   equ 0
STATE_NEW_ACTOR_CLASS_DESCRIPTORS: equ 1 ; dl
STATE_NEW_ACTOR_CLASS_RECONCILE: equ 4   ; dl
STATE_NEW_ACTOR_CLASS_SIZE:    equ 7

STATE_NEW_ACTOR_CLASS_PLAYER:  equ 0
STATE_NEW_ACTOR_NUM_CLASSES:   equ 1

STATE_NEW_ACTOR_RUNTIME_UPDATE:equ 0

STATE_NEW_ACTOR_ERROR_CONFIG:  equ 0
STATE_NEW_ACTOR_ERROR_STATE:   equ 1
STATE_NEW_ACTOR_ERROR_TARGET:  equ 2
STATE_NEW_ACTOR_ERROR_CONFLICT:equ 3
STATE_NEW_ACTOR_ERROR_PHASE:   equ 4

; Resolve a writable class ID to an immutable class record.
; Inputs: iy = actor controller
; Returns: carry set and ix = class record, or carry clear with error latched
state_new_actor_get_class:
    ld a,(iy+STATE_NEW_ACTOR_CLASS)
    cp STATE_NEW_ACTOR_NUM_CLASSES
    jp nc,state_new_actor_error_config
    ld de,0
    ld d,a
    ld e,STATE_NEW_ACTOR_CLASS_SIZE
    mlt de
    ld ix,state_new_actor_classes
    add ix,de
    scf
    ret

; Initialize a configured actor controller.
; Inputs: iy = controller, a = initial local state ID
state_new_actor_init:
    ld b,a
    xor a
    ld (iy+STATE_NEW_ACTOR_ERRORS),a
    ld (iy+STATE_NEW_ACTOR_RUNTIME),a
    ld a,STATE_NEW_NONE
    ld (iy+STATE_NEW_ACTOR_PREVIOUS),a
    ld (iy+STATE_NEW_ACTOR_PENDING),a
    ld (iy+STATE_NEW_ACTOR_CANDIDATE),a
    call state_new_actor_get_class
    ret nc
    ld a,(ix+STATE_NEW_ACTOR_CLASS_COUNT)
    or a
    jp z,state_new_actor_error_config
    ld a,b
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp nc,@invalid_initial
    ld (iy+STATE_NEW_ACTOR_CURRENT),a
    jp @finish
@invalid_initial:
    set STATE_NEW_ACTOR_ERROR_TARGET,(iy+STATE_NEW_ACTOR_ERRORS)
    xor a ; every actor table must reserve state zero as safe/inactive
    ld (iy+STATE_NEW_ACTOR_CURRENT),a
@finish:
    ld hl,0
    ld (iy+STATE_NEW_ACTOR_STATE_FRAMES),hl
; Let the actor reconcile its initial choice before the first ENTER callback.
    push iy
    call state_new_actor_call_reconcile
    pop iy
    cp STATE_NEW_NONE
    jp z,@enter
    ld b,a
    call state_new_actor_get_class
    ret nc
    ld a,b
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp c,@use_reconciled
    set STATE_NEW_ACTOR_ERROR_TARGET,(iy+STATE_NEW_ACTOR_ERRORS)
    xor a ; recover initial selection to the required safe state zero
@use_reconciled:
    ld (iy+STATE_NEW_ACTOR_CURRENT),a
@enter:
    call state_new_actor_ack_game
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    push iy
    call state_new_actor_call_enter
    pop iy
    scf
    ret

; Run the actor-owned global-phase reconciliation callback. Carry set means the
; actor changed/recovered (or configuration failed), so UPDATE must be skipped
; for this outer frame. Carry clear means its current local state remains valid.
state_new_actor_reconcile:
    call state_new_actor_get_class
    jp nc,@stop
    ld a,(ix+STATE_NEW_ACTOR_CLASS_COUNT)
    or a
    jp z,@bad_config
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp nc,@bad_current
; Actor-owned reconciliation against state_new_current happens every tick.
    push iy
    call state_new_actor_call_reconcile
    pop iy
    cp STATE_NEW_NONE
    jp z,@reconciled
    ld b,a
    call state_new_actor_get_class
    jp nc,@stop
    ld a,b
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp nc,@bad_target
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    cp b
    jp z,@reconciled
    ld a,b
    call state_new_actor_transition_to
    call state_new_actor_ack_game
    scf
    ret ; ENTER happened; local UPDATE begins on the next outer frame
@reconciled:
    call state_new_actor_ack_game
    or a
    ret
@bad_config:
    call state_new_actor_error_config
@stop:
    scf
    ret
@bad_current:
    call state_new_actor_recover
    scf
    ret
@bad_target:
    call state_new_actor_recover_target
    scf
    ret

; Actor preflight and one bounded local update.
; RECONCILE always runs first and is the actor's ownership boundary.
; Inputs: iy = configured actor controller
state_new_actor_tick:
    call state_new_actor_reconcile
    ret c
    ld a,(state_new_policy)
    and STATE_NEW_POLICY_ACTOR_UPDATE
    ret z
    set STATE_NEW_ACTOR_RUNTIME_UPDATE,(iy+STATE_NEW_ACTOR_RUNTIME)
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    push iy
    call state_new_actor_call_update
    pop iy
    res STATE_NEW_ACTOR_RUNTIME_UPDATE,(iy+STATE_NEW_ACTOR_RUNTIME)
    ld hl,(iy+STATE_NEW_ACTOR_STATE_FRAMES)
    inc hl
    ld (iy+STATE_NEW_ACTOR_STATE_FRAMES),hl
    call state_new_actor_apply_pending
    ret

; Render one actor without exposing its state to the master scheduler.
state_new_actor_render:
    call state_new_actor_reconcile
    ld a,(state_new_policy)
    and STATE_NEW_POLICY_ACTOR_RENDER
    ret z
    call state_new_actor_get_class
    ret nc
    ld a,(ix+STATE_NEW_ACTOR_CLASS_COUNT)
    or a
    jp z,state_new_actor_error_config
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp nc,state_new_actor_recover
    push iy
    call state_new_actor_call_render
    pop iy
    ret

; Queue a local transition from the actor's UPDATE callback.
; Inputs: iy = actor controller, a = destination local state
; Returns: carry set if accepted/duplicate, clear if rejected
state_new_actor_request:
    ld c,a
    call state_new_actor_get_class
    ret nc
    ld a,c
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp nc,state_new_actor_error_target
    bit STATE_NEW_ACTOR_RUNTIME_UPDATE,(iy+STATE_NEW_ACTOR_RUNTIME)
    jp z,state_new_actor_error_phase
    ld a,(iy+STATE_NEW_ACTOR_PENDING)
    cp STATE_NEW_NONE
    jp z,@no_pending
    cp c
    jp z,@accepted
    jp state_new_actor_error_conflict
@no_pending:
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    cp c
    jp z,@accepted
    ld a,c
    ld (iy+STATE_NEW_ACTOR_PENDING),a
@accepted:
    scf
    ret

state_new_actor_apply_pending:
    ld a,(iy+STATE_NEW_ACTOR_PENDING)
    cp STATE_NEW_NONE
    ret z
    ld b,a
    call state_new_actor_get_class
    jp nc,@discard
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp nc,state_new_actor_recover
    ld a,b
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp c,@valid
    set STATE_NEW_ACTOR_ERROR_TARGET,(iy+STATE_NEW_ACTOR_ERRORS)
    jp state_new_actor_recover
@valid:
    ld a,b
    jp state_new_actor_transition_to
@discard:
    ld a,STATE_NEW_NONE
    ld (iy+STATE_NEW_ACTOR_PENDING),a
    or a
    ret

; Inputs: iy = actor controller, a = validated destination local state.
state_new_actor_transition_to:
    ld (iy+STATE_NEW_ACTOR_CANDIDATE),a
    ld a,STATE_NEW_NONE
    ld (iy+STATE_NEW_ACTOR_PENDING),a
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    ld (iy+STATE_NEW_ACTOR_PREVIOUS),a
    ld a,(iy+STATE_NEW_ACTOR_CLASS)
    push af ; expected actor class
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    push af ; expected old local state
    ld a,(iy+STATE_NEW_ACTOR_CANDIDATE)
    push af ; expected next local state
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    push iy
    call state_new_actor_call_exit
    pop iy
    pop af
    ld c,a ; expected next local state
    pop af
    ld b,a ; expected old local state
    pop af
    cp (iy+STATE_NEW_ACTOR_CLASS)
    jp nz,state_new_actor_quarantine
    call state_new_actor_get_class
    jp nc,state_new_actor_quarantine
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    cp b
    jp nz,state_new_actor_recover
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp nc,state_new_actor_recover
    ld a,(iy+STATE_NEW_ACTOR_CANDIDATE)
    cp c
    jp nz,state_new_actor_recover_target
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp nc,state_new_actor_recover_target
    ld a,b
    ld (iy+STATE_NEW_ACTOR_PREVIOUS),a
    ld a,c
    ld (iy+STATE_NEW_ACTOR_CURRENT),a
    ld hl,0
    ld (iy+STATE_NEW_ACTOR_STATE_FRAMES),hl
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    push iy
    call state_new_actor_call_enter
    pop iy
    ret

; A corrupted class ID cannot safely resolve even state zero. Quarantine the
; actor without another indirect call; later ticks keep latching CONFIG.
state_new_actor_quarantine:
    set STATE_NEW_ACTOR_ERROR_CONFIG,(iy+STATE_NEW_ACTOR_ERRORS)
    xor a
    ld (iy+STATE_NEW_ACTOR_CURRENT),a
    ld a,STATE_NEW_NONE
    ld (iy+STATE_NEW_ACTOR_PENDING),a
    ld (iy+STATE_NEW_ACTOR_CANDIDATE),a
    ld hl,0
    ld (iy+STATE_NEW_ACTOR_STATE_FRAMES),hl
    or a
    ret

; Invalid local state recovery never dereferences the corrupt ID. State zero is
; a required safe/inactive state in every actor descriptor table.
state_new_actor_recover:
    set STATE_NEW_ACTOR_ERROR_STATE,(iy+STATE_NEW_ACTOR_ERRORS)
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    ld (iy+STATE_NEW_ACTOR_PREVIOUS),a
    xor a
    ld (iy+STATE_NEW_ACTOR_CURRENT),a
    ld a,STATE_NEW_NONE
    ld (iy+STATE_NEW_ACTOR_PENDING),a
    ld hl,0
    ld (iy+STATE_NEW_ACTOR_STATE_FRAMES),hl
    call state_new_actor_ack_game
    xor a
    push iy
    call state_new_actor_call_enter
    pop iy
    or a
    ret

state_new_actor_recover_target:
    set STATE_NEW_ACTOR_ERROR_TARGET,(iy+STATE_NEW_ACTOR_ERRORS)
    jp state_new_actor_recover

state_new_actor_ack_game:
    ld hl,(state_new_transition_serial)
    ld (iy+STATE_NEW_ACTOR_GAME_SERIAL),hl
    ret

; Inputs: iy = actor controller, a = valid local state, c = callback offset.
state_new_actor_call_callback:
    ld b,a
    call state_new_actor_get_class
    ret nc
    ld ix,(ix+STATE_NEW_ACTOR_CLASS_DESCRIPTORS)
    ld de,0
    ld d,b
    ld e,STATE_NEW_DESC_SIZE
    mlt de
    add ix,de
    ld de,0
    ld e,c
    add ix,de
    ld hl,(ix)
    jp state_new_call_hl

state_new_actor_call_enter:
    ld c,STATE_NEW_DESC_ENTER
    jp state_new_actor_call_callback

state_new_actor_call_update:
    ld c,STATE_NEW_DESC_UPDATE
    jp state_new_actor_call_callback

state_new_actor_call_render:
    ld c,STATE_NEW_DESC_RENDER
    jp state_new_actor_call_callback

state_new_actor_call_exit:
    ld c,STATE_NEW_DESC_EXIT
    jp state_new_actor_call_callback

state_new_actor_call_reconcile:
    call state_new_actor_get_class
    ret nc
    ld hl,(ix+STATE_NEW_ACTOR_CLASS_RECONCILE)
    jp state_new_call_hl

state_new_actor_error_config:
    set STATE_NEW_ACTOR_ERROR_CONFIG,(iy+STATE_NEW_ACTOR_ERRORS)
    or a
    ret

state_new_actor_error_target:
    set STATE_NEW_ACTOR_ERROR_TARGET,(iy+STATE_NEW_ACTOR_ERRORS)
    or a
    ret

state_new_actor_error_conflict:
    set STATE_NEW_ACTOR_ERROR_CONFLICT,(iy+STATE_NEW_ACTOR_ERRORS)
    or a
    ret

state_new_actor_error_phase:
    set STATE_NEW_ACTOR_ERROR_PHASE,(iy+STATE_NEW_ACTOR_ERRORS)
    or a
    ret

; ---------------------------------------------------------------------------
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
