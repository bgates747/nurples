; ASM-001B deterministic pure-CPU qualification for the replacement state core.

    assume adl=1
    org 040000h
    jp fixture_start

    include "../../src/asm/state_ids.inc"

STATE_NEW_ACTOR_CLASS_TEST:    equ 0
STATE_NEW_ACTOR_CLASS_POISON:  equ 1
STATE_NEW_ACTOR_CLASS_EMPTY:   equ 2
STATE_NEW_ACTOR_NUM_CLASSES:   equ 3

    include "../../src/asm/state_core.inc"
    include "../../src/asm/state_graph.inc"

FIXTURE_PASS:                  equ 02ah
FIXTURE_TRACE_CAPACITY:        equ 16
FIXTURE_ACTOR_SAFE:            equ 0
FIXTURE_ACTOR_A:               equ 1
FIXTURE_ACTOR_B:               equ 2
FIXTURE_ACTOR_COUNT:           equ 3
FIXTURE_POISON_COUNT:          equ 1

FIXTURE_G_ENTER:               equ 010h
FIXTURE_G_UPDATE:              equ 011h
FIXTURE_G_RENDER:              equ 012h
FIXTURE_G_EXIT:                equ 013h
FIXTURE_G_REQUEST_OK:          equ 014h
FIXTURE_G_REQUEST_REJECT:      equ 015h
FIXTURE_A_RECONCILE:           equ 020h
FIXTURE_A_ENTER:               equ 021h
FIXTURE_A_UPDATE:              equ 022h
FIXTURE_A_RENDER:              equ 023h
FIXTURE_A_EXIT:                equ 024h
FIXTURE_A_REQUEST_OK:          equ 025h
FIXTURE_A_REQUEST_REJECT:      equ 026h

FIXTURE_NEST_TICK:             equ 1
FIXTURE_NEST_RENDER:           equ 2
FIXTURE_NEST_INIT:             equ 3
FIXTURE_NEST_ACTOR:            equ 4

    macro CHECK8 check_address, check_expected
        ld a,(check_address)
        ld b,check_expected
        call fixture_assert_a
    endmacro

    macro CHECK24 check_address, byte0, byte1, byte2
        CHECK8 check_address+0,byte0
        CHECK8 check_address+1,byte1
        CHECK8 check_address+2,byte2
    endmacro

; ---------------------------------------------------------------------------
; Fixture-owned immutable configuration
; ---------------------------------------------------------------------------

state_new_descriptors:
    dl fixture_global_enter,fixture_global_update,fixture_global_render,fixture_global_exit
    dl fixture_global_enter,fixture_global_update,fixture_global_render,fixture_global_exit
    dl fixture_global_enter,fixture_global_update,fixture_global_render,fixture_global_exit
    dl fixture_global_enter,fixture_global_update,fixture_global_render,fixture_global_exit
    dl fixture_global_enter,fixture_global_update,fixture_global_render,fixture_global_exit
    dl fixture_global_enter,fixture_global_update,fixture_global_render,fixture_global_exit
    dl fixture_global_enter,fixture_global_update,fixture_global_render,fixture_global_exit
    dl fixture_global_enter,fixture_global_update,fixture_global_render,fixture_global_exit
    dl fixture_global_enter,fixture_global_update,fixture_global_render,fixture_global_exit
    dl fixture_global_enter,fixture_global_update,fixture_global_render,fixture_global_exit
    dl fixture_global_enter,fixture_global_update,fixture_global_render,fixture_global_exit
    dl fixture_global_enter,fixture_global_update,fixture_global_render,fixture_global_exit
    dl fixture_global_enter,fixture_global_update,fixture_global_render,fixture_global_exit
    dl fixture_global_enter,fixture_global_update,fixture_global_render,fixture_global_exit
state_new_descriptors_end:

state_new_policy_table:
    db 020h,030h,031h,030h,034h,035h,03fh
    db 035h,034h,034h,031h,031h,000h,030h
state_new_policy_table_end:

fixture_actor:
    db FIXTURE_ACTOR_SAFE,STATE_NEW_NONE,STATE_NEW_NONE,STATE_NEW_NONE
    db STATE_NEW_ACTOR_CLASS_TEST
    dl 0,0,0
    db 0,0
fixture_actor_end:

fixture_actor_descriptors:
    dl fixture_actor_enter,fixture_actor_update,fixture_actor_render,fixture_actor_exit
    dl fixture_actor_enter,fixture_actor_update,fixture_actor_render,fixture_actor_exit
    dl fixture_actor_enter,fixture_actor_update,fixture_actor_render,fixture_actor_exit
fixture_actor_descriptors_end:

fixture_poison_descriptors:
    dl fixture_poison_callback,fixture_poison_callback,fixture_poison_callback,fixture_poison_callback
fixture_poison_descriptors_end:

state_new_actor_classes:
    db FIXTURE_ACTOR_COUNT
    dl fixture_actor_descriptors
    dl fixture_actor_reconcile
    db FIXTURE_POISON_COUNT
    dl fixture_poison_descriptors
    dl fixture_poison_reconcile
    db 0
    dl fixture_poison_descriptors
    dl fixture_poison_reconcile
state_new_actor_classes_end:

    .if [state_new_descriptors_end-state_new_descriptors]-[STATE_NEW_COUNT*STATE_NEW_DESC_SIZE]
        dl fixture_global_descriptor_span_mismatch
    .endif
    .if [state_new_policy_table_end-state_new_policy_table]-STATE_NEW_COUNT
        dl fixture_policy_span_mismatch
    .endif
    .if [fixture_actor_end-fixture_actor]-STATE_NEW_ACTOR_SIZE
        dl fixture_actor_size_mismatch
    .endif
    .if [fixture_actor_descriptors_end-fixture_actor_descriptors]-[FIXTURE_ACTOR_COUNT*STATE_NEW_DESC_SIZE]
        dl fixture_actor_descriptor_span_mismatch
    .endif
    .if [fixture_poison_descriptors_end-fixture_poison_descriptors]-[FIXTURE_POISON_COUNT*STATE_NEW_DESC_SIZE]
        dl fixture_poison_descriptor_span_mismatch
    .endif
    .if [state_new_actor_classes_end-state_new_actor_classes]-[STATE_NEW_ACTOR_NUM_CLASSES*STATE_NEW_ACTOR_CLASS_SIZE]
        dl fixture_actor_registry_span_mismatch
    .endif

; Request-layer acceptance masks. Self requests are accepted before graph lookup.
fixture_request_masks:
    dl 03003h,03006h,0300ch,03018h,03030h,03060h,03fc0h
    dl 030c4h,03d10h,03620h,0340ch,0380ch,01000h,03000h
fixture_request_masks_end:

