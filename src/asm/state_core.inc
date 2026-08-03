; Purple Nurples generic numeric-ID state coordinator
;
; This file contains only the reusable coordinator and actor-controller
; primitives. The including translation unit owns descriptor, policy, and
; actor-class tables. It requires ADL mode and has no dependency on the project
; CALL_HL macro.
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
; - The including translation unit must include state_ids.inc first, define
;   STATE_NEW_ACTOR_NUM_CLASSES, and provide the named configuration tables.

; ---------------------------------------------------------------------------
; Global game phases
; ---------------------------------------------------------------------------

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
STATE_NEW_ERROR_CONFIG:        equ 7

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
state_new_state_frames:        dl 0
state_new_total_frames:        dl 0
state_new_transition_serial:   dl 0
state_new_callback_expected:   db STATE_NEW_NONE
state_new_actor_dispatch_busy: db 0
state_new_actor_expected_class: db STATE_NEW_NONE
state_new_actor_expected_current: db STATE_NEW_NONE
state_new_actor_callback_result: db STATE_NEW_NONE

; A real CALL followed by this tail jump supplies the handler's return address
; without consuming BC as the legacy CALL_HL macro does.
state_new_call_hl:
    jp (hl)

; Reject malformed immutable configuration before the first table lookup.
; The fixture additionally enforces each actor descriptor span at assembly and
; host-test time because per-class descriptor ends are not part of this ABI.
state_new_validate_config:
    ld hl,state_new_descriptors_end-state_new_descriptors
    ld de,STATE_NEW_COUNT*STATE_NEW_DESC_SIZE
    or a
    sbc hl,de
    jp nz,state_new_error_config
    ld hl,state_new_policy_table_end-state_new_policy_table
    ld de,STATE_NEW_COUNT
    or a
    sbc hl,de
    jp nz,state_new_error_config
    ld hl,state_new_actor_classes_end-state_new_actor_classes
    ld de,STATE_NEW_ACTOR_NUM_CLASSES*STATE_NEW_ACTOR_CLASS_SIZE
    or a
    sbc hl,de
    jp nz,state_new_error_config
    scf
    ret

; Bootstrap entry for a fresh process invocation. MOS can retain writable RAM
; across RUN, so this entry deliberately clears stale dispatcher guards before
; using the guarded public initializer.
state_new_process_init:
    xor a
    ld (state_new_in_tick),a
    ld (state_new_initialized),a
    ld (state_new_actor_dispatch_busy),a
    jp state_new_init

; Initialize the coordinator in BOOT. The first tick advances to LOADING.
; Inputs: none
; Destroys: af, hl, de, ix and anything destroyed by the BOOT enter callback
state_new_init:
    ld a,(state_new_in_tick)
    or a
    jp nz,state_new_error_reentrant
    ld a,(state_new_actor_dispatch_busy)
    or a
    jp nz,state_new_error_reentrant
    xor a
    ld (state_new_initialized),a
    ld (state_new_errors),a
    call state_new_validate_config
    ret nc
    xor a
    ld (state_new_policy),a
    ld (state_new_posted_events),a
    ld (state_new_frame_events),a
    ld (state_new_last_transition_events),a
    ld (state_new_in_tick),a
    ld hl,0
    ld (state_new_state_frames),hl
    ld (state_new_total_frames),hl
    ld (state_new_transition_serial),hl
    ld a,STATE_NEW_NONE
    ld (state_new_previous),a
    ld (state_new_pending),a
    ld (state_new_candidate),a
    ld (state_new_callback_expected),a
    ld (state_new_actor_expected_class),a
    ld (state_new_actor_expected_current),a
    ld (state_new_actor_callback_result),a
    ld a,STATE_NEW_BOOT
    ld (state_new_current),a
    call state_new_load_policy
    ld a,1
    ld (state_new_initialized),a
    ld (state_new_in_tick),a
    ld a,STATE_NEW_PHASE_ENTER
    ld (state_new_phase),a
    ld a,STATE_NEW_BOOT
    ld (state_new_callback_expected),a
    call state_new_call_enter
    call state_new_validate_callback_current
    jp nc,@callback_fault
