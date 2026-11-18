; Frame-bounded start-flow configuration controller.
;
; Device code reduces one outer frame to an any-key-down sample and an optional
; normalized Y/N choice. The controller requires a complete release after the
; attract dismissal key, then accepts only a fresh Y or N press. Other fresh
; keys are consumed and require another release; no routine blocks or polls.

START_CHOICE_NONE: equ 0
START_CHOICE_YES:  equ 1
START_CHOICE_NO:   equ 2

start_choice_gate: db INPUT_GATE_WAIT_RELEASE
start_choice_result: db START_CHOICE_NONE
start_choice_sample: db START_CHOICE_NONE

; Enter a new bounded choice. The prior session choice is deliberately not
; inherited: every new game asks afresh, matching the legacy behavior.
start_choice_enter:
    xor a
    ld (start_choice_result),a
    ld (start_choice_sample),a
    ld iy,start_choice_gate
    jp input_gate_reset
; end start_choice_enter

; Advance one outer frame.
; inputs: A = zero when every key is up, nonzero when any key is down
;         C = START_CHOICE_NONE/YES/NO for this frame's fresh key
; outputs: carry set exactly once when YES or NO is accepted
;          A = accepted normalized choice when carry is set
;          carry clear otherwise
; destroys: A,HL,IY; preserves BC/DE
start_choice_update:
    ld (start_choice_sample),a
    ld iy,start_choice_gate
    call input_gate_update
    jp nc,@waiting
    ld a,c
    cp START_CHOICE_YES
    jp z,@accepted
    cp START_CHOICE_NO
    jp z,@accepted
; An unrelated fresh key consumed the input edge. The gate already returned to
; WAIT_RELEASE, so a held key cannot become a later accidental choice.
@waiting:
    xor a
    ret
@accepted:
    ld (start_choice_result),a
    scf
    ret
; end start_choice_update

; Clear volatile ownership after the caller has consumed the result.
start_choice_exit:
    xor a
    ld (start_choice_result),a
    ld (start_choice_sample),a
    ld iy,start_choice_gate
    jp input_gate_reset
; end start_choice_exit