fixture_policy_oracle:
    db 020h,030h,031h,030h,034h,035h,03fh
    db 035h,034h,034h,031h,031h,000h,030h

; ---------------------------------------------------------------------------
; Trace-only callbacks
; ---------------------------------------------------------------------------

fixture_global_enter:
    ld a,FIXTURE_G_ENTER
    call fixture_trace_push
    ld a,(fixture_global_enter_nested)
    call fixture_global_nested
    jp fixture_poison_registers

fixture_global_update:
    ld a,FIXTURE_G_UPDATE
    call fixture_trace_push
    ld a,(state_new_frame_events)
    ld (fixture_seen_events),a
    ld a,(fixture_global_post_event)
    or a
    call nz,state_new_post_events
    ld a,(fixture_global_update_nested)
    call fixture_global_nested
    ld a,(fixture_global_request_first)
    cp STATE_NEW_NONE
    jp z,@second
    call state_new_request
    ld a,0
    adc a,0
    ld (fixture_global_request_result_first),a
    or a
    ld a,FIXTURE_G_REQUEST_REJECT
    jp z,@trace_first
    ld a,FIXTURE_G_REQUEST_OK
@trace_first:
    call fixture_trace_push
@second:
    ld a,(fixture_global_request_second)
    cp STATE_NEW_NONE
    jp z,@complete
    call state_new_request
    ld a,0
    adc a,0
    ld (fixture_global_request_result_second),a
    or a
    ld a,FIXTURE_G_REQUEST_REJECT
    jp z,@trace_second
    ld a,FIXTURE_G_REQUEST_OK
@trace_second:
    call fixture_trace_push
@complete:
    jp fixture_poison_registers

fixture_global_render:
    ld a,FIXTURE_G_RENDER
    call fixture_trace_push
    ld a,(fixture_global_render_nested)
    call fixture_global_nested
    jp fixture_poison_registers

fixture_global_exit:
    ld a,FIXTURE_G_EXIT
    call fixture_trace_push
    ld a,(fixture_global_exit_nested)
    call fixture_global_nested
    ld a,(fixture_global_exit_corrupt_current)
    or a
    jp z,@candidate
    ld (state_new_current),a
@candidate:
    ld a,(fixture_global_exit_corrupt_candidate)
    or a
    jp z,@complete
    ld (state_new_candidate),a
@complete:
    jp fixture_poison_registers

fixture_global_nested:
    or a
    ret z
    cp FIXTURE_NEST_TICK
    jp z,state_new_tick
    cp FIXTURE_NEST_RENDER
    jp z,state_new_render
    cp FIXTURE_NEST_INIT
    jp z,state_new_init
    cp FIXTURE_NEST_ACTOR
    ret nz
    ld iy,fixture_actor
    jp state_new_actor_tick

fixture_actor_reconcile:
    ld a,FIXTURE_A_RECONCILE
    call fixture_trace_push
    ld a,(fixture_actor_reconcile_corrupt_class)
    or a
    jp z,@current
    ld (iy+STATE_NEW_ACTOR_CLASS),a
@current:
    ld a,(fixture_actor_reconcile_corrupt_current)
    or a
    jp z,@nested
    ld (iy+STATE_NEW_ACTOR_CURRENT),a
@nested:
    ld a,(fixture_actor_reconcile_nested)
    call fixture_actor_nested
    ld a,(fixture_actor_reconcile_target)
    ret

fixture_actor_enter:
    ld a,FIXTURE_A_ENTER
    call fixture_trace_push
    ld a,(fixture_actor_enter_nested)
    call fixture_actor_nested
    jp fixture_poison_registers

fixture_actor_update:
    ld a,FIXTURE_A_UPDATE
    call fixture_trace_push
    ld a,(fixture_actor_update_nested)
    call fixture_actor_nested
    ld a,(fixture_actor_request_first)
    cp STATE_NEW_NONE
    jp z,@second
    call state_new_actor_request
    ld a,0
    adc a,0
    ld (fixture_actor_request_result_first),a
    or a
    ld a,FIXTURE_A_REQUEST_REJECT
    jp z,@trace_first
    ld a,FIXTURE_A_REQUEST_OK
@trace_first:
    call fixture_trace_push
@second:
    ld a,(fixture_actor_request_second)
    cp STATE_NEW_NONE
    jp z,@complete
    call state_new_actor_request
    ld a,0
    adc a,0
    ld (fixture_actor_request_result_second),a
    or a
    ld a,FIXTURE_A_REQUEST_REJECT
    jp z,@trace_second
    ld a,FIXTURE_A_REQUEST_OK
@trace_second:
    call fixture_trace_push
@complete:
    jp fixture_poison_registers

fixture_actor_render:
    ld a,FIXTURE_A_RENDER
    call fixture_trace_push
    jp fixture_poison_registers

fixture_actor_exit:
    ld a,FIXTURE_A_EXIT
    call fixture_trace_push
    jp fixture_poison_registers

fixture_actor_nested:
    or a
    ret z
    cp FIXTURE_NEST_TICK
    jp z,state_new_actor_tick
    cp FIXTURE_NEST_RENDER
    jp z,state_new_actor_render
    cp FIXTURE_NEST_INIT
    jp z,state_new_actor_init
    cp FIXTURE_NEST_ACTOR
    jp z,state_new_tick
    ret

fixture_poison_reconcile:
    ld hl,fixture_poison_callbacks
    inc (hl)
    xor a
    ret

fixture_poison_callback:
    ld hl,fixture_poison_callbacks
    inc (hl)
    ret

fixture_poison_registers:
    ld bc,0abcdefh
    ld de,0123456h
    ld hl,0654321h
    ld ix,055aa55h
    ld iy,0aa55aah
    ret

; ---------------------------------------------------------------------------
; Assertions, reset helpers, and stable result record
; ---------------------------------------------------------------------------

fixture_trace_push:
    ld (fixture_trace_value),a
    ld a,(fixture_trace_count)
    cp FIXTURE_TRACE_CAPACITY
    ret nc
    ld de,0
    ld e,a
    ld hl,fixture_trace
    add hl,de
    ld a,(fixture_trace_value)
    ld (hl),a
    ld hl,fixture_trace_count
    inc (hl)
    ret

fixture_assert_a:
    ld (fixture_assert_actual),a
    ld a,b
    ld (fixture_assert_expected),a
    ld hl,(fixture_checks)
    inc hl
    ld (fixture_checks),hl
    ld a,(fixture_assert_actual)
    cp b
    ret z
    ld hl,(fixture_errors)
    inc hl
    ld (fixture_errors),hl
    ld a,(fixture_first_case)
    or a
    ret nz
    ld a,(fixture_case)
    ld (fixture_first_case),a
    ld a,(fixture_assert_expected)
    ld (fixture_first_expected),a
    ld a,(fixture_assert_actual)
    ld (fixture_first_actual),a
    ld a,(fixture_matrix_source)
    ld (fixture_first_source),a
    ld a,(fixture_matrix_target)
    ld (fixture_first_target),a
    ret