@complete:
    xor a
    ld (state_new_phase),a
    ld (state_new_in_tick),a
    scf
    ret
@callback_fault:
    xor a
    ld (state_new_phase),a
    ld (state_new_in_tick),a
    or a
    ret

; OR event bits from A into the pending event latch.
; Safe for input code, actors, and other subsystems between global ticks.
state_new_post_events:
    ld hl,state_new_posted_events
    or (hl)
    ld (hl),a
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

; Run one bounded global update and commit at most one transition.
state_new_tick:
    ld a,(state_new_initialized)
    or a
    jp z,state_new_error_uninitialized
    ld a,(state_new_in_tick)
    or a
    jp nz,state_new_error_reentrant
    ld a,(state_new_actor_dispatch_busy)
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
    ld (state_new_callback_expected),a
    call state_new_call_update
    call state_new_validate_callback_current
    jp nc,@callback_fault
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
@callback_fault:
    xor a
    ld (state_new_phase),a
    ld (state_new_in_tick),a
    or a
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
    ld a,(state_new_in_tick)
    or a
    jp nz,state_new_error_reentrant
    ld a,(state_new_actor_dispatch_busy)
    or a
    jp nz,state_new_error_reentrant
    inc a
    ld (state_new_in_tick),a
    ld a,(state_new_current)
    cp STATE_NEW_COUNT
    jp nc,@invalid_current
    ld a,(state_new_policy)
    and STATE_NEW_POLICY_STATE_RENDER
    jp z,@complete
    ld a,STATE_NEW_PHASE_RENDER
    ld (state_new_phase),a
    ld a,(state_new_current)
    ld (state_new_callback_expected),a
    call state_new_call_render
    call state_new_validate_callback_current
    jp nc,@callback_fault
@complete:
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
@callback_fault:
    xor a
    ld (state_new_phase),a
    ld (state_new_in_tick),a
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
    ld (state_new_candidate),a
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
    ld a,STATE_NEW_NONE
    ld (state_new_candidate),a
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
    ld (state_new_callback_expected),a
    call state_new_call_enter
    call state_new_validate_callback_current
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
    ld (state_new_callback_expected),a
    call state_new_call_enter
; FAULT is the final containment callback. Do not invoke it recursively if it
; violates the state-write contract; pin the coordinator back to FAULT.
    ld a,(state_new_callback_expected)
    ld b,a
    ld a,STATE_NEW_NONE
    ld (state_new_callback_expected),a
    ld a,(state_new_current)
    cp b
    ret z
    ld hl,state_new_errors
    set STATE_NEW_ERROR_INVALID_STATE,(hl)
    ld a,STATE_NEW_FAULT
    ld (state_new_current),a
    ld a,STATE_NEW_NONE
    ld (state_new_pending),a
    ld (state_new_candidate),a
    ret

; Verify that a callback obeyed the no-direct-state-write contract before a
; later table lookup can consume its result. Carry clear means recovery entered
; FAULT; the caller owns its surrounding frame-accounting policy.
state_new_validate_callback_current:
    ld a,(state_new_callback_expected)
    ld b,a
    ld a,STATE_NEW_NONE
    ld (state_new_callback_expected),a
    ld a,(state_new_current)
    cp b
    jp nz,@invalid
    cp STATE_NEW_COUNT
    jp nc,@invalid
    scf
    ret
@invalid:
    ld hl,state_new_errors
    set STATE_NEW_ERROR_INVALID_STATE,(hl)
    call state_new_force_fault
    or a
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

state_new_error_config:
    ld hl,state_new_errors
    set STATE_NEW_ERROR_CONFIG,(hl)
    or a
    ret

; ---------------------------------------------------------------------------

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

STATE_NEW_ACTOR_RUNTIME_UPDATE:equ 0
STATE_NEW_ACTOR_RUNTIME_BUSY:  equ 1

