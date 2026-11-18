; Exact feature-variable transaction for prospective hardware-sprite overlays.
;
; The production game includes this only after the standalone Alexe R07 gate.
; It implements the qualified transaction in docs/hardware-sprite-overlays.md.
; The VDP itself snapshots both the presence and 16-bit value of variable 2, so
; no CPU-side response packet is required.
;
; prerequisites:
;   vdu_write_block_to_buffer, vdu_call_buffer, vdu_clear_buffer

OVERLAY_FEATURE_VARIABLE:          equ 2
OVERLAY_BUF_RESTORE_PROGRAM:       equ 63985
OVERLAY_BUF_MARK_PRESENT:          equ 63987
OVERLAY_BUF_SNAPSHOT_DISPATCH:     equ 63988

overlay_feature_active: db 0

; App-process initialization is RAM-only. Process entry has not yet acquired
; or globally cleared the VDP registry, so it must not touch buffer IDs that
; could still belong to the caller. Every begin transaction clears and rebuilds
; the three application-owned controls after resource ownership is established.
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

    ld hl,OVERLAY_BUF_MARK_PRESENT
    ld bc,overlay_mark_present_end-overlay_mark_present
    ld de,overlay_mark_present
    call vdu_write_block_to_buffer

    ld hl,OVERLAY_BUF_SNAPSHOT_DISPATCH
    ld bc,overlay_snapshot_dispatch_end-overlay_snapshot_dispatch
    ld de,overlay_snapshot_dispatch
    call vdu_write_block_to_buffer

; The prepared program defaults to CLEAR. If variable 2 exists, the dispatcher
; patches its opcode to SET and copies the complete 16-bit incoming value.
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
    ld hl,OVERLAY_BUF_RESTORE_PROGRAM
    call vdu_call_buffer
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
    ret
; end overlay_feature_clear_control_buffers

; Buffer 63985: seven-byte prepared restore program. It begins as a five-byte
; CLEAR command followed by two harmless NULs. If the incoming variable exists,
; overlay_mark_present changes byte 2 to F8 (SET) and fills bytes 5..6 with the
; original 16-bit value, making the same seven bytes a complete SET command.
overlay_restore_program:
    db 23,0,0f9h
    dw OVERLAY_FEATURE_VARIABLE
    dw 0
overlay_restore_program_end:

; Buffer 63987: patch the prepared opcode to SET, then copy the complete 16-bit
; incoming variable value into bytes 5..6 of the prepared restore command.
overlay_mark_present:
    db 23,0,0a0h
    dw OVERLAY_BUF_RESTORE_PROGRAM
    db 5,2
    dw 2
    db 0f8h
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
