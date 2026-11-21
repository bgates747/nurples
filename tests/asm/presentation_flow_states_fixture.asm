; Native transition fixture for the cooperative presentation states.

    assume adl=1
    org 040000h
    jp fixture_start
    include "../../src/asm/mus_config.inc"

STATE_NEW_EVENT_START: equ 1
STATE_NEW_NEW_GAME: equ 2
STATE_NEW_LEVEL_ENTER: equ 3
game_fault_presentation: equ 1
game_fault_session_init: equ 2
PLAYFIELD_CHECK_ATTRACT: equ 2
PLAYFIELD_CHECK_START: equ 3

game_state: dl 0
game_fault_flags: db 0
game_exit_requested: db 0
attract_lifecycle_active: db 0
attract_dismiss_requested: db 0
game_over_lifecycle_active: db 0
game_over_dismiss_requested: db 0
fixture_attract_enter_ok: db 1
fixture_attract_gate: db INPUT_GATE_WAIT_RELEASE
fixture_game_over_enter_ok: db 1
fixture_game_over_dismiss: db 0
fixture_input_down: db 0
fixture_input_choice: db 0
fixture_joystick: db 0
fixture_session_calls: db 0
fixture_prepare_calls: db 0
fixture_reset_calls: db 0
fixture_shutdown_order: db 0

    include "../../src/asm/input_gate.inc"
    include "../../src/asm/start_choice_controller.inc"
    include "../../src/asm/presentation_flow_states.inc"

FIXTURE_PASS: equ 02ah

fixture_start:
    ld sp,05ff00h
    call presentation_flow_process_init

; Shutdown is safe while inactive and still visits renderer, orphan feature,
; controller and choice cleanup in that order. The first three are observed;
; the real choice controller's cleared fields are checked below.
    call presentation_flow_shutdown
    ld a,(fixture_shutdown_order)
    cp 3
    jp nz,fixture_fail
    ld a,(start_choice_result)
    or a
    jp nz,fixture_fail

; Loading hands off to controller choice while the splash is still present.
    call game_start_config_begin
    jp nc,fixture_fail
    ld hl,(game_state)
    ld de,game_start_config
    or a
    sbc hl,de
    jp nz,fixture_fail
    ld a,(fixture_prepare_calls)
    or a
    jp nz,fixture_fail

; The MOS entry key is held: the configuration gate must ignore it until up.
    ld a,1
    ld (fixture_input_down),a
    ld a,START_CHOICE_YES
    ld (fixture_input_choice),a
    call game_start_config
    ld a,(fixture_joystick)
    or a
    jp nz,fixture_fail
    ld a,(game_start_config_active)
    cp 1
    jp nz,fixture_fail

; A released then fresh Y configures joystick and prepares ATTRACT. It does
; not initialize gameplay.
    xor a
    ld (fixture_input_down),a
    ld (fixture_input_choice),a
    call game_start_config
    ld a,1
    ld (fixture_input_down),a
    ld a,START_CHOICE_YES
    ld (fixture_input_choice),a
    call game_start_config
    ld a,(fixture_joystick)
    cp 1
    jp nz,fixture_fail
    ld hl,(game_state)
    ld de,game_attract
    or a
    sbc hl,de
    jp nz,fixture_fail
    ld a,(fixture_prepare_calls)
    cp 1
    jp nz,fixture_fail
    ld a,(fixture_reset_calls)
    cp 1
    jp nz,fixture_fail

; The choice key is still down at ATTRACT entry. Only the next released then
; fresh edge initializes a new session.
    call game_attract
    ld a,(fixture_session_calls)
    or a
    jp nz,fixture_fail
    xor a
    ld (fixture_input_down),a
    call game_attract
    ld a,(fixture_session_calls)
    or a
    jp nz,fixture_fail
    inc a
    ld (fixture_input_down),a
    call game_attract
    ld a,(fixture_session_calls)
    cp 1
    jp nz,fixture_fail
    ld hl,(game_state)
    ld de,fixture_game_playing
    or a
    sbc hl,de
    jp nz,fixture_fail