STATE_NEW_ACTOR_ERROR_CONFIG:  equ 0
STATE_NEW_ACTOR_ERROR_STATE:   equ 1
STATE_NEW_ACTOR_ERROR_TARGET:  equ 2
STATE_NEW_ACTOR_ERROR_CONFLICT:equ 3
STATE_NEW_ACTOR_ERROR_PHASE:   equ 4
STATE_NEW_ACTOR_ERROR_REENTRANT: equ 5

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

; Save and validate the writable dispatch identity around arbitrary actor
; callbacks. A callback may destroy registers but may not rewrite its class or
; current local state behind the dispatcher's back.
state_new_actor_save_expected:
    ld a,(iy+STATE_NEW_ACTOR_CLASS)
    ld (state_new_actor_expected_class),a
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    ld (state_new_actor_expected_current),a
    ret

state_new_actor_validate_expected:
    ld a,(state_new_actor_expected_class)
    cp (iy+STATE_NEW_ACTOR_CLASS)
    jp nz,@bad_class
    call state_new_actor_get_class
    jp nc,@bad_class
    ld a,(ix+STATE_NEW_ACTOR_CLASS_COUNT)
    or a
    jp z,@bad_class
    ld a,(state_new_actor_expected_current)
    ld b,a
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    cp b
    jp nz,@bad_state
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp nc,@bad_state
    scf
    ret
@bad_class:
    set STATE_NEW_ACTOR_ERROR_CONFIG,(iy+STATE_NEW_ACTOR_ERRORS)
    or a
    ret
@bad_state:
    set STATE_NEW_ACTOR_ERROR_STATE,(iy+STATE_NEW_ACTOR_ERRORS)
    or a
    ret

state_new_actor_begin_dispatch:
    ld a,(state_new_in_tick)
    or a
    jp nz,state_new_actor_error_reentrant
    ld a,(state_new_actor_dispatch_busy)
    or a
    jp nz,state_new_actor_error_reentrant
    inc a
    ld (state_new_actor_dispatch_busy),a
    res STATE_NEW_ACTOR_RUNTIME_UPDATE,(iy+STATE_NEW_ACTOR_RUNTIME)
    set STATE_NEW_ACTOR_RUNTIME_BUSY,(iy+STATE_NEW_ACTOR_RUNTIME)
    scf
    ret

state_new_actor_end_dispatch:
    res STATE_NEW_ACTOR_RUNTIME_UPDATE,(iy+STATE_NEW_ACTOR_RUNTIME)
    res STATE_NEW_ACTOR_RUNTIME_BUSY,(iy+STATE_NEW_ACTOR_RUNTIME)
    xor a
    ld (state_new_actor_dispatch_busy),a
    ld a,STATE_NEW_NONE
    ld (state_new_actor_expected_class),a
    ld (state_new_actor_expected_current),a
    ld (state_new_actor_callback_result),a
    ret

; Initialize a configured actor controller.
; Inputs: iy = controller, a = initial local state ID
state_new_actor_init:
    ld b,a
; A process bootstrap clears the shared guard. Reject an init attempted from an
; actor callback before touching the target controller.
    ld a,(state_new_actor_dispatch_busy)
    or a
    jp nz,state_new_actor_error_reentrant
    ld a,(state_new_in_tick)
    or a
    jp nz,state_new_actor_error_reentrant
    xor a
    ld (iy+STATE_NEW_ACTOR_ERRORS),a
    ld (iy+STATE_NEW_ACTOR_RUNTIME),a
    ld (iy+STATE_NEW_ACTOR_CURRENT),a
    ld hl,0
    ld (iy+STATE_NEW_ACTOR_STATE_FRAMES),hl
    ld a,STATE_NEW_NONE
    ld (iy+STATE_NEW_ACTOR_PREVIOUS),a
    ld (iy+STATE_NEW_ACTOR_PENDING),a
    ld (iy+STATE_NEW_ACTOR_CANDIDATE),a
    call state_new_actor_ack_game
    call state_new_actor_begin_dispatch
    jp nc,@failed
    call state_new_actor_get_class
    jp nc,@quarantine
    ld a,(ix+STATE_NEW_ACTOR_CLASS_COUNT)
    or a
    jp z,@quarantine
    ld a,b
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp c,@initial_valid
    set STATE_NEW_ACTOR_ERROR_TARGET,(iy+STATE_NEW_ACTOR_ERRORS)
    xor a ; every actor table must reserve state zero as safe/inactive
