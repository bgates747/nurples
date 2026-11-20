; Assembly boundary for both ordinary and scrolling presentation renderers.

    assume adl=1
    org 040000h
    jp fixture_start

BUF_GAME_OVER_0: equ 63990
BUF_GAME_OVER_1: equ 63991
BUF_PRESS_ANY_KEY_TO_CONTINUE: equ 63992
BUF_PRESS_ANY_KEY_TO_START: equ 63993
origin_left: equ 128
origin_top: equ 48

    include "../../src/asm/game_over_review_config.inc"
    include "../../src/asm/input_gate.inc"
    include "../../src/asm/presentation_overlay_controller.inc"
    include "../../src/asm/hardware_overlay_feature.inc"
    include "../../src/asm/presentation_overlay_renderer.inc"

fixture_start:
    call overlay_feature_process_init
    call presentation_renderer_process_init
    ld a,PRESENTATION_ATTRACT
    call presentation_overlay_enter
    call presentation_renderer_enter
    call presentation_renderer_update
    call presentation_renderer_exit
    call presentation_overlay_exit
    ld a,PRESENTATION_GAME_OVER
    call presentation_overlay_enter
    call presentation_renderer_enter
    call presentation_renderer_update
    call presentation_renderer_exit
    call presentation_overlay_exit
    ret

vdu_write_block_to_buffer:
    ret
vdu_call_buffer:
    ret
vdu_clear_buffer:
    ret
vdu_sprite_select:
    ret
vdu_sprite_clear_frames:
    ret
vdu_sprite_add_buff:
    ret
vdu_sprite_set_gcol:
    ret
vdu_sprite_mark_hardware:
    ret
vdu_sprite_mark_software:
    ret
vdu_sprite_move_abs:
    ret
vdu_sprite_select_frame:
    ret
vdu_sprite_show:
    ret
vdu_sprite_hide:
    ret
vdu_sprite_activate:
    ret
set_gfx_playing_field:
    ret
vdu_buff_select:
    ret
vdu_plot_bmp:
    ret