; GAME OVER continues directly with the already selected controller.
    ld a,1
    ld (fixture_game_over_dismiss),a
    call game_over_bounded
    ld hl,(game_state)
    ld de,fixture_game_playing
    or a
    sbc hl,de
    jp nz,fixture_fail
    ld a,(fixture_session_calls)
    cp 2
    jp nz,fixture_fail
    ld a,(fixture_joystick)
    cp 1
    jp nz,fixture_fail

; A new entry also permits N for keyboard mode.
    call game_start_config_begin
    xor a
    ld (fixture_input_down),a
    call game_start_config
    ld a,1
    ld (fixture_input_down),a
    ld a,START_CHOICE_NO
    ld (fixture_input_choice),a
    call game_start_config
    ld a,(fixture_joystick)
    or a
    jp nz,fixture_fail
    ld a,(fixture_session_calls)
    cp 2
    jp nz,fixture_fail

; A failed overlay acquisition is terminal and never publishes ATTRACT.
    xor a
    ld (fixture_attract_enter_ok),a
    ld (game_exit_requested),a
    ld (game_fault_flags),a
    call game_attract_begin
    jp c,fixture_fail
    ld a,(game_exit_requested)
    cp 1
    jp nz,fixture_fail
    ld a,(game_fault_flags)
    bit game_fault_presentation,a
    jp z,fixture_fail

    ld a,FIXTURE_PASS
    out (0),a

fixture_fail:
    ld a,0feh
    out (0),a

overlay_feature_process_init:
presentation_renderer_process_init:
attract_lifecycle_process_init:
game_over_lifecycle_process_init:
start_choice_exit_stub:
    ret

presentation_renderer_exit:
    ld a,1
    ld (fixture_shutdown_order),a
    ret

overlay_feature_end:
    ld a,(fixture_shutdown_order)
    cp 1
    jp nz,fixture_fail
    inc a
    ld (fixture_shutdown_order),a
    ret

presentation_overlay_exit:
    ld a,(fixture_shutdown_order)
    cp 2
    jp nz,fixture_fail
    inc a
    ld (fixture_shutdown_order),a
    ret

game_prepare_playfield:
    ld hl,fixture_prepare_calls
    inc (hl)
    scf
    ret

reset_runtime_tables:
    ld hl,fixture_reset_calls
    inc (hl)
    ret

attract_lifecycle_enter:
    ld a,(fixture_attract_enter_ok)
    or a
    ret z
    ld iy,fixture_attract_gate
    call input_gate_reset
    scf
    ret

attract_lifecycle_update:
    ld a,(fixture_input_down)
    ld iy,fixture_attract_gate
    jp input_gate_update

attract_lifecycle_exit:
    ret

game_over_lifecycle_enter:
    ld a,(fixture_game_over_enter_ok)
    or a
    ret z
    scf
    ret

game_over_lifecycle_update:
    ld a,(fixture_game_over_dismiss)
    or a
    ret z
    scf
    ret

game_over_lifecycle_exit:
    ret

presentation_input_poll:
    ld a,(fixture_input_choice)
    ld c,a
    ld a,(fixture_input_down)
    ret

player_joystick_enable:
    ld a,1
    ld (fixture_joystick),a
    ret

player_joystick_disable:
    xor a
    ld (fixture_joystick),a
    ret

game_new_session_init:
    ld hl,fixture_session_calls
    inc (hl)
    ld hl,fixture_game_playing
    ld (game_state),hl
    scf
    ret

fixture_game_playing:
    ret

vdu_move_cursor:
    ret

; Skip printInline's embedded zero-terminated bytes in the same manner as the
; production helper, without requiring the VDU character path in this fixture.
printInline:
    pop hl
@scan:
    ld a,(hl)
    inc hl
    or a
    jp nz,@scan
    push hl
    ret
