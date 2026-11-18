; Standalone visual/runtime qualifier for the prospective Nurples hardware
; overlay path. This is deliberately not included by the production game.
;
; SD directory contents:
;   overlay.bin
;   game_over_0.rgba2
;   game_over_1.rgba2
;   press_any_key_to_continue.rgba2
;   press_any_key_to_start.rgba2

    assume adl=1
    org 040000h
    jp start
    align 64
    db "MOS",0,1

    include "../../src/asm/mos_api.inc"
    include "../../src/asm/macros.inc"
    include "../../src/asm/vdu_buffered_api.inc"
    include "../../src/asm/hardware_overlay_feature.inc"

OVERLAY_SPRITE_TITLE:       equ 20
OVERLAY_SPRITE_PROMPT:      equ 21
OVERLAY_ACTIVE_ACTORS:      equ 20
OVERLAY_ACTIVE_ALL:         equ 22
OVERLAY_BUF_TITLE_0:        equ 63990
OVERLAY_BUF_TITLE_1:        equ 63991
OVERLAY_BUF_PROMPT_CONTINUE: equ 63992
OVERLAY_BUF_PROMPT_START:   equ 63993
OVERLAY_BUF_DUMMY:          equ 63994
OVERLAY_BUF_PASS:           equ 63995
OVERLAY_BUF_FAIL:           equ 63996
OVERLAY_BUF_VERIFY:         equ 63997

FIXTURE_CASE_ABSENT:        equ 0
FIXTURE_CASE_ZERO:          equ 1
FIXTURE_CASE_BEEF:          equ 2
FIXTURE_CASE_COUNT:         equ 3
FIXTURE_CASE_FRAMES:        equ 360
FIXTURE_STATUS_FRAMES:      equ 90

start:
    push af
    push bc
    push de
    push ix
    push iy
    call fixture_main
    pop iy
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0
    ret

fixture_main:
    MOSCALL mos_sysvars
    ld a,(ix+sysvar_scrMode)
    ld (fixture_original_mode),a

    call fixture_clear_all_buffers
    ld a,20
    call fixture_set_mode
    xor a
    call fixture_set_scaling
    call fixture_cursor_off

    call fixture_create_dummy_bitmap
    ret nc
    call fixture_define_actor_prefix

    xor a
    ld (fixture_case),a
@case_loop:
    call fixture_prepare_case_input
    call fixture_draw_background
    call fixture_load_overlay_bitmaps
    jp nc,@load_failed
    call overlay_feature_process_init
    call overlay_feature_begin
    jp nc,@load_failed
    call fixture_define_overlay_sprites
    call fixture_run_overlay
    call fixture_destroy_overlay_sprites
    call overlay_feature_end
    jp nc,@load_failed
    call fixture_show_restore_result

    ld a,(fixture_case)
    inc a
    ld (fixture_case),a
    cp FIXTURE_CASE_COUNT
    jp c,@case_loop

    call fixture_clear_external_feature
    call fixture_sprite_reset
    call fixture_clear_all_buffers
    ld a,(fixture_original_mode)
    call fixture_set_mode
    call fixture_cursor_on
    ld hl,fixture_complete_message
    call fixture_print_string
    MOSCALL mos_getkey
    ret

@load_failed:
    call fixture_clear_external_feature
    call fixture_sprite_reset
    call fixture_clear_all_buffers
    ld a,(fixture_original_mode)
    call fixture_set_mode
    call fixture_cursor_on
    ld hl,fixture_failed_message
    call fixture_print_string
    MOSCALL mos_getkey
    ret

; Establish the three exact incoming states immediately before the transaction.
fixture_prepare_case_input:
    ld a,(fixture_case)
    or a
    jp z,fixture_clear_external_feature
    cp FIXTURE_CASE_ZERO
    jp z,@zero
    ld hl,fixture_feature_beef
    jp @send
