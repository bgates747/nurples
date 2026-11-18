; Deterministic pure-CPU qualification for overlay animation/input policy.

    assume adl=1
    org 040000h
    jp fixture_start

    include "../../src/asm/input_gate.inc"
    include "../../src/asm/presentation_overlay_controller.inc"

FIXTURE_PASS: equ 02ah
fixture_case: db 0

fixture_start:
    ld sp,05ff00h

; GAME OVER initializes both overlays and refuses a held entry key.
    ld a,1
    ld (fixture_case),a
    ld a,PRESENTATION_GAME_OVER
    call presentation_overlay_enter
    jp nc,fixture_fail
    ld a,1
    call presentation_overlay_update
    jp c,fixture_fail

; Release arms; the next fresh press fires exactly once.
    ld a,2
    ld (fixture_case),a
    xor a
    call presentation_overlay_update
    jp c,fixture_fail
    ld a,1
    call presentation_overlay_update
    jp nc,fixture_fail
    ld a,1
    call presentation_overlay_update
    jp c,fixture_fail

; Fresh entry: title flips after eight frames, prompt stays visible.
    ld a,3
    ld (fixture_case),a
    ld a,PRESENTATION_GAME_OVER
    call presentation_overlay_enter
    ld b,presentation_title_period
@title_loop:
    push bc
    ld a,1 ; held input is irrelevant to animation and cannot dismiss
    call presentation_overlay_update
    jp c,fixture_fail
    pop bc
    djnz @title_loop
    ld a,(presentation_title_frame)
    cp 1
    jp nz,fixture_fail
    ld a,(presentation_dirty)
    and PRESENTATION_DIRTY_TITLE
    jp z,fixture_fail
    ld a,(presentation_prompt_visible)
    cp 1
    jp nz,fixture_fail

; Attract never animates the absent GAME OVER title; prompt toggles at 30.
    ld a,4
    ld (fixture_case),a
    ld a,PRESENTATION_ATTRACT
    call presentation_overlay_enter
    ld b,presentation_prompt_period
@prompt_loop:
    push bc
    ld a,1
    call presentation_overlay_update
    jp c,fixture_fail
    pop bc
    djnz @prompt_loop
    ld a,(presentation_title_frame)
    or a
    jp nz,fixture_fail
    ld a,(presentation_prompt_visible)
    or a
    jp nz,fixture_fail
    ld a,(presentation_dirty)
    cp PRESENTATION_DIRTY_PROMPT
    jp nz,fixture_fail

; Exit clears visible/dirty state and revokes any armed input edge.
    ld a,5
    ld (fixture_case),a
    call presentation_overlay_exit
    ld a,(presentation_prompt_visible)
    or a
    jp nz,fixture_fail
    ld a,(presentation_dirty)
    or a
    jp nz,fixture_fail
    ld a,(presentation_input_gate)
    cp INPUT_GATE_WAIT_RELEASE
    jp nz,fixture_fail

; Invalid modes fail without publishing a new owner.
    ld a,6
    ld (fixture_case),a
    ld a,2
    call presentation_overlay_enter
    jp c,fixture_fail

    ld a,FIXTURE_PASS
    out (0),a

fixture_fail:
    ld a,(fixture_case)
    or a
    jp nz,@emit
    ld a,0feh
@emit:
    out (0),a