fixture_begin_case:
    ld (fixture_case),a
    ld hl,fixture_cases
    inc (hl)
    call fixture_reset_controls
    xor a
    ld (fixture_trace_count),a
    ret

fixture_reset_controls:
    xor a
    ld hl,fixture_controls_begin
    ld (hl),a
    ld de,fixture_controls_begin+1
    ld bc,fixture_controls_end-fixture_controls_begin-1
    ldir
    ld a,STATE_NEW_NONE
    ld (fixture_global_request_first),a
    ld (fixture_global_request_second),a
    ld (fixture_global_request_result_first),a
    ld (fixture_global_request_result_second),a
    ld (fixture_actor_reconcile_target),a
    ld (fixture_actor_request_first),a
    ld (fixture_actor_request_second),a
    ld (fixture_actor_request_result_first),a
    ld (fixture_actor_request_result_second),a
    ret

fixture_expected_request:
    ld a,(fixture_matrix_source)
    ld de,0
    ld d,a
    ld e,3
    mlt de
    ld hl,fixture_request_masks
    add hl,de
    ld hl,(hl)
    ld a,(fixture_matrix_target)
    or a
    jp z,@result
    ld b,a
@shift:
    srl h
    rr l
    djnz @shift
@result:
    ld a,l
    and 1
    ret

fixture_expected_policy:
    ld de,0
    ld e,a
    ld hl,fixture_policy_oracle
    add hl,de
    ld a,(hl)
    ret

fixture_prepare_global_pair:
    call fixture_reset_controls
    xor a
    ld (fixture_trace_count),a
    ld (state_new_errors),a
    ld (state_new_posted_events),a
    ld (state_new_frame_events),a
    ld (state_new_in_tick),a
    ld (state_new_actor_dispatch_busy),a
    ld a,1
    ld (state_new_initialized),a
    ld a,(fixture_matrix_source)
    ld (state_new_current),a
    call state_new_load_policy
    ld a,0a5h
    ld (state_new_previous),a
    ld (state_new_last_transition_events),a
    ld a,STATE_NEW_NONE
    ld (state_new_pending),a
    ld (state_new_candidate),a
    ld (state_new_callback_expected),a
    ld a,STATE_NEW_PHASE_IDLE
    ld (state_new_phase),a
    ld hl,5
    ld (state_new_state_frames),hl
    ld hl,7
    ld (state_new_total_frames),hl
    ld hl,9
    ld (state_new_transition_serial),hl
    ld a,(fixture_matrix_target)
    ld (fixture_global_request_first),a
    ld a,STATE_NEW_EVENT_START
    call state_new_post_events
    ret

fixture_reset_actor:
    xor a
    ld hl,fixture_actor
    ld (hl),a
    ld de,fixture_actor+1
    ld bc,STATE_NEW_ACTOR_SIZE-1
    ldir
    ld a,STATE_NEW_NONE
    ld (fixture_actor+STATE_NEW_ACTOR_PREVIOUS),a
    ld (fixture_actor+STATE_NEW_ACTOR_PENDING),a
    ld (fixture_actor+STATE_NEW_ACTOR_CANDIDATE),a
    ld a,STATE_NEW_ACTOR_CLASS_TEST
    ld (fixture_actor+STATE_NEW_ACTOR_CLASS),a
    xor a
    ld (state_new_actor_dispatch_busy),a
    ld (state_new_in_tick),a
    ld a,STATE_NEW_PLAYING
    ld (state_new_current),a
    ld hl,012345h
    ld (state_new_transition_serial),hl
    ld a,STATE_NEW_POLICY_ACTOR_UPDATE+STATE_NEW_POLICY_ACTOR_RENDER
    ld (state_new_policy),a
    ld iy,fixture_actor
    ret

fixture_result:
    db "PNCF"
    db 1
fixture_cases:                  db 0
fixture_checks:                 dl 0
fixture_errors:                 dl 0
fixture_first_case:             db 0
fixture_first_expected:         db 0
fixture_first_actual:           db 0
fixture_first_source:           db 0
fixture_first_target:           db 0
fixture_result_end:

fixture_case:                   db 0
fixture_matrix_source:          db 0
fixture_matrix_target:          db 0
fixture_matrix_expected:        db 0
fixture_matrix_commits:         db 0
fixture_matrix_self:            db 0
fixture_matrix_illegal:         db 0
fixture_expected_current:       db 0
fixture_assert_actual:          db 0
fixture_assert_expected:        db 0
fixture_trace_value:            db 0
fixture_nested_value:           db 0
fixture_trace_count:            db 0
fixture_trace:                  blkb FIXTURE_TRACE_CAPACITY,0
fixture_seen_events:            db 0
fixture_poison_callbacks:       db 0

fixture_controls_begin:
fixture_global_request_first:   db STATE_NEW_NONE
fixture_global_request_second:  db STATE_NEW_NONE
fixture_global_request_result_first: db STATE_NEW_NONE
fixture_global_request_result_second: db STATE_NEW_NONE
fixture_global_post_event:      db 0
fixture_global_update_nested:   db 0
fixture_global_enter_nested:    db 0
fixture_global_exit_nested:     db 0
fixture_global_render_nested:   db 0
fixture_global_exit_corrupt_current: db 0
fixture_global_exit_corrupt_candidate: db 0
fixture_actor_reconcile_target: db STATE_NEW_NONE
fixture_actor_reconcile_corrupt_class: db 0
fixture_actor_reconcile_corrupt_current: db 0
fixture_actor_reconcile_nested: db 0
fixture_actor_enter_nested:     db 0
fixture_actor_update_nested:    db 0
fixture_actor_request_first:    db STATE_NEW_NONE
fixture_actor_request_second:   db STATE_NEW_NONE
fixture_actor_request_result_first: db STATE_NEW_NONE
fixture_actor_request_result_second: db STATE_NEW_NONE
fixture_controls_end:

; ---------------------------------------------------------------------------
; Test group 1: every global current/request pair
; ---------------------------------------------------------------------------

fixture_test_global_matrix:
    ld a,1
    call fixture_begin_case
    xor a
    ld (fixture_matrix_source),a
    ld (fixture_matrix_commits),a
    ld (fixture_matrix_self),a
    ld (fixture_matrix_illegal),a
@source:
    xor a
    ld (fixture_matrix_target),a