@zero:
    ld hl,fixture_feature_zero
@send:
    ld bc,7
    rst.lil 18h
    ret

fixture_clear_external_feature:
    ld hl,fixture_feature_clear
    ld bc,5
    rst.lil 18h
    ret

fixture_feature_zero:  db 23,0,0f8h,2,0,0,0
fixture_feature_beef:  db 23,0,0f8h,2,0,0efh,0beh
fixture_feature_clear: db 23,0,0f9h,2,0

; Load the four raw RGBA2222 files and create bitmap objects at their frozen
; overlay buffer IDs. Carry is set only if all files open and complete.
fixture_load_overlay_bitmaps:
    ld hl,OVERLAY_BUF_TITLE_0
    ld iy,fixture_file_title_0
    ld bc,240
    ld de,64
    call fixture_load_bitmap
    ret nc
    ld hl,OVERLAY_BUF_TITLE_1
    ld iy,fixture_file_title_1
    ld bc,240
    ld de,64
    call fixture_load_bitmap
    ret nc
    ld hl,OVERLAY_BUF_PROMPT_CONTINUE
    ld iy,fixture_file_prompt_continue
    ld bc,208
    ld de,16
    call fixture_load_bitmap
    ret nc
    ld hl,OVERLAY_BUF_PROMPT_START
    ld iy,fixture_file_prompt_start
    ld bc,208
    ld de,16
    jp fixture_load_bitmap

fixture_file_title_0: db "game_over_0.rgba2",0
fixture_file_title_1: db "game_over_1.rgba2",0
fixture_file_prompt_continue: db "press_any_key_to_continue.rgba2",0
fixture_file_prompt_start: db "press_any_key_to_start.rgba2",0

; Define a transparent 1x1 software frame for every production-owned prefix
; slot. This lets the fixture test overlays at the real IDs 20/21 without
; pretending the ordinary actor prefix is absent.
fixture_create_dummy_bitmap:
    ld hl,OVERLAY_BUF_DUMMY
    ld bc,1
    ld de,fixture_transparent_pixel
    call vdu_write_block_to_buffer
    ld hl,OVERLAY_BUF_DUMMY
    call fixture_select_bitmap
    ld bc,1
    ld de,1
    ld a,1
    call fixture_create_bitmap
    scf
    ret
fixture_transparent_pixel: db 0

fixture_define_actor_prefix:
    xor a
@loop:
    push af
    call fixture_sprite_select
    call fixture_sprite_clear_frames
    ld hl,OVERLAY_BUF_DUMMY
    call fixture_sprite_add_buffer
    call fixture_sprite_mark_software
    call fixture_sprite_hide
    pop af
    inc a
    cp OVERLAY_ACTIVE_ACTORS
    jp c,@loop
    ld a,OVERLAY_ACTIVE_ACTORS
    jp fixture_sprite_activate

fixture_define_overlay_sprites:
; The VGA scanline compositor reads active Sprite objects asynchronously.
; Deactivate the global engine before changing frame vectors/hardware bits.
    xor a
    call fixture_sprite_activate
