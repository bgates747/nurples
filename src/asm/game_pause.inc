; Screenshot pause. The active gameplay handler stays published; suppress
; its entire frame rather than changing or saving actor/transition state.
; MOS held-key map: P is bit 7 of byte 6 (independent of case and key repeat).
game_pause_key_byte: equ 6
game_pause_key_bit: equ 7
; MOS keyboard.asm: KEY(102) = zero-based bit 101 (byte 12, bit 5).
music_key_byte: equ 12
music_key_bit: equ 5
game_pause_active: db 0
game_pause_key_gate: db INPUT_GATE_WAIT_RELEASE

game_pause_process_init:
    xor a
    ld (game_pause_active),a
    ld iy,game_pause_key_gate
    jp input_gate_reset
; end game_pause_process_init

; Before any timestamp or gameplay advance. Carry means skip the entire frame.
game_pause_poll:
    .if mus_all50
    MOSCALL mos_getkbmap
    push ix
    call lp_sample_m
    pop ix
    .endif
    ld hl,(game_state)
    ld de,game_playing
    or a
    sbc hl,de
    jp z,@sample
    ld hl,(game_state)
    ld de,game_level_transition
    or a
    sbc hl,de
    jp nz,game_pause_process_init ; a pre-start P cannot pause the first frame
@sample:
    .if 1-mus_all50
    MOSCALL mos_getkbmap
    .endif
    xor a
    bit game_pause_key_bit,(ix+game_pause_key_byte)
    jp z,game_pause_sample
    inc a
; end game_pause_poll

; A = normalized P held. Carry set while paused, even without a new edge.
game_pause_sample:
    ld iy,game_pause_key_gate
    call input_gate_update
    jp nc,@status
    ld a,(game_pause_active)
    xor 1
    ld (game_pause_active),a
    call z,timestamp_rebase
@status:
    ld a,(game_pause_active)
    or a
    ret z
    scf
    ret
; end game_pause_sample