@initial_valid:
    ld (iy+STATE_NEW_ACTOR_CURRENT),a
; Let the actor reconcile its initial choice before the first ENTER callback.
    call state_new_actor_save_expected
    push iy
    call state_new_actor_call_reconcile
    pop iy
    ld (state_new_actor_callback_result),a
    call state_new_actor_validate_expected
    jp nc,@recover_context
    ld a,(state_new_actor_callback_result)
    cp STATE_NEW_NONE
    jp z,@enter
    ld b,a
    call state_new_actor_get_class
    jp nc,@quarantine
    ld a,b
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp c,@use_reconciled
    set STATE_NEW_ACTOR_ERROR_TARGET,(iy+STATE_NEW_ACTOR_ERRORS)
    xor a
@use_reconciled:
    ld (iy+STATE_NEW_ACTOR_CURRENT),a
@enter:
    call state_new_actor_ack_game
    call state_new_actor_save_expected
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    push iy
    call state_new_actor_call_enter
    pop iy
    call state_new_actor_validate_expected
    jp nc,@recover_after_enter
    call state_new_actor_end_dispatch
    scf
    ret
@recover_context:
; Reconciliation already ran, so fail closed to state zero before ENTER.
    ld a,(state_new_actor_expected_class)
    cp (iy+STATE_NEW_ACTOR_CLASS)
    jp nz,@quarantine
    xor a
    ld (iy+STATE_NEW_ACTOR_CURRENT),a
    ld a,STATE_NEW_NONE
    ld (iy+STATE_NEW_ACTOR_PENDING),a
    ld (iy+STATE_NEW_ACTOR_CANDIDATE),a
    ld hl,0
    ld (iy+STATE_NEW_ACTOR_STATE_FRAMES),hl
    jp @enter
@recover_after_enter:
; A corrupt ENTER callback is allowed one bounded recovery through the normal
; safe-state path; that path never dereferences the corrupted local ID.
    ld a,(state_new_actor_expected_class)
    cp (iy+STATE_NEW_ACTOR_CLASS)
    jp nz,@quarantine
    call state_new_actor_recover
    call state_new_actor_end_dispatch
    or a
    ret
@quarantine:
    call state_new_actor_quarantine
@failed:
    call state_new_actor_end_dispatch
    or a
    ret
state_new_actor_reconcile:
    call state_new_actor_get_class
    jp nc,@quarantine
    ld a,(ix+STATE_NEW_ACTOR_CLASS_COUNT)
    or a
    jp z,@bad_config
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp nc,@bad_current
; Actor-owned reconciliation against state_new_current happens every tick.
    call state_new_actor_save_expected
    push iy
    call state_new_actor_call_reconcile
    pop iy
    ld (state_new_actor_callback_result),a
    call state_new_actor_validate_expected
    jp nc,@bad_callback
    ld a,(state_new_actor_callback_result)
    cp STATE_NEW_NONE
    jp z,@reconciled
    ld b,a
    call state_new_actor_get_class
    jp nc,@quarantine
    ld a,b
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp nc,@bad_target
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    cp b
    jp z,@reconciled
    ld a,b
    call state_new_actor_ack_game
    ld a,b
    call state_new_actor_transition_to
    scf
    ret ; ENTER happened; local UPDATE begins on the next outer frame
@reconciled:
    call state_new_actor_ack_game
    or a
    ret