; ID 19 is a visible software-sprite ordering witness beneath the title. The
; higher-ID hardware title must cover its pixels on overlapping scanlines.
    ld a,19
    call fixture_sprite_select
    call fixture_sprite_clear_frames
    ld hl,OVERLAY_BUF_PROMPT_START
    call fixture_sprite_add_buffer
    call fixture_sprite_mark_software
    ld bc,152
    ld de,140
    call fixture_sprite_move_abs
    call fixture_sprite_show
    call fixture_sprite_update

    ld a,OVERLAY_SPRITE_TITLE
    call fixture_sprite_select
    call fixture_sprite_clear_frames
    ld hl,OVERLAY_BUF_TITLE_0
    call fixture_sprite_add_buffer
    ld hl,OVERLAY_BUF_TITLE_1
    call fixture_sprite_add_buffer
    xor a
    call fixture_sprite_set_gcol
    call fixture_sprite_mark_hardware
    ld bc,136
    ld de,116
    call fixture_sprite_move_abs
    xor a
    call fixture_sprite_select_frame
    call fixture_sprite_show

    ld a,OVERLAY_SPRITE_PROMPT
    call fixture_sprite_select
    call fixture_sprite_clear_frames
    ld hl,OVERLAY_BUF_PROMPT_CONTINUE
    call fixture_sprite_add_buffer
    ld hl,OVERLAY_BUF_PROMPT_START
    call fixture_sprite_add_buffer
    xor a
    call fixture_sprite_set_gcol
    call fixture_sprite_mark_hardware
    ld bc,152
    ld de,210
    call fixture_sprite_move_abs
    xor a
    call fixture_sprite_select_frame
    call fixture_sprite_show

    ld a,OVERLAY_ACTIVE_ALL
    jp fixture_sprite_activate

fixture_destroy_overlay_sprites:
; Detach before buffer release, reduce the active prefix, and demote both slots.
    ld a,OVERLAY_SPRITE_PROMPT
    call fixture_sprite_select
    call fixture_sprite_hide
    ld a,OVERLAY_SPRITE_TITLE
    call fixture_sprite_select
    call fixture_sprite_hide
; Hide is safe while active; frame detachment/demotion requires count zero.
    xor a
    call fixture_sprite_activate
    ld a,OVERLAY_SPRITE_PROMPT
    call fixture_sprite_select
    call fixture_sprite_clear_frames
    call fixture_sprite_mark_software
    ld a,OVERLAY_SPRITE_TITLE
    call fixture_sprite_select
    call fixture_sprite_clear_frames
    call fixture_sprite_mark_software
; The ordering witness shares a prompt bitmap, so restore its transparent
; production-prefix frame before any overlay buffer is released.
    ld a,19
    call fixture_sprite_select
    call fixture_sprite_hide
    call fixture_sprite_clear_frames
    ld hl,OVERLAY_BUF_DUMMY
    call fixture_sprite_add_buffer
    call fixture_sprite_mark_software
    ld a,OVERLAY_ACTIVE_ACTORS
    call fixture_sprite_activate
    ld hl,OVERLAY_BUF_TITLE_0
    call vdu_clear_buffer
    ld hl,OVERLAY_BUF_TITLE_1
    call vdu_clear_buffer
    ld hl,OVERLAY_BUF_PROMPT_CONTINUE
    call vdu_clear_buffer
    ld hl,OVERLAY_BUF_PROMPT_START
    jp vdu_clear_buffer

fixture_run_overlay:
    ld hl,FIXTURE_CASE_FRAMES
    ld (fixture_frames_left),hl
    xor a
    ld (fixture_animation_frame),a
@loop:
    call fixture_scroll_background
    ld a,(fixture_animation_frame)
    inc a
    ld (fixture_animation_frame),a

; Title alternates colour every 32 frames and nudges horizontally every 64,
; proving both frame selection and immediate absolute movement.
    push af
    ld a,OVERLAY_SPRITE_TITLE
    call fixture_sprite_select
    pop af
    push af
    rrca
    rrca
    rrca
    rrca
    rrca
    and 1
    call fixture_sprite_select_frame
    pop af
    bit 6,a
    ld bc,136
    jp z,@title_position
    ld bc,144
@title_position:
    ld de,116
    call fixture_sprite_move_abs

; Prompt frames alternate independently. It is hidden for half of each
; 64-frame interval, then shown again without a software refresh command.
    ld a,OVERLAY_SPRITE_PROMPT
    call fixture_sprite_select
    ld a,(fixture_animation_frame)
    rrca
    rrca
    rrca
    rrca
    rrca
    rrca
    and 1
    call fixture_sprite_select_frame
    ld a,(fixture_animation_frame)
    bit 5,a
    jp z,@show_prompt
    call fixture_sprite_hide
    jp @wait