@target:
    call fixture_prepare_global_pair
    call fixture_expected_request
    ld (fixture_matrix_expected),a
    or a
    jp z,@classified_illegal
    ld a,(fixture_matrix_source)
    ld b,a
    ld a,(fixture_matrix_target)
    cp b
    jp z,@classified_self
    ld hl,fixture_matrix_commits
    inc (hl)
    jp @run
@classified_self:
    ld hl,fixture_matrix_self
    inc (hl)
    jp @run
@classified_illegal:
    ld hl,fixture_matrix_illegal
    inc (hl)
@run:
    call state_new_tick

; Callback request result must equal the independent mask oracle.
    ld a,(fixture_matrix_expected)
    ld b,a
    ld a,(fixture_global_request_result_first)
    call fixture_assert_a

; A legal non-self request commits; every other pair stays in its source.
    ld a,(fixture_matrix_expected)
    or a
    jp z,@expected_source
    ld a,(fixture_matrix_source)
    ld b,a
    ld a,(fixture_matrix_target)
    cp b
    jp z,@expected_source
    ld a,(fixture_matrix_target)
    jp @have_expected_current
@expected_source:
    ld a,(fixture_matrix_source)
@have_expected_current:
    ld (fixture_expected_current),a
    ld b,a
    ld a,(state_new_current)
    call fixture_assert_a

    CHECK8 state_new_pending,STATE_NEW_NONE
    CHECK8 state_new_candidate,STATE_NEW_NONE
    CHECK8 state_new_phase,STATE_NEW_PHASE_IDLE
    CHECK8 state_new_in_tick,0
    CHECK8 state_new_actor_dispatch_busy,0
    CHECK24 state_new_total_frames,8,0,0

; Illegal requests latch only ILLEGAL_EDGE. Accepted requests stay fault-free.
    ld a,(fixture_matrix_expected)
    or a
    ld b,0
    jp nz,@check_errors
    ld b,1<<STATE_NEW_ERROR_ILLEGAL_EDGE
@check_errors:
    ld a,(state_new_errors)
    call fixture_assert_a

; Policy follows the final current ID.
    ld a,(fixture_expected_current)
    call fixture_expected_policy
    ld b,a
    ld a,(state_new_policy)
    call fixture_assert_a

; Lifecycle and counter invariants differ only for a committed non-self edge.
    ld a,(fixture_matrix_expected)
    or a
    jp z,@no_commit
    ld a,(fixture_matrix_source)
    ld b,a
    ld a,(fixture_matrix_target)
    cp b
    jp z,@no_commit
    ld a,(fixture_matrix_source)
    ld b,a
    ld a,(state_new_previous)
    call fixture_assert_a
    CHECK8 state_new_last_transition_events,STATE_NEW_EVENT_START
    CHECK24 state_new_state_frames,0,0,0
    CHECK24 state_new_transition_serial,10,0,0
    CHECK8 fixture_trace_count,4
    CHECK8 fixture_trace+0,FIXTURE_G_UPDATE
    CHECK8 fixture_trace+1,FIXTURE_G_REQUEST_OK
    CHECK8 fixture_trace+2,FIXTURE_G_EXIT
    CHECK8 fixture_trace+3,FIXTURE_G_ENTER
    jp @next
@no_commit:
    CHECK8 state_new_previous,0a5h
    CHECK8 state_new_last_transition_events,0a5h
    CHECK24 state_new_state_frames,6,0,0
    CHECK24 state_new_transition_serial,9,0,0
    CHECK8 fixture_trace_count,2
    CHECK8 fixture_trace+0,FIXTURE_G_UPDATE
    ld a,(fixture_matrix_expected)
    or a
    ld b,FIXTURE_G_REQUEST_REJECT
    jp z,@request_trace
    ld b,FIXTURE_G_REQUEST_OK
@request_trace:
    ld a,(fixture_trace+1)
    call fixture_assert_a

@next:
    ld hl,fixture_matrix_target
    inc (hl)
    ld a,(hl)
    cp STATE_NEW_COUNT
    jp c,@target
    ld hl,fixture_matrix_source
    inc (hl)
    ld a,(hl)
    cp STATE_NEW_COUNT
    jp c,@source

    CHECK8 fixture_matrix_commits,47
    CHECK8 fixture_matrix_self,14
    CHECK8 fixture_matrix_illegal,135
    ret

; ---------------------------------------------------------------------------
; Test groups 2-9: focused global recovery, arbitration, and reentrancy
; ---------------------------------------------------------------------------

fixture_test_process_init:
    ld a,2
    call fixture_begin_case
    ld a,0a5h
    ld hl,state_new_current
    ld (hl),a
    ld de,state_new_current+1
    ld bc,state_new_actor_callback_result-state_new_current
    ldir
    call state_new_process_init
    CHECK8 state_new_current,STATE_NEW_BOOT
    CHECK8 state_new_previous,STATE_NEW_NONE
    CHECK8 state_new_pending,STATE_NEW_NONE
    CHECK8 state_new_candidate,STATE_NEW_NONE
    CHECK8 state_new_phase,STATE_NEW_PHASE_IDLE
    CHECK8 state_new_policy,020h
    CHECK8 state_new_initialized,1
    CHECK8 state_new_in_tick,0
    CHECK8 state_new_actor_dispatch_busy,0
    CHECK8 state_new_errors,0
    CHECK8 state_new_posted_events,0
    CHECK8 state_new_frame_events,0
    CHECK24 state_new_state_frames,0,0,0
    CHECK24 state_new_total_frames,0,0,0
    CHECK24 state_new_transition_serial,0,0,0
    CHECK8 state_new_callback_expected,STATE_NEW_NONE
    CHECK8 state_new_actor_expected_class,STATE_NEW_NONE
    CHECK8 state_new_actor_expected_current,STATE_NEW_NONE
    CHECK8 state_new_actor_callback_result,STATE_NEW_NONE
    CHECK8 fixture_trace_count,1
    CHECK8 fixture_trace+0,FIXTURE_G_ENTER
    ret

fixture_test_invalid_global_ids:
    ld a,3
    call fixture_begin_case
    call state_new_process_init
    xor a
    ld (fixture_trace_count),a
    ld a,STATE_NEW_COUNT
    ld (state_new_current),a
    call state_new_tick
    CHECK8 state_new_current,STATE_NEW_FAULT
    CHECK8 state_new_previous,STATE_NEW_COUNT
    CHECK8 state_new_errors,1<<STATE_NEW_ERROR_INVALID_STATE
    CHECK8 state_new_pending,STATE_NEW_NONE
    CHECK8 state_new_candidate,STATE_NEW_NONE
    CHECK8 state_new_phase,STATE_NEW_PHASE_IDLE
    CHECK8 state_new_in_tick,0
    CHECK24 state_new_total_frames,0,0,0
    CHECK24 state_new_transition_serial,1,0,0
    CHECK8 fixture_trace_count,1
    CHECK8 fixture_trace+0,FIXTURE_G_ENTER

    call state_new_process_init
    xor a
    ld (fixture_trace_count),a
    ld a,STATE_NEW_NONE
    ld (state_new_current),a
    call state_new_render
    CHECK8 state_new_current,STATE_NEW_FAULT
    CHECK8 state_new_previous,STATE_NEW_NONE
    CHECK8 state_new_errors,1<<STATE_NEW_ERROR_INVALID_STATE
    CHECK8 state_new_phase,STATE_NEW_PHASE_IDLE
    CHECK8 state_new_in_tick,0
    CHECK8 fixture_trace_count,1
    CHECK8 fixture_trace+0,FIXTURE_G_ENTER
    ret

