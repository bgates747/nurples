; Cooperative global presentation-flow states.
;
; Integrated after the standalone hardware-sprite qualifier passed. It owns
; only global phase transitions. Device sampling, overlay animation/rendering,
; actor behavior and session initialization stay in their bounded owners.

game_start_config_active: db 0

presentation_flow_process_init:
    call overlay_feature_process_init
    call presentation_renderer_process_init
    call attract_lifecycle_process_init
    call game_over_lifecycle_process_init
    call start_choice_exit
    xor a
    ld (game_start_config_active),a
    ret
; end presentation_flow_process_init

; Process-exit cleanup for Escape, state-requested exit, and setup failure.
; Restore the hardware feature transaction before the ordinary main_end path
; resets sprites or deletes the control programs. Every callee is idempotent.
presentation_flow_shutdown:
    call presentation_renderer_exit
; Cover the fail-closed partial-ownership case where the feature transaction is
; active but the renderer never published its active byte.
    call overlay_feature_end
    call presentation_overlay_exit
    call start_choice_exit
    xor a
    ld (attract_lifecycle_active),a
    ld (attract_dismiss_requested),a
    ld (game_over_lifecycle_active),a
    ld (game_over_dismiss_requested),a
    ld (game_start_config_active),a
    ret
; end presentation_flow_shutdown

; Enter the first autonomous title after the one-time resource load. Ordinary
; sprite slots must be defined before the renderer publishes slots 20/21.
; outputs: carry set after publishing game_attract; carry clear on fail-closed
game_attract_begin:
    call game_prepare_playfield
    call reset_runtime_tables
    call attract_lifecycle_enter
    jp nc,presentation_flow_fail
    ld hl,game_attract
    ld (game_state),hl
    scf
    ret
; end game_attract_begin

; Bounded TITLE/ATTRACT state. The main loop supplies the sole VBlank.
game_attract:
    call attract_lifecycle_update
    ret nc
    call attract_lifecycle_exit
    jp game_start_config_begin
; end game_attract

; Draw the temporary configuration boundary once. Opaque regular text is
; deliberate here; the autonomous attract prompt has already been detached.
game_start_config_begin:
    call start_choice_enter
    ld c,21
    ld b,24
    call vdu_move_cursor
    call printInline
    asciz "ENABLE JOYSTICK (Y/N)?"
    ld a,1
    ld (game_start_config_active),a
    ld hl,game_start_config
    ld (game_state),hl
    scf
    ret
; end game_start_config_begin

; One keyboard snapshot and one choice-controller update per outer frame.
game_start_config:
    ld a,(game_start_config_active)
    or a
    jp nz,@poll
    call game_start_config_begin
    ret
@poll:
    call presentation_input_poll
    call start_choice_update
    ret nc
    cp START_CHOICE_YES
    jp z,@joystick_yes
    call player_joystick_disable
    jp @selected
@joystick_yes:
    call player_joystick_enable
@selected:
    call start_choice_exit
    xor a
    ld (game_start_config_active),a
; Redraw the canonical surface, then initialize the new session entirely from
; resident assets. Failure is terminal for this invocation; silently returning
; to game_init would violate the no-reload lifecycle and could loop forever.
    call game_prepare_playfield
    call game_new_session_init
    ret c
    ld hl,game_fault_flags
    set game_fault_session_init,(hl)
    ld a,1
    ld (game_exit_requested),a
    xor a
    ret
; end game_start_config

; Replacement bounded GAME OVER state. Entry is idempotent so the state itself
; remains recoverable if a caller publishes it one frame before setup.
game_over_bounded:
    call game_over_lifecycle_enter
    jp nc,presentation_flow_fail
    call game_over_lifecycle_update
    ret nc
    call game_over_lifecycle_exit
    jp game_start_config_begin
; end game_over_bounded

; Fail closed without reloading VDP resources or repeatedly retrying a hardware
; transaction whose ownership is no longer proven.
presentation_flow_fail:
    ld hl,game_fault_flags
    set game_fault_presentation,(hl)
    ld a,1
    ld (game_exit_requested),a
    xor a
    ret
; end presentation_flow_fail