@show_prompt:
    call fixture_sprite_show
@wait:
    call fixture_vblank
    ld hl,(fixture_frames_left)
    dec hl
    ld (fixture_frames_left),hl
    ld de,0
    or a
    sbc hl,de
    jp nz,@loop
    ret

; Print FAIL first, then let the VDP overwrite it with PASS only if the exact
; expected post-teardown variable condition succeeds.
fixture_show_restore_result:
    ld c,10
    ld b,1
    call fixture_move_cursor
    ld a,(fixture_case)
    or a
    ld hl,fixture_case_absent_message
    jp z,@case_ready
    cp FIXTURE_CASE_ZERO
    ld hl,fixture_case_zero_message
    jp z,@case_ready
    ld hl,fixture_case_beef_message
@case_ready:
    call fixture_print_string
    call fixture_write_status_programs
    ld hl,OVERLAY_BUF_FAIL
    call vdu_call_buffer
    call fixture_write_verifier
    ld hl,OVERLAY_BUF_VERIFY
    call vdu_call_buffer
    ld hl,FIXTURE_STATUS_FRAMES
    ld (fixture_frames_left),hl
@wait:
    call fixture_vblank
    ld hl,(fixture_frames_left)
    dec hl
    ld (fixture_frames_left),hl
    ld de,0
    or a
    sbc hl,de
    jp nz,@wait
    ld hl,OVERLAY_BUF_PASS
    call vdu_clear_buffer
    ld hl,OVERLAY_BUF_FAIL
    call vdu_clear_buffer
    ld hl,OVERLAY_BUF_VERIFY
    call vdu_clear_buffer
    ret

fixture_write_status_programs:
    ld hl,OVERLAY_BUF_PASS
    call vdu_clear_buffer
    ld hl,OVERLAY_BUF_PASS
    ld bc,fixture_status_pass_end-fixture_status_pass
    ld de,fixture_status_pass
    call vdu_write_block_to_buffer
    ld hl,OVERLAY_BUF_FAIL
    call vdu_clear_buffer
    ld hl,OVERLAY_BUF_FAIL
    ld bc,fixture_status_fail_end-fixture_status_fail
    ld de,fixture_status_fail
    jp vdu_write_block_to_buffer

fixture_write_verifier:
    ld hl,OVERLAY_BUF_VERIFY
    call vdu_clear_buffer
    ld a,(fixture_case)
    or a
    jp z,@absent
    cp FIXTURE_CASE_ZERO
    jp z,@zero
    ld bc,fixture_verify_beef_end-fixture_verify_beef
    ld de,fixture_verify_beef
    jp @write
@zero:
    ld bc,fixture_verify_zero_end-fixture_verify_zero
    ld de,fixture_verify_zero
    jp @write
@absent:
    ld bc,fixture_verify_absent_end-fixture_verify_absent
    ld de,fixture_verify_absent
@write:
    ld hl,OVERLAY_BUF_VERIFY
    jp vdu_write_block_to_buffer

; Cursor column 28, row 1; PASS and FAIL have equal width for overwrite.
fixture_status_pass: db 31,28,1,17,10,"PASS"
fixture_status_pass_end:
fixture_status_fail: db 31,28,1,17,12,"FAIL"
fixture_status_fail_end:
fixture_case_absent_message: db "ABSENT RESTORE",0
fixture_case_zero_message: db "ZERO RESTORE",0
fixture_case_beef_message: db "BEEF RESTORE",0

; Variable-specific conditional calls. Equal comparisons are false for an
; absent variable, so the zero/nonzero cases prove both presence and value.
fixture_verify_absent:
    db 23,0,0a0h
    dw OVERLAY_BUF_PASS
    db 6,041h
    dw OVERLAY_FEATURE_VARIABLE