fixture_test_invalid_global_targets:
    ld a,4
    call fixture_begin_case
    call state_new_process_init
    ld a,STATE_NEW_PLAYING
    ld (state_new_current),a
    ld a,STATE_NEW_PHASE_UPDATE
    ld (state_new_phase),a
    ld a,STATE_NEW_COUNT
    call state_new_request
    ld a,0
    adc a,0
    ld b,0
    call fixture_assert_a
    CHECK8 state_new_errors,1<<STATE_NEW_ERROR_INVALID_TARGET
    CHECK8 state_new_pending,STATE_NEW_NONE

    xor a
    ld (state_new_errors),a
    ld a,STATE_NEW_NONE
    call state_new_request
    ld a,0
    adc a,0
    ld b,0
    call fixture_assert_a
    CHECK8 state_new_errors,1<<STATE_NEW_ERROR_INVALID_TARGET
    CHECK8 state_new_pending,STATE_NEW_NONE

; A corrupted pending target must enter FAULT without calling the old EXIT.
    ld a,STATE_NEW_PLAYING
    ld (fixture_matrix_source),a
    xor a
    ld (fixture_matrix_target),a
    call fixture_prepare_global_pair
    ld a,STATE_NEW_NONE
    ld (fixture_global_request_first),a
    ld a,STATE_NEW_COUNT
    ld (state_new_pending),a
    call state_new_tick
    CHECK8 state_new_current,STATE_NEW_FAULT
    CHECK8 state_new_errors,1<<STATE_NEW_ERROR_INVALID_TARGET
    CHECK8 state_new_pending,STATE_NEW_NONE
    CHECK8 state_new_candidate,STATE_NEW_NONE
    CHECK8 fixture_trace_count,2
    CHECK8 fixture_trace+0,FIXTURE_G_UPDATE
    CHECK8 fixture_trace+1,FIXTURE_G_ENTER
    ret

fixture_test_global_conflicts:
    ld a,5
    call fixture_begin_case
    ld a,STATE_NEW_PLAYING
    ld (fixture_matrix_source),a
    ld a,STATE_NEW_PAUSED
    ld (fixture_matrix_target),a
    call fixture_prepare_global_pair
    ld a,STATE_NEW_RESPAWN
    ld (fixture_global_request_second),a
    call state_new_tick
    CHECK8 state_new_current,STATE_NEW_PAUSED
    CHECK8 state_new_errors,1<<STATE_NEW_ERROR_CONFLICT
    CHECK8 fixture_global_request_result_first,1
    CHECK8 fixture_global_request_result_second,0
    CHECK8 state_new_pending,STATE_NEW_NONE
    CHECK8 state_new_candidate,STATE_NEW_NONE
    CHECK8 fixture_trace_count,5
    CHECK8 fixture_trace+0,FIXTURE_G_UPDATE
    CHECK8 fixture_trace+1,FIXTURE_G_REQUEST_OK
    CHECK8 fixture_trace+2,FIXTURE_G_REQUEST_REJECT
    CHECK8 fixture_trace+3,FIXTURE_G_EXIT
    CHECK8 fixture_trace+4,FIXTURE_G_ENTER

; A same-state no-op does not claim the first-request slot.
    ld a,STATE_NEW_PLAYING
    ld (fixture_matrix_source),a
    ld (fixture_matrix_target),a
    call fixture_prepare_global_pair
    ld a,STATE_NEW_PAUSED
    ld (fixture_global_request_second),a
    call state_new_tick
    CHECK8 state_new_current,STATE_NEW_PAUSED
    CHECK8 state_new_errors,0
    CHECK8 fixture_global_request_result_first,1
    CHECK8 fixture_global_request_result_second,1
    CHECK8 state_new_candidate,STATE_NEW_NONE
    ret

fixture_test_event_snapshot_and_quit:
    ld a,6
    call fixture_begin_case
    ld a,STATE_NEW_PLAYING
    ld (fixture_matrix_source),a
    xor a
    ld (fixture_matrix_target),a
    call fixture_prepare_global_pair
    ld a,STATE_NEW_NONE
    ld (fixture_global_request_first),a
    ld a,STATE_NEW_EVENT_PAUSE
    ld (fixture_global_post_event),a
    call state_new_tick
    CHECK8 fixture_seen_events,STATE_NEW_EVENT_START
    CHECK8 state_new_frame_events,STATE_NEW_EVENT_START
    CHECK8 state_new_posted_events,STATE_NEW_EVENT_PAUSE
    xor a
    ld (fixture_global_post_event),a
    call state_new_tick
    CHECK8 fixture_seen_events,STATE_NEW_EVENT_PAUSE
    CHECK8 state_new_frame_events,STATE_NEW_EVENT_PAUSE
    CHECK8 state_new_posted_events,0

; QUIT skips UPDATE and retains the complete initiating snapshot.
    ld a,STATE_NEW_PLAYING
    ld (fixture_matrix_source),a
    ld a,STATE_NEW_PAUSED
    ld (fixture_matrix_target),a
    call fixture_prepare_global_pair
    ld a,STATE_NEW_EVENT_QUIT+STATE_NEW_EVENT_START
    ld (state_new_posted_events),a
    call state_new_tick
    CHECK8 state_new_current,STATE_NEW_EXIT
    CHECK8 state_new_last_transition_events,STATE_NEW_EVENT_QUIT+STATE_NEW_EVENT_START
    CHECK8 fixture_global_request_result_first,STATE_NEW_NONE
    CHECK8 fixture_trace_count,2
    CHECK8 fixture_trace+0,FIXTURE_G_EXIT
    CHECK8 fixture_trace+1,FIXTURE_G_ENTER
    ret

fixture_test_global_reentrancy:
    ld a,7
    call fixture_begin_case
    ld a,FIXTURE_NEST_TICK
    call fixture_test_update_nested
    ld a,FIXTURE_NEST_RENDER
    call fixture_test_update_nested
    ld a,FIXTURE_NEST_INIT
    call fixture_test_update_nested

