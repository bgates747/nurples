; Syntax/link fixture for the currently unintegrated hardware-overlay feature
; transaction. Runtime VDP qualification is intentionally a separate fixture.

    assume adl=1
    org 040000h
    jp fixture_start

; Minimal dependency surface used by hardware_overlay_feature.inc.
vdu_write_block_to_buffer:
    ret

vdu_call_buffer:
    ret

vdu_clear_buffer:
    ret

    include "../../src/asm/hardware_overlay_feature.inc"

fixture_start:
    ld sp,05ff00h
    call overlay_feature_process_init
    call overlay_feature_begin
    call overlay_feature_end
    ret