fixture_verify_absent_end:
fixture_verify_zero:
    db 23,0,0a0h
    dw OVERLAY_BUF_PASS
    db 6,0c2h
    dw OVERLAY_FEATURE_VARIABLE
    dw 0
fixture_verify_zero_end:
fixture_verify_beef:
    db 23,0,0a0h
    dw OVERLAY_BUF_PASS
    db 6,0c2h
    dw OVERLAY_FEATURE_VARIABLE
    dw 0beefh
fixture_verify_beef_end:

; ---------------------------------------------------------------------------
; Minimal file/bitmap and drawing support
; ---------------------------------------------------------------------------

fixture_load_bitmap:
    ld (fixture_bitmap_id),hl
    ld (fixture_bitmap_width),bc
    ld (fixture_bitmap_height),de
    call fixture_load_buffer_from_file
    ret nc
    ld hl,(fixture_bitmap_id)
    call fixture_consolidate_buffer
    ld hl,(fixture_bitmap_id)
    call fixture_select_bitmap
    ld bc,(fixture_bitmap_width)
    ld de,(fixture_bitmap_height)
    ld a,1
    call fixture_create_bitmap
    scf
    ret

fixture_load_buffer_from_file:
    ld (fixture_bitmap_id),hl
    call vdu_clear_buffer
    push iy
    pop hl
    ld c,fa_read
    MOSCALL mos_fopen
    or a
    ret z
    ld (fixture_file_handle),a
@read:
    ld a,(fixture_file_handle)
    ld c,a
    ld hl,0b7e000h
    ld de,8192
    MOSCALL mos_fread
    ld hl,0
    xor a
    sbc hl,de
    jp z,@close
    push de
    pop bc
    ld de,0b7e000h
    ld hl,(fixture_bitmap_id)
    call vdu_write_block_to_buffer
    jp @read
@close:
    ld a,(fixture_file_handle)
    ld c,a
    MOSCALL mos_fclose
    scf
    ret

fixture_consolidate_buffer:
    ld (fixture_cmd_buffer_id),hl
    ld a,14
    ld (fixture_cmd_buffer_id+2),a
    ld hl,fixture_cmd_consolidate
    ld bc,6
    rst.lil 18h
    ret
fixture_cmd_consolidate: db 23,0,0a0h
fixture_cmd_buffer_id: dw 0
    db 14

fixture_select_bitmap:
    ld (fixture_select_bitmap_id),hl
    ld hl,fixture_select_bitmap_cmd
    ld bc,5
    rst.lil 18h
    ret
fixture_select_bitmap_cmd: db 23,27,020h
fixture_select_bitmap_id: dw 0
    db 0 ; absorbs HLU outside the five-byte VDU packet

fixture_create_bitmap:
    ld (fixture_create_width),bc
    ld (fixture_create_height),de
    ld (fixture_create_format),a
    ld hl,fixture_create_bitmap_cmd
    ld bc,8
    rst.lil 18h
    ret
fixture_create_bitmap_cmd: db 23,27,021h
fixture_create_width: dw 0
fixture_create_height: dw 0
fixture_create_format: db 0

fixture_draw_background:
    call fixture_clg
    ld a,0
    ld c,17
    call fixture_gcol
    ld bc,0
    ld de,0
    ld ix,511
    ld iy,383
    call fixture_plot_rect
    ld a,0
    ld c,21
    call fixture_gcol
    ld bc,0
    ld de,48
    ld ix,511
    ld iy,79
    call fixture_plot_rect
    ld a,0
    ld c,37
    call fixture_gcol
    ld bc,0
    ld de,176
    ld ix,511
    ld iy,207
    call fixture_plot_rect
    ret

fixture_scroll_background:
    ld hl,fixture_scroll_cmd
    ld bc,5
    rst.lil 18h
    ld a,(fixture_animation_frame)
    and 3
    add a,17
    ld c,a
    xor a
    call fixture_gcol
    ld bc,0
    ld de,0
    ld ix,511
    ld iy,0
    jp fixture_plot_rect