; Actor dispatch from a global callback is rejected by the cross-domain guard.
    call fixture_reset_actor
    ld a,STATE_NEW_PLAYING
    ld (fixture_matrix_source),a
    xor a
    ld (fixture_matrix_target),a
    call fixture_prepare_global_pair
    ld a,STATE_NEW_NONE
    ld (fixture_global_request_first),a
    ld a,FIXTURE_NEST_ACTOR
    ld (fixture_global_update_nested),a
    call state_new_tick
    CHECK8 fixture_actor+STATE_NEW_ACTOR_ERRORS,1<<STATE_NEW_ACTOR_ERROR_REENTRANT
    CHECK8 state_new_errors,0
    CHECK8 state_new_in_tick,0
    CHECK8 state_new_actor_dispatch_busy,0

; EXIT and ENTER both reject nested ticks while the outer transition commits.
    ld a,STATE_NEW_PLAYING
    ld (fixture_matrix_source),a
    ld a,STATE_NEW_PAUSED
    ld (fixture_matrix_target),a
    call fixture_prepare_global_pair
    ld a,FIXTURE_NEST_TICK
    ld (fixture_global_exit_nested),a
    ld (fixture_global_enter_nested),a
    call state_new_tick
    CHECK8 state_new_current,STATE_NEW_PAUSED
    CHECK8 state_new_errors,1<<STATE_NEW_ERROR_REENTRANT
    CHECK8 state_new_in_tick,0
    CHECK8 state_new_phase,STATE_NEW_PHASE_IDLE

; RENDER owns the same guard and cannot recursively advance UPDATE.
    call state_new_process_init
    call fixture_reset_controls
    xor a
    ld (fixture_trace_count),a
    ld a,STATE_NEW_PLAYING
    ld (state_new_current),a
    call state_new_load_policy
    ld a,FIXTURE_NEST_TICK
    ld (fixture_global_render_nested),a
    call state_new_render
    CHECK8 state_new_errors,1<<STATE_NEW_ERROR_REENTRANT
    CHECK8 state_new_in_tick,0
    CHECK8 state_new_phase,STATE_NEW_PHASE_IDLE
    CHECK8 fixture_trace_count,1
    CHECK8 fixture_trace+0,FIXTURE_G_RENDER
    ret

fixture_test_update_nested:
    ld (fixture_nested_value),a
    ld a,STATE_NEW_PLAYING
    ld (fixture_matrix_source),a
    xor a
    ld (fixture_matrix_target),a
    call fixture_prepare_global_pair
    ld a,STATE_NEW_NONE
    ld (fixture_global_request_first),a
    ld a,(fixture_nested_value)
    ld (fixture_global_update_nested),a
    call state_new_tick
    CHECK8 state_new_errors,1<<STATE_NEW_ERROR_REENTRANT
    CHECK8 state_new_in_tick,0
    CHECK8 state_new_phase,STATE_NEW_PHASE_IDLE
    CHECK24 state_new_total_frames,8,0,0
    CHECK8 fixture_trace_count,1
    CHECK8 fixture_trace+0,FIXTURE_G_UPDATE
    ret

fixture_test_corrupt_global_exit:
    ld a,8
    call fixture_begin_case
    ld a,STATE_NEW_PLAYING
    ld (fixture_matrix_source),a
    ld a,STATE_NEW_PAUSED
    ld (fixture_matrix_target),a
    call fixture_prepare_global_pair
    ld a,STATE_NEW_COUNT
    ld (fixture_global_exit_corrupt_current),a
    call state_new_tick
    CHECK8 state_new_current,STATE_NEW_FAULT
    CHECK8 state_new_errors,1<<STATE_NEW_ERROR_INVALID_STATE
    CHECK8 state_new_pending,STATE_NEW_NONE
    CHECK8 state_new_candidate,STATE_NEW_NONE
    CHECK8 fixture_trace_count,4
    CHECK8 fixture_trace+0,FIXTURE_G_UPDATE
    CHECK8 fixture_trace+1,FIXTURE_G_REQUEST_OK
    CHECK8 fixture_trace+2,FIXTURE_G_EXIT
    CHECK8 fixture_trace+3,FIXTURE_G_ENTER

    ld a,STATE_NEW_PLAYING
    ld (fixture_matrix_source),a
    ld a,STATE_NEW_PAUSED
    ld (fixture_matrix_target),a
    call fixture_prepare_global_pair
    ld a,STATE_NEW_COUNT
    ld (fixture_global_exit_corrupt_candidate),a
    call state_new_tick
    CHECK8 state_new_current,STATE_NEW_FAULT
    CHECK8 state_new_errors,1<<STATE_NEW_ERROR_INVALID_TARGET
    CHECK8 state_new_candidate,STATE_NEW_NONE
    ret

; ---------------------------------------------------------------------------
; Test groups 9-14: actor-owned reconciliation and containment
; ---------------------------------------------------------------------------

fixture_init_actor_a:
    call fixture_reset_actor
    call fixture_reset_controls
    xor a
    ld (fixture_trace_count),a
    ld iy,fixture_actor
    ld a,FIXTURE_ACTOR_A
    call state_new_actor_init
    ret

fixture_test_actor_order:
    ld a,9
    call fixture_begin_case
; Initial reconciliation selects B before the first ENTER; A is never entered.
    call fixture_reset_actor
    ld a,FIXTURE_ACTOR_B
    ld (fixture_actor_reconcile_target),a
    ld iy,fixture_actor
    ld a,FIXTURE_ACTOR_A
    call state_new_actor_init
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CURRENT,FIXTURE_ACTOR_B
    CHECK8 fixture_actor+STATE_NEW_ACTOR_PREVIOUS,STATE_NEW_NONE
    CHECK8 fixture_actor+STATE_NEW_ACTOR_PENDING,STATE_NEW_NONE
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CANDIDATE,STATE_NEW_NONE
    CHECK8 fixture_actor+STATE_NEW_ACTOR_ERRORS,0
    CHECK8 fixture_actor+STATE_NEW_ACTOR_RUNTIME,0
    CHECK24 fixture_actor+STATE_NEW_ACTOR_GAME_SERIAL,045h,023h,001h
    CHECK8 state_new_actor_dispatch_busy,0
    CHECK8 fixture_trace_count,2
    CHECK8 fixture_trace+0,FIXTURE_A_RECONCILE
    CHECK8 fixture_trace+1,FIXTURE_A_ENTER

; Keeping A always reconciles before UPDATE and acknowledges the game serial.
    call fixture_init_actor_a
    call fixture_reset_controls
    xor a
    ld (fixture_trace_count),a
    ld iy,fixture_actor
    call state_new_actor_tick
    CHECK8 fixture_trace_count,2
    CHECK8 fixture_trace+0,FIXTURE_A_RECONCILE
    CHECK8 fixture_trace+1,FIXTURE_A_UPDATE
    CHECK24 fixture_actor+STATE_NEW_ACTOR_STATE_FRAMES,1,0,0
    CHECK24 fixture_actor+STATE_NEW_ACTOR_GAME_SERIAL,045h,023h,001h

