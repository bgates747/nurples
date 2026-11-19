; Exact feature-variable transaction for prospective hardware-sprite overlays.
;
; The restore branches require VDPlet v0.1.3-alpha or a compatible fix.
; The VDP itself snapshots both the presence and 16-bit value of variable 2, so
; no CPU-side response packet is required.
;
; prerequisites:
;   vdu_write_block_to_buffer, vdu_call_buffer, vdu_clear_buffer

OVERLAY_FEATURE_VARIABLE:          equ 2
OVERLAY_BUF_RESTORE_PROGRAM:       equ 63985
OVERLAY_BUF_MARK_PRESENT:          equ 63987
OVERLAY_BUF_SNAPSHOT_DISPATCH:     equ 63988
OVERLAY_BUF_ORIGINAL_PRESENT:      equ 63989
OVERLAY_BUF_RESTORE_CLEAR:         equ 63986

overlay_feature_active: db 0

; App-process initialization is RAM-only. Process entry has not yet acquired
; or globally cleared the VDP registry, so it must not touch buffer IDs that
; could still belong to the caller. Every begin transaction clears and rebuilds
; the application-owned controls after resource ownership is established.
overlay_feature_process_init:
    xor a
    ld (overlay_feature_active),a
    ret
; end overlay_feature_process_init

; Snapshot the incoming feature-variable state and enable hardware sprites.
;
; outputs: carry set after a new transaction is submitted; carry clear when a
;          transaction is already active
overlay_feature_begin:
    ld a,(overlay_feature_active)
    or a
    ret nz
; Command 0 appends a block, so always begin from empty app-owned controls.
    call overlay_feature_clear_control_buffers

    ld hl,OVERLAY_BUF_RESTORE_PROGRAM
    ld bc,overlay_restore_program_end-overlay_restore_program
    ld de,overlay_restore_program
    call vdu_write_block_to_buffer

    ld hl,OVERLAY_BUF_ORIGINAL_PRESENT
    ld bc,1
    ld de,overlay_initial_presence
    call vdu_write_block_to_buffer

    ld hl,OVERLAY_BUF_RESTORE_CLEAR
    ld bc,overlay_restore_clear_end-overlay_restore_clear
    ld de,overlay_restore_clear
    call vdu_write_block_to_buffer

    ld hl,OVERLAY_BUF_MARK_PRESENT
    ld bc,overlay_mark_present_end-overlay_mark_present
    ld de,overlay_mark_present
    call vdu_write_block_to_buffer

    ld hl,OVERLAY_BUF_SNAPSHOT_DISPATCH
    ld bc,overlay_snapshot_dispatch_end-overlay_snapshot_dispatch
    ld de,overlay_snapshot_dispatch
    call vdu_write_block_to_buffer

; If variable 2 exists, latch presence and copy its full value into SET.
    ld hl,OVERLAY_BUF_SNAPSHOT_DISPATCH
    call vdu_call_buffer

; Presence, including value zero, enables the experimental hardware path.
    ld hl,overlay_feature_enable
    ld bc,overlay_feature_enable_end-overlay_feature_enable
    rst.lil 18h

    ld a,1
    ld (overlay_feature_active),a
    scf
    ret
; end overlay_feature_begin

; Restore the exact incoming state, then release every control buffer.
;
; Overlay callers must first hide/detach owned sprites, restore the active
; prefix, and demote the owned slots to software. A standalone fixture may then
; release fixture-owned loose bitmap buffers; the game retains its immutable
; AGNB bitmap assets for later attract/game-over entries.
; outputs: carry set after an active transaction is restored; carry clear when
;          no transaction is active
overlay_feature_end:
    ld a,(overlay_feature_active)
    or a
    ret z
; Buffered calls complete before following clear commands release the program.
    ld hl,overlay_restore_dispatch
    ld bc,overlay_restore_dispatch_end-overlay_restore_dispatch
    rst.lil 18h
    call overlay_feature_clear_control_buffers
    xor a
    ld (overlay_feature_active),a
    scf
    ret
; end overlay_feature_end

overlay_feature_clear_control_buffers:
    ld hl,OVERLAY_BUF_RESTORE_PROGRAM
    call vdu_clear_buffer
    ld hl,OVERLAY_BUF_MARK_PRESENT
    call vdu_clear_buffer
    ld hl,OVERLAY_BUF_SNAPSHOT_DISPATCH
    call vdu_clear_buffer
    ld hl,OVERLAY_BUF_ORIGINAL_PRESENT
    call vdu_clear_buffer
    ld hl,OVERLAY_BUF_RESTORE_CLEAR
    call vdu_clear_buffer
    ret
; end overlay_feature_clear_control_buffers

; The SET program receives the original value only when it was present.
overlay_restore_program:
    db 23,0,0f8h
    dw OVERLAY_FEATURE_VARIABLE
    dw 0
overlay_restore_program_end:

overlay_restore_clear:
    db 23,0,0f9h
    dw OVERLAY_FEATURE_VARIABLE
overlay_restore_clear_end:

overlay_initial_presence: db 0

; Latch presence separately and copy the incoming value into SET bytes 5..6.
overlay_mark_present:
    db 23,0,0a0h
    dw OVERLAY_BUF_ORIGINAL_PRESENT
    db 5,2
    dw 0
    db 1
    db 23,0,0a0h
    dw OVERLAY_BUF_RESTORE_PROGRAM
    db 48,080h
    dw 5
    dw OVERLAY_FEATURE_VARIABLE
overlay_mark_present_end:

; Buffer 63988: call mark-present only when variable 2 truly exists. Operation
; 40h means Exists with the VDP-variable operand selector.
overlay_snapshot_dispatch:
    db 23,0,0a0h
    dw OVERLAY_BUF_MARK_PRESENT
    db 6,040h
    dw OVERLAY_FEATURE_VARIABLE
overlay_snapshot_dispatch_end:

; Ordinary byte equality selects exactly one restore branch.
overlay_restore_dispatch:
    db 23,0,0a0h
    dw OVERLAY_BUF_RESTORE_CLEAR
    db 6,2
    dw OVERLAY_BUF_ORIGINAL_PRESENT,0
    db 0
    db 23,0,0a0h
    dw OVERLAY_BUF_RESTORE_PROGRAM
    db 6,2
    dw OVERLAY_BUF_ORIGINAL_PRESENT,0
    db 1
overlay_restore_dispatch_end:

overlay_feature_enable:
    db 23,0,0f8h
    dw OVERLAY_FEATURE_VARIABLE
    dw 0
overlay_feature_enable_end:

; Freeze command packet sizes and the two patch offsets used above.
    .if overlay_restore_program_end-overlay_restore_program-7
        dl overlay_restore_program_size_mismatch
    .endif
    .if overlay_mark_present_end-overlay_mark_present-21
        dl overlay_mark_present_size_mismatch
    .endif
    .if overlay_snapshot_dispatch_end-overlay_snapshot_dispatch-9
        dl overlay_snapshot_dispatch_size_mismatch
    .endif
    .if overlay_feature_enable_end-overlay_feature_enable-7
        dl overlay_feature_enable_size_mismatch
    .endif
    .if overlay_restore_dispatch_end-overlay_restore_dispatch-24
        dl overlay_restore_dispatch_size_mismatch
    .endif