fixture_scroll_cmd: db 23,7,1,2,1

fixture_plot_rect:
    ld (fixture_rect_x0),bc
    ld (fixture_rect_y0),de
    ld (fixture_rect_x1),ix
    ld (fixture_rect_y1),iy
    ld a,25
    ld (fixture_rect_cmd_second),a
    ld hl,fixture_rect_cmd
    ld bc,12
    rst.lil 18h
    ret
fixture_rect_cmd:
    db 25,4
fixture_rect_x0: dw 0
fixture_rect_y0: dw 0
fixture_rect_cmd_second: db 25
    db 101
fixture_rect_x1: dw 0
fixture_rect_y1: dw 0
    db 0 ; absorbs IYU outside the twelve-byte VDU packet

fixture_gcol:
    ld (fixture_gcol_mode),a
    ld a,c
    ld (fixture_gcol_color),a
    ld hl,fixture_gcol_cmd
    ld bc,3
    rst.lil 18h
    ret
fixture_gcol_cmd: db 18
fixture_gcol_mode: db 0
fixture_gcol_color: db 0

fixture_set_mode:
    ld (fixture_mode_value),a
    ld hl,fixture_mode_cmd
    ld bc,2
    rst.lil 18h
    ret
fixture_mode_cmd: db 22
fixture_mode_value: db 0

fixture_set_scaling:
    ld (fixture_scaling_value),a
    ld hl,fixture_scaling_cmd
    ld bc,4
    rst.lil 18h
    ret
fixture_scaling_cmd: db 23,0,0c0h
fixture_scaling_value: db 0

fixture_cls:
    ld a,12
    rst.lil 10h
    ret

fixture_clg:
    ld a,16
    rst.lil 10h
    ret

fixture_move_cursor:
    ld (fixture_cursor_position),bc
    ld hl,fixture_cursor_position_cmd
    ld bc,3
    rst.lil 18h
    ret
fixture_cursor_position_cmd: db 31
fixture_cursor_position: dw 0
    db 0 ; absorbs BCU outside the three-byte VDU packet

fixture_cursor_off:
    ld hl,fixture_cursor_off_cmd
    ld bc,3
    rst.lil 18h
    ret
fixture_cursor_off_cmd: db 23,1,0

fixture_cursor_on:
    ld hl,fixture_cursor_on_cmd
    ld bc,3
    rst.lil 18h
    ret
fixture_cursor_on_cmd: db 23,1,1

fixture_vblank:
    push bc
    push de
    push ix
    MOSCALL mos_sysvars
    ld e,(ix+sysvar_time)
    ld bc,0
@wait:
    ld a,(ix+sysvar_time)
    cp e
    jp nz,@done
    dec c
    jp nz,@wait
    djnz @wait
@done:
    pop ix
    pop de
    pop bc
    ret

fixture_print_string:
    ld a,(hl)
    or a
    ret z
    rst.lil 10h
    inc hl
    jp fixture_print_string

fixture_complete_message: db "Overlay qualifier complete. Press any key.",13,10,0
fixture_failed_message: db "Overlay qualifier failed to load. Press any key.",13,10,0

; ---------------------------------------------------------------------------
; Minimal sprite command surface
; ---------------------------------------------------------------------------

fixture_sprite_select:
    ld (fixture_sprite_id),a
    ld hl,fixture_sprite_select_cmd
    ld bc,4
    rst.lil 18h
    ret
fixture_sprite_select_cmd: db 23,27,4
fixture_sprite_id: db 0

fixture_sprite_clear_frames:
    ld hl,fixture_sprite_clear_cmd
    ld bc,3
    rst.lil 18h
    ret
fixture_sprite_clear_cmd: db 23,27,5

