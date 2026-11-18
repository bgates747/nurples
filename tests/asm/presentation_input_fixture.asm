; Assembly boundary for the one-frame MOS presentation-input sampler.

    assume adl=1
    org 040000h
    jp fixture_start

    include "../../src/asm/mos_api.inc"
    include "../../src/asm/input_gate.inc"
    include "../../src/asm/start_choice_controller.inc"
    include "../../src/asm/presentation_input.inc"

fixture_start:
    call presentation_input_poll
    ret