; Reconciliation-driven A->B runs EXIT/ENTER and skips UPDATE this frame.
    call fixture_init_actor_a
    call fixture_reset_controls
    xor a
    ld (fixture_trace_count),a
    ld a,FIXTURE_ACTOR_B
    ld (fixture_actor_reconcile_target),a
    ld iy,fixture_actor
    call state_new_actor_tick
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CURRENT,FIXTURE_ACTOR_B
    CHECK8 fixture_actor+STATE_NEW_ACTOR_PREVIOUS,FIXTURE_ACTOR_A
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CANDIDATE,STATE_NEW_NONE
    CHECK24 fixture_actor+STATE_NEW_ACTOR_STATE_FRAMES,0,0,0
    CHECK8 fixture_trace_count,3
    CHECK8 fixture_trace+0,FIXTURE_A_RECONCILE
    CHECK8 fixture_trace+1,FIXTURE_A_EXIT
    CHECK8 fixture_trace+2,FIXTURE_A_ENTER

; With actor UPDATE disabled, reconciliation still runs and age does not move.
    call fixture_init_actor_a
    call fixture_reset_controls
    xor a
    ld (fixture_trace_count),a
    ld (state_new_policy),a
    ld iy,fixture_actor
    call state_new_actor_tick
    CHECK8 fixture_trace_count,1
    CHECK8 fixture_trace+0,FIXTURE_A_RECONCILE
    CHECK24 fixture_actor+STATE_NEW_ACTOR_STATE_FRAMES,0,0,0
    ret

fixture_test_actor_requests:
    ld a,10
    call fixture_begin_case
    call fixture_init_actor_a
    call fixture_reset_controls
    xor a
    ld (fixture_trace_count),a
    ld a,FIXTURE_ACTOR_B
    ld (fixture_actor_request_first),a
    xor a
    ld (fixture_actor_request_second),a
    ld iy,fixture_actor
    call state_new_actor_tick
    CHECK8 fixture_actor_request_result_first,1
    CHECK8 fixture_actor_request_result_second,0
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CURRENT,FIXTURE_ACTOR_B
    CHECK8 fixture_actor+STATE_NEW_ACTOR_ERRORS,1<<STATE_NEW_ACTOR_ERROR_CONFLICT
    CHECK8 fixture_actor+STATE_NEW_ACTOR_PENDING,STATE_NEW_NONE
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CANDIDATE,STATE_NEW_NONE
    CHECK8 fixture_trace_count,6
    CHECK8 fixture_trace+0,FIXTURE_A_RECONCILE
    CHECK8 fixture_trace+1,FIXTURE_A_UPDATE
    CHECK8 fixture_trace+2,FIXTURE_A_REQUEST_OK
    CHECK8 fixture_trace+3,FIXTURE_A_REQUEST_REJECT
    CHECK8 fixture_trace+4,FIXTURE_A_EXIT
    CHECK8 fixture_trace+5,FIXTURE_A_ENTER

; Duplicate requests commit once without conflict.
    call fixture_init_actor_a
    call fixture_reset_controls
    xor a
    ld (fixture_trace_count),a
    ld a,FIXTURE_ACTOR_B
    ld (fixture_actor_request_first),a
    ld (fixture_actor_request_second),a
    ld iy,fixture_actor
    call state_new_actor_tick
    CHECK8 fixture_actor_request_result_first,1
    CHECK8 fixture_actor_request_result_second,1
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CURRENT,FIXTURE_ACTOR_B
    CHECK8 fixture_actor+STATE_NEW_ACTOR_ERRORS,0
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CANDIDATE,STATE_NEW_NONE

; Requests outside UPDATE are phase errors and never become pending.
    call fixture_init_actor_a
    call fixture_reset_controls
    ld iy,fixture_actor
    ld a,FIXTURE_ACTOR_B
    call state_new_actor_request
    ld a,0
    adc a,0
    ld b,0
    call fixture_assert_a
    CHECK8 fixture_actor+STATE_NEW_ACTOR_ERRORS,1<<STATE_NEW_ACTOR_ERROR_PHASE
    CHECK8 fixture_actor+STATE_NEW_ACTOR_PENDING,STATE_NEW_NONE
    ret

fixture_test_actor_recovery:
    ld a,11
    call fixture_begin_case
; Invalid current is never dispatched; recovery reconciles SAFE before ENTER.
    call fixture_init_actor_a
    call fixture_reset_controls
    xor a
    ld (fixture_trace_count),a
    ld a,STATE_NEW_NONE
    ld (fixture_actor+STATE_NEW_ACTOR_CURRENT),a
    ld a,FIXTURE_ACTOR_B
    ld (fixture_actor_reconcile_target),a
    ld iy,fixture_actor
    call state_new_actor_tick
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CURRENT,FIXTURE_ACTOR_B
    CHECK8 fixture_actor+STATE_NEW_ACTOR_PREVIOUS,STATE_NEW_NONE
    CHECK8 fixture_actor+STATE_NEW_ACTOR_ERRORS,1<<STATE_NEW_ACTOR_ERROR_STATE
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CANDIDATE,STATE_NEW_NONE
    CHECK8 fixture_trace_count,2
    CHECK8 fixture_trace+0,FIXTURE_A_RECONCILE
    CHECK8 fixture_trace+1,FIXTURE_A_ENTER

; Invalid reconcile targets recover to SAFE after a second bounded reconcile.
    call fixture_init_actor_a
    call fixture_reset_controls
    xor a
    ld (fixture_trace_count),a
    ld a,FIXTURE_ACTOR_COUNT
    ld (fixture_actor_reconcile_target),a
    ld iy,fixture_actor
    call state_new_actor_tick
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CURRENT,FIXTURE_ACTOR_SAFE
    CHECK8 fixture_actor+STATE_NEW_ACTOR_ERRORS,[1<<STATE_NEW_ACTOR_ERROR_STATE]+[1<<STATE_NEW_ACTOR_ERROR_TARGET]
    CHECK8 fixture_actor+STATE_NEW_ACTOR_PENDING,STATE_NEW_NONE
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CANDIDATE,STATE_NEW_NONE
    CHECK8 fixture_trace_count,3
    CHECK8 fixture_trace+0,FIXTURE_A_RECONCILE
    CHECK8 fixture_trace+1,FIXTURE_A_RECONCILE
    CHECK8 fixture_trace+2,FIXTURE_A_ENTER
    ret

fixture_test_actor_quarantine:
    ld a,12
    call fixture_begin_case