@bad_config:
    call state_new_actor_quarantine
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
@bad_callback:
    ld a,(state_new_actor_expected_class)
    cp (iy+STATE_NEW_ACTOR_CLASS)
    jp nz,@quarantine
    call state_new_actor_recover
    scf
    ret
@quarantine:
    call state_new_actor_quarantine
    scf
    ret

; Actor preflight and one bounded local update.
; RECONCILE always runs first and is the actor's ownership boundary.
; Inputs: iy = configured actor controller
state_new_actor_tick:
    call state_new_actor_begin_dispatch
    ret nc
    call state_new_actor_reconcile
    jp c,@complete
    ld a,(state_new_policy)
    and STATE_NEW_POLICY_ACTOR_UPDATE
    jp z,@complete
    set STATE_NEW_ACTOR_RUNTIME_UPDATE,(iy+STATE_NEW_ACTOR_RUNTIME)
    call state_new_actor_save_expected
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    push iy
    call state_new_actor_call_update
    pop iy
    res STATE_NEW_ACTOR_RUNTIME_UPDATE,(iy+STATE_NEW_ACTOR_RUNTIME)
    call state_new_actor_validate_expected
    jp nc,@callback_fault
    ld hl,(iy+STATE_NEW_ACTOR_STATE_FRAMES)
    inc hl
    ld (iy+STATE_NEW_ACTOR_STATE_FRAMES),hl
    call state_new_actor_apply_pending
@complete:
    call state_new_actor_end_dispatch
    scf
    ret
@callback_fault:
    ld a,(state_new_actor_expected_class)
    cp (iy+STATE_NEW_ACTOR_CLASS)
    jp nz,@quarantine
    call state_new_actor_recover
    jp @complete
@quarantine:
    call state_new_actor_quarantine
    jp @complete

; Render one actor without exposing its state to the master scheduler. A
; reconciliation-driven transition may render the newly entered state in this
; same render call; UPDATE still waits for the next outer frame.
state_new_actor_render:
    call state_new_actor_begin_dispatch
    ret nc
    call state_new_actor_reconcile
    ld a,(state_new_policy)
    and STATE_NEW_POLICY_ACTOR_RENDER
    jp z,@complete
    call state_new_actor_get_class
    jp nc,@complete
    ld a,(ix+STATE_NEW_ACTOR_CLASS_COUNT)
    or a
    jp z,@bad_config
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp nc,@bad_current
    call state_new_actor_save_expected
    push iy
    call state_new_actor_call_render
    pop iy
    call state_new_actor_validate_expected
    jp nc,@callback_fault
@complete:
    call state_new_actor_end_dispatch
    scf
    ret
@bad_config:
    call state_new_actor_quarantine
    jp @complete
@bad_current:
    call state_new_actor_recover
    jp @complete
@callback_fault:
    ld a,(state_new_actor_expected_class)
    cp (iy+STATE_NEW_ACTOR_CLASS)
    jp nz,@bad_config
    call state_new_actor_recover
    jp @complete
state_new_actor_request:
    ld c,a
    call state_new_actor_get_class
    ret nc
    ld a,(ix+STATE_NEW_ACTOR_CLASS_COUNT)
    or a
    jp z,state_new_actor_error_config
    ld a,c
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp nc,state_new_actor_error_target
    bit STATE_NEW_ACTOR_RUNTIME_BUSY,(iy+STATE_NEW_ACTOR_RUNTIME)
    jp z,state_new_actor_error_phase
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
    jp nc,@bad_config
    ld a,(ix+STATE_NEW_ACTOR_CLASS_COUNT)
    or a
    jp z,@bad_config
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
@bad_config:
    call state_new_actor_quarantine
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
    ld a,STATE_NEW_NONE
    ld (iy+STATE_NEW_ACTOR_CANDIDATE),a
    ld hl,0
    ld (iy+STATE_NEW_ACTOR_STATE_FRAMES),hl
    call state_new_actor_save_expected
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    push iy
    call state_new_actor_call_enter
    pop iy
    call state_new_actor_validate_expected
    ret c
    ld a,(state_new_actor_expected_class)
    cp (iy+STATE_NEW_ACTOR_CLASS)
    jp nz,state_new_actor_quarantine
    jp state_new_actor_recover

