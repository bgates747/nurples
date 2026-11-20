; Shared attract/game-over renderer. The controller owns cadence and input.
; Prompts use hardware slot 21. Only the scrolling review uses a hardware
; title; the frozen game's title is painted onto the playfield. Overlay
; bitmaps are immutable process resources retained for later entries.

PRESENTATION_SPRITE_TITLE: equ 20
PRESENTATION_SPRITE_PROMPT: equ 21
PRESENTATION_ACTIVE_ACTORS: equ 20
PRESENTATION_ACTIVE_ALL: equ 22
PRESENTATION_TITLE_X: equ 136 ; physical pixels, shared with the review sprite
PRESENTATION_TITLE_Y: equ 116

PRESENTATION_BUF_TITLE_0: equ BUF_GAME_OVER_0
PRESENTATION_BUF_TITLE_1: equ BUF_GAME_OVER_1
PRESENTATION_BUF_PROMPT_CONTINUE: equ BUF_PRESS_ANY_KEY_TO_CONTINUE
PRESENTATION_BUF_PROMPT_START: equ BUF_PRESS_ANY_KEY_TO_START

presentation_renderer_active: db 0

presentation_renderer_process_init:
    xor a
    ld (presentation_renderer_active),a
    ret
; end presentation_renderer_process_init

; Define both reserved slots while they are outside the active prefix, then
; publish the complete 22-slot prefix atomically from the game's perspective.
; outputs: carry set on a new active renderer; carry clear on invalid/stale use
presentation_renderer_enter:
    ld a,(presentation_renderer_active)
    or a
    ret nz
    ld a,(presentation_mode)
    cp PRESENTATION_GAME_OVER+1
    ret nc
    call overlay_feature_begin
    ret nc

; The scanline compositor reads active Sprite objects asynchronously. Stop the
; global engine before changing frame vectors or the hardware bit, then publish
; the complete mixed software/hardware prefix only after both slots are ready.
    xor a
    call vdu_sprite_activate

    ld a,PRESENTATION_SPRITE_TITLE
    call vdu_sprite_select
    call vdu_sprite_clear_frames
    .if GAME_OVER_REVIEW_SCROLL
    ld hl,PRESENTATION_BUF_TITLE_0
    call vdu_sprite_add_buff
    ld hl,PRESENTATION_BUF_TITLE_1
    call vdu_sprite_add_buff
    xor a
    call vdu_sprite_set_gcol
    call vdu_sprite_mark_hardware
    ld bc,PRESENTATION_TITLE_X
    ld de,PRESENTATION_TITLE_Y
    call vdu_sprite_move_abs
    ld a,(presentation_title_frame)
    call vdu_sprite_select_frame
    ld a,(presentation_mode)
    cp PRESENTATION_GAME_OVER
    jp nz,@hide_title
    call vdu_sprite_show
    jp @title_ready
@hide_title:
    call vdu_sprite_hide
@title_ready:
    .else
; Slot 21 needs a contiguous active prefix. Keep slot 20 defined but hidden,
; with no title frames and no hardware compositor membership.
    ld hl,PRESENTATION_BUF_PROMPT_START
    call vdu_sprite_add_buff
    call vdu_sprite_hide
    .endif

    ld a,PRESENTATION_SPRITE_PROMPT
    call vdu_sprite_select
    call vdu_sprite_clear_frames
    ld hl,PRESENTATION_BUF_PROMPT_CONTINUE
    call vdu_sprite_add_buff
    ld hl,PRESENTATION_BUF_PROMPT_START
    call vdu_sprite_add_buff
    xor a
    call vdu_sprite_set_gcol
    call vdu_sprite_mark_hardware
    ld bc,152
    ld de,210
    call vdu_sprite_move_abs
    ld a,(presentation_mode)
    xor %00000001 ; attract=START frame 1; game-over=CONTINUE frame 0
    call vdu_sprite_select_frame
    ld a,(presentation_prompt_visible)
    or a
    jp z,@hide_prompt
    call vdu_sprite_show
    jp @prompt_ready
