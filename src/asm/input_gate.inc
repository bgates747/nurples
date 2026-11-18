; Frame-bounded release-then-press input gate.
;
; This module is deliberately independent of MOS keyboard sampling. Callers
; reduce their input source to an any-key-down byte, then update one byte of
; lifecycle-owned state. TITLE, GAME_OVER, replay interruption, and future
; controller-selection screens can therefore share edge semantics without a
; blocking wait or a global last-key value.

INPUT_GATE_WAIT_RELEASE: equ 0
INPUT_GATE_ARMED:        equ 1

; Reset one gate. A newly entered state always waits for a complete release,
; even if no key appears to be held in the transition frame.
;
; inputs:  IY = address of one gate-state byte
; outputs: (IY+0) = INPUT_GATE_WAIT_RELEASE, carry clear
; destroys: A, flags
input_gate_reset:
    xor a
    ld (iy+0),a
    ret
; end input_gate_reset

; Advance one gate by one input sample.
;
; inputs:  IY = address of one gate-state byte
;          A = zero when all keys are up, nonzero when any key is down
; outputs: carry set exactly once for a fresh press observed after release;
;          carry clear otherwise
; destroys: A, flags
input_gate_update:
    or a
    jp z,@released
    ld a,(iy+0)
    cp INPUT_GATE_ARMED
    jp z,@fire
; A held entry key cannot arm or fire the gate.
    xor a
    ret
@released:
    ld a,INPUT_GATE_ARMED
    ld (iy+0),a
    xor a
    ret
@fire:
; Consume the edge immediately. Continued key-down samples cannot retrigger;
; another event requires another complete release.
    xor a
    ld (iy+0),a
    scf
    ret
; end input_gate_update