fixture_sprite_add_buffer:
    ld (fixture_sprite_buffer_id),hl
    ld hl,fixture_sprite_add_cmd
    ld bc,5
    rst.lil 18h
    ret
fixture_sprite_add_cmd: db 23,27,026h
fixture_sprite_buffer_id: dw 0
    db 0 ; absorbs HLU outside the five-byte VDU packet

fixture_sprite_activate:
    ld (fixture_sprite_count),a
    ld hl,fixture_sprite_activate_cmd
    ld bc,4
    rst.lil 18h
    ret
fixture_sprite_activate_cmd: db 23,27,7
fixture_sprite_count: db 0

fixture_sprite_select_frame:
    ld (fixture_sprite_frame),a
    ld hl,fixture_sprite_frame_cmd
    ld bc,4
    rst.lil 18h
    ret
fixture_sprite_frame_cmd: db 23,27,10
fixture_sprite_frame: db 0

fixture_sprite_show:
    ld hl,fixture_sprite_show_cmd
    ld bc,3
    rst.lil 18h
    ret
fixture_sprite_show_cmd: db 23,27,11

fixture_sprite_hide:
    ld hl,fixture_sprite_hide_cmd
    ld bc,3
    rst.lil 18h
    ret
fixture_sprite_hide_cmd: db 23,27,12

fixture_sprite_move_abs:
    ld (fixture_sprite_x),bc
    ld (fixture_sprite_y),de
    ld hl,fixture_sprite_move_cmd
    ld bc,7
    rst.lil 18h
    ret
fixture_sprite_move_cmd: db 23,27,13
fixture_sprite_x: dw 0
fixture_sprite_y: dw 0
    db 0 ; absorbs DEU outside the seven-byte VDU packet

fixture_sprite_set_gcol:
    ld (fixture_sprite_gcol),a
    ld hl,fixture_sprite_gcol_cmd
    ld bc,4
    rst.lil 18h
    ret
fixture_sprite_gcol_cmd: db 23,27,18
fixture_sprite_gcol: db 0

fixture_sprite_mark_hardware:
    ld hl,fixture_sprite_hardware_cmd
    ld bc,3
    rst.lil 18h
    ret
fixture_sprite_hardware_cmd: db 23,27,19

fixture_sprite_mark_software:
    ld hl,fixture_sprite_software_cmd
    ld bc,3
    rst.lil 18h
    ret
fixture_sprite_software_cmd: db 23,27,20

fixture_sprite_update:
    ld hl,fixture_sprite_update_cmd
    ld bc,3
    rst.lil 18h
    ret
fixture_sprite_update_cmd: db 23,27,15

fixture_sprite_reset:
    ld hl,fixture_sprite_reset_cmd
    ld bc,3
    rst.lil 18h
    ret
fixture_sprite_reset_cmd: db 23,27,17

fixture_clear_all_buffers:
    ld hl,fixture_clear_all_cmd
    ld bc,6
    rst.lil 18h
    ret
fixture_clear_all_cmd: db 23,0,0a0h,0ffh,0ffh,2

fixture_original_mode: db 0
fixture_case: db 0
fixture_animation_frame: db 0
fixture_frames_left: dl 0
fixture_bitmap_id: dl 0
fixture_bitmap_width: dl 0
fixture_bitmap_height: dl 0
fixture_file_handle: db 0

    .if fixture_status_pass_end-fixture_status_pass-9
        dl fixture_status_pass_size_mismatch
    .endif
    .if fixture_status_fail_end-fixture_status_fail-9
        dl fixture_status_fail_size_mismatch
    .endif
    .if fixture_verify_absent_end-fixture_verify_absent-9
        dl fixture_verify_absent_size_mismatch
    .endif
    .if fixture_verify_zero_end-fixture_verify_zero-11
        dl fixture_verify_zero_size_mismatch
    .endif
    .if fixture_verify_beef_end-fixture_verify_beef-11
        dl fixture_verify_beef_size_mismatch
    .endif