@hide_prompt:
    call vdu_sprite_hide
@prompt_ready:

    ld a,PRESENTATION_ACTIVE_ALL
    call vdu_sprite_activate
    ld a,1
    ld (presentation_renderer_active),a
    scf
    ret
; end presentation_renderer_enter

; Update the prompt on its dirty bit. The frozen title is redrawn once per
; Game Over frame after active scenery, including on its first frame.
; outputs: carry set while active; carry clear when no renderer owns the slots
presentation_renderer_update:
    ld a,(presentation_renderer_active)
    or a
    ret z
    .if GAME_OVER_REVIEW_SCROLL
    ld a,(presentation_dirty)
    bit 0,a
    jp z,@prompt
    ld a,PRESENTATION_SPRITE_TITLE
    call vdu_sprite_select
    ld a,(presentation_title_frame)
    call vdu_sprite_select_frame
    ld a,(presentation_mode)
    cp PRESENTATION_GAME_OVER
    jp nz,@title_hide
    call vdu_sprite_show
    jp @prompt
@title_hide:
    call vdu_sprite_hide
    .else
    ld a,(presentation_mode)
    cp PRESENTATION_GAME_OVER
    call z,presentation_renderer_plot_title
    .endif

@prompt:
    ld a,(presentation_dirty)
    bit 1,a
    jp z,@active
    ld a,PRESENTATION_SPRITE_PROMPT
    call vdu_sprite_select
    ld a,(presentation_prompt_visible)
    or a
    jp z,@prompt_hide
    call vdu_sprite_show
    jp @active
@prompt_hide:
    call vdu_sprite_hide
@active:
    scf
    ret
; end presentation_renderer_update

    .if GAME_OVER_REVIEW_SCROLL
    .else
; Both frames have identical opaque pixels and binary alpha. Swap the colors
; in place without a background capture or erasing the title rectangle.
; On continuation game_prepare_playfield redraws the entire surface; on
; Escape the common MOS exit clears it. Neither needs a saved background.
presentation_renderer_plot_title:
    call set_gfx_playing_field ; cockpit gauges left their own viewport active
    ld hl,PRESENTATION_BUF_TITLE_0
    ld a,(presentation_title_frame)
    or a
    jp z,@selected
    ld hl,PRESENTATION_BUF_TITLE_1
@selected:
    call vdu_buff_select
    ld bc,PRESENTATION_TITLE_X-origin_left
    ld de,PRESENTATION_TITLE_Y-origin_top
    jp vdu_plot_bmp
; end presentation_renderer_plot_title
    .endif

; Restore the exact production prefix and detach/demote both slots before the
; hardware feature flag is restored. Immutable bitmap buffers stay under the
; app-wide asset lifetime and are released by the normal process resource reset.
; outputs: carry set after active teardown; carry clear when already inactive
presentation_renderer_exit:
    ld a,(presentation_renderer_active)
    or a
    ret z
    ld a,PRESENTATION_SPRITE_PROMPT
    call vdu_sprite_select
    call vdu_sprite_hide
    ld a,PRESENTATION_SPRITE_TITLE
    call vdu_sprite_select
    call vdu_sprite_hide
; Hide is safe while active, but frame-list and hardware-bit mutation is not:
; detach the scanline compositor from every sprite before demotion.
    xor a
    call vdu_sprite_activate

    ld a,PRESENTATION_SPRITE_PROMPT
    call vdu_sprite_select
    call vdu_sprite_clear_frames
    call vdu_sprite_mark_software
    ld a,PRESENTATION_SPRITE_TITLE
    call vdu_sprite_select
    call vdu_sprite_clear_frames
    .if GAME_OVER_REVIEW_SCROLL
    call vdu_sprite_mark_software
    .endif

    ld a,PRESENTATION_ACTIVE_ACTORS
    call vdu_sprite_activate
    call overlay_feature_end
    xor a
    ld (presentation_renderer_active),a
    scf
    ret
; end presentation_renderer_exit