; A corrupted class ID cannot safely resolve even state zero. Quarantine the
; actor without another indirect call; later ticks keep latching CONFIG.
state_new_actor_quarantine:
    set STATE_NEW_ACTOR_ERROR_CONFIG,(iy+STATE_NEW_ACTOR_ERRORS)
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    ld (iy+STATE_NEW_ACTOR_PREVIOUS),a
    xor a
    ld (iy+STATE_NEW_ACTOR_CURRENT),a
    ld a,STATE_NEW_NONE
    ld (iy+STATE_NEW_ACTOR_CLASS),a
    ld (iy+STATE_NEW_ACTOR_PENDING),a
    ld (iy+STATE_NEW_ACTOR_CANDIDATE),a
    ld hl,0
    ld (iy+STATE_NEW_ACTOR_STATE_FRAMES),hl
    call state_new_actor_ack_game
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
    ld (iy+STATE_NEW_ACTOR_CANDIDATE),a
    ld hl,0
    ld (iy+STATE_NEW_ACTOR_STATE_FRAMES),hl
    call state_new_actor_get_class
    jp nc,@quarantine
    ld a,(ix+STATE_NEW_ACTOR_CLASS_COUNT)
    or a
    jp z,@quarantine
; Recovery also honors actor ownership: reconcile the safe state against the
; global phase before invoking its ENTER handler.
    call state_new_actor_save_expected
    push iy
    call state_new_actor_call_reconcile
    pop iy
    ld (state_new_actor_callback_result),a
    call state_new_actor_validate_expected
    jp nc,@recover_context
    ld a,(state_new_actor_callback_result)
    cp STATE_NEW_NONE
    jp z,@enter
    ld b,a
    call state_new_actor_get_class
    jp nc,@quarantine
    ld a,b
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp c,@use_reconciled
    set STATE_NEW_ACTOR_ERROR_TARGET,(iy+STATE_NEW_ACTOR_ERRORS)
    xor a
@use_reconciled:
    ld (iy+STATE_NEW_ACTOR_CURRENT),a
@enter:
    call state_new_actor_ack_game
    call state_new_actor_save_expected
    ld a,(iy+STATE_NEW_ACTOR_CURRENT)
    push iy
    call state_new_actor_call_enter
    pop iy
    call state_new_actor_validate_expected
    jp nc,@quarantine
    or a
    ret
@recover_context:
    ld a,(state_new_actor_expected_class)
    cp (iy+STATE_NEW_ACTOR_CLASS)
    jp nz,@quarantine
    xor a
    ld (iy+STATE_NEW_ACTOR_CURRENT),a
    ld a,STATE_NEW_NONE
    ld (iy+STATE_NEW_ACTOR_PENDING),a
    ld (iy+STATE_NEW_ACTOR_CANDIDATE),a
    jp @enter
@quarantine:
    call state_new_actor_quarantine
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
    ld a,(ix+STATE_NEW_ACTOR_CLASS_COUNT)
    or a
    jp z,state_new_actor_error_config
    ld a,b
    cp (ix+STATE_NEW_ACTOR_CLASS_COUNT)
    jp nc,state_new_actor_error_state
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
    ld a,(ix+STATE_NEW_ACTOR_CLASS_COUNT)
    or a
    jp z,state_new_actor_error_config
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

state_new_actor_error_reentrant:
    set STATE_NEW_ACTOR_ERROR_REENTRANT,(iy+STATE_NEW_ACTOR_ERRORS)
    or a
    ret

state_new_actor_error_state:
    set STATE_NEW_ACTOR_ERROR_STATE,(iy+STATE_NEW_ACTOR_ERRORS)
    or a
    ret

; ---------------------------------------------------------------------------