; Mutation to another numerically valid class is quarantined permanently.
    call fixture_init_actor_a
    call fixture_reset_controls
    xor a
    ld (fixture_trace_count),a
    ld (fixture_poison_callbacks),a
    ld a,STATE_NEW_ACTOR_CLASS_POISON
    ld (fixture_actor_reconcile_corrupt_class),a
    ld iy,fixture_actor
    call state_new_actor_tick
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CLASS,STATE_NEW_NONE
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CURRENT,FIXTURE_ACTOR_SAFE
    CHECK8 fixture_actor+STATE_NEW_ACTOR_ERRORS,1<<STATE_NEW_ACTOR_ERROR_CONFIG
    CHECK8 fixture_poison_callbacks,0
    ld iy,fixture_actor
    call state_new_actor_tick
    ld iy,fixture_actor
    call state_new_actor_render
    CHECK8 fixture_poison_callbacks,0
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CLASS,STATE_NEW_NONE

; A configured zero-count class also quarantines without any callback lookup.
    call fixture_reset_actor
    call fixture_reset_controls
    xor a
    ld (fixture_poison_callbacks),a
    ld a,STATE_NEW_ACTOR_CLASS_EMPTY
    ld (fixture_actor+STATE_NEW_ACTOR_CLASS),a
    ld a,FIXTURE_ACTOR_A
    ld (fixture_actor+STATE_NEW_ACTOR_CURRENT),a
    ld iy,fixture_actor
    call state_new_actor_tick
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CLASS,STATE_NEW_NONE
    CHECK8 fixture_actor+STATE_NEW_ACTOR_ERRORS,1<<STATE_NEW_ACTOR_ERROR_CONFIG
    CHECK8 fixture_poison_callbacks,0

; Bad-class initialization deterministically clears the poisoned controller.
    call fixture_reset_actor
    ld a,STATE_NEW_NONE
    ld (fixture_actor+STATE_NEW_ACTOR_CLASS),a
    ld a,0a5h
    ld (fixture_actor+STATE_NEW_ACTOR_CURRENT),a
    ld iy,fixture_actor
    ld a,FIXTURE_ACTOR_A
    call state_new_actor_init
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CLASS,STATE_NEW_NONE
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CURRENT,FIXTURE_ACTOR_SAFE
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CANDIDATE,STATE_NEW_NONE
    CHECK8 fixture_actor+STATE_NEW_ACTOR_ERRORS,1<<STATE_NEW_ACTOR_ERROR_CONFIG
    CHECK24 fixture_actor+STATE_NEW_ACTOR_GAME_SERIAL,045h,023h,001h
    ret

fixture_test_actor_reentrancy:
    ld a,13
    call fixture_begin_case
    ld a,FIXTURE_NEST_TICK
    call fixture_test_actor_update_nested
    ld a,FIXTURE_NEST_RENDER
    call fixture_test_actor_update_nested
    ld a,FIXTURE_NEST_INIT
    call fixture_test_actor_update_nested

; Actor UPDATE cannot recursively tick the global coordinator either.
    call fixture_init_actor_a
    call fixture_reset_controls
    xor a
    ld (fixture_trace_count),a
    ld (state_new_errors),a
    ld a,1
    ld (state_new_initialized),a
    ld a,FIXTURE_NEST_ACTOR
    ld (fixture_actor_update_nested),a
    ld iy,fixture_actor
    call state_new_actor_tick
    CHECK8 state_new_errors,1<<STATE_NEW_ERROR_REENTRANT
    CHECK8 fixture_actor+STATE_NEW_ACTOR_ERRORS,0
    CHECK8 state_new_actor_dispatch_busy,0
    CHECK8 fixture_actor+STATE_NEW_ACTOR_RUNTIME,0
    CHECK8 fixture_trace_count,2
    CHECK8 fixture_trace+0,FIXTURE_A_RECONCILE
    CHECK8 fixture_trace+1,FIXTURE_A_UPDATE

; Reconcile-driven render enters B, then renders B in the same render call.
    call fixture_init_actor_a
    call fixture_reset_controls
    xor a
    ld (fixture_trace_count),a
    ld a,FIXTURE_ACTOR_B
    ld (fixture_actor_reconcile_target),a
    ld iy,fixture_actor
    call state_new_actor_render
    CHECK8 fixture_actor+STATE_NEW_ACTOR_CURRENT,FIXTURE_ACTOR_B
    CHECK8 fixture_trace_count,4
    CHECK8 fixture_trace+0,FIXTURE_A_RECONCILE
    CHECK8 fixture_trace+1,FIXTURE_A_EXIT
    CHECK8 fixture_trace+2,FIXTURE_A_ENTER
    CHECK8 fixture_trace+3,FIXTURE_A_RENDER
    ret

fixture_test_actor_update_nested:
    ld (fixture_nested_value),a
    call fixture_init_actor_a
    call fixture_reset_controls
    xor a
    ld (fixture_trace_count),a
    ld a,(fixture_nested_value)
    ld (fixture_actor_update_nested),a
    ld iy,fixture_actor
    call state_new_actor_tick
    CHECK8 fixture_actor+STATE_NEW_ACTOR_ERRORS,1<<STATE_NEW_ACTOR_ERROR_REENTRANT
    CHECK8 state_new_actor_dispatch_busy,0
    CHECK8 fixture_actor+STATE_NEW_ACTOR_RUNTIME,0
    CHECK24 fixture_actor+STATE_NEW_ACTOR_STATE_FRAMES,1,0,0
    CHECK8 fixture_trace_count,2
    CHECK8 fixture_trace+0,FIXTURE_A_RECONCILE
    CHECK8 fixture_trace+1,FIXTURE_A_UPDATE
    ret

fixture_start:
    ld sp,05ff00h
    xor a
    ld hl,fixture_cases
    ld (hl),a
    ld de,fixture_cases+1
    ld bc,fixture_result_end-fixture_cases-1
    ldir
    call fixture_test_global_matrix
    call fixture_test_process_init
    call fixture_test_invalid_global_ids
    call fixture_test_invalid_global_targets
    call fixture_test_global_conflicts
    call fixture_test_event_snapshot_and_quit
    call fixture_test_global_reentrancy
    call fixture_test_corrupt_global_exit
    call fixture_test_actor_order
    call fixture_test_actor_requests
    call fixture_test_actor_recovery
    call fixture_test_actor_quarantine
    call fixture_test_actor_reentrancy
    ld hl,(fixture_errors)
    ld a,h
    or l
    jp nz,fixture_fail
    ld a,FIXTURE_PASS
    out (0),a

fixture_fail:
    ld a,(fixture_first_case)
    or a
    jp nz,@emit
    ld a,0feh
@emit:
    out (0),a
