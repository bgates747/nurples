; Device-independent overlay animation and dismissal policy for TITLE/ATTRACT
; and GAME_OVER. Hardware-sprite rendering remains behind its separate gate.

PRESENTATION_ATTRACT: equ 0
PRESENTATION_GAME_OVER: equ 1

PRESENTATION_DIRTY_TITLE: equ %00000001
PRESENTATION_DIRTY_PROMPT: equ %00000010
PRESENTATION_DIRTY_ALL: equ PRESENTATION_DIRTY_TITLE+PRESENTATION_DIRTY_PROMPT

presentation_title_period: equ 8
presentation_prompt_period: equ 30

presentation_mode: db PRESENTATION_ATTRACT
presentation_title_frame: db 0
presentation_title_timer: db presentation_title_period
presentation_prompt_visible: db 1
presentation_prompt_timer: db presentation_prompt_period
presentation_dirty: db PRESENTATION_DIRTY_ALL
presentation_input_gate: db INPUT_GATE_WAIT_RELEASE
presentation_input_sample: db 0

; inputs: A = PRESENTATION_ATTRACT or PRESENTATION_GAME_OVER
; outputs: carry set for a valid initialized mode; carry clear for invalid mode
presentation_overlay_enter:
    cp PRESENTATION_GAME_OVER+1
    jp nc,@invalid
    ld (presentation_mode),a
    xor a
    ld (presentation_title_frame),a
    ld a,presentation_title_period
    ld (presentation_title_timer),a
    ld a,1
    ld (presentation_prompt_visible),a
    ld a,presentation_prompt_period
    ld (presentation_prompt_timer),a
    ld a,PRESENTATION_DIRTY_ALL
    ld (presentation_dirty),a
    ld iy,presentation_input_gate
    call input_gate_reset
    scf
    ret
@invalid:
    xor a
    ret
; end presentation_overlay_enter

; Advance one bounded presentation frame.
; inputs: A = normalized any-key-down sample
; outputs: carry set exactly once for a fresh press after release
;          title/prompt state and dirty bits updated for the renderer
; destroys: a,hl,iy
presentation_overlay_update:
    ld (presentation_input_sample),a
    xor a
    ld (presentation_dirty),a

    ld a,(presentation_mode)
    cp PRESENTATION_GAME_OVER
    jp nz,@prompt
    ld hl,presentation_title_timer
    dec (hl)
    jp nz,@prompt
    ld a,presentation_title_period
    ld (presentation_title_timer),a
    ld a,(presentation_title_frame)
    xor %00000001
    ld (presentation_title_frame),a
    ld a,(presentation_dirty)
    or PRESENTATION_DIRTY_TITLE
    ld (presentation_dirty),a

@prompt:
    ld hl,presentation_prompt_timer
    dec (hl)
    jp nz,@input
    ld a,presentation_prompt_period
    ld (presentation_prompt_timer),a
    ld a,(presentation_prompt_visible)
    xor %00000001
    ld (presentation_prompt_visible),a
    ld a,(presentation_dirty)
    or PRESENTATION_DIRTY_PROMPT
    ld (presentation_dirty),a

@input:
    ld iy,presentation_input_gate
    ld a,(presentation_input_sample)
    jp input_gate_update
; end presentation_overlay_update

; Reset volatile ownership after the hardware renderer has detached resources.
presentation_overlay_exit:
    xor a
    ld (presentation_title_frame),a
    ld (presentation_prompt_visible),a
    ld (presentation_dirty),a
    ld (presentation_input_sample),a
    ld iy,presentation_input_gate
    jp input_gate_reset
; end presentation_overlay_exit
