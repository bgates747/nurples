; Bounded phase/timing controller for GAME-001F.
;
; It owns no VDU or actor calls: the global state consumes frame_speed and
; frame_policy, runs the ordinary one-line terrain path only during DRAIN, and
; uses starfield_warp_scroll afterward.

LEVEL_TRANSITION_DRAIN: equ 0
LEVEL_TRANSITION_CENTER: equ 1
LEVEL_TRANSITION_ACCEL: equ 2
LEVEL_TRANSITION_HOLD: equ 3
LEVEL_TRANSITION_DECEL: equ 4
LEVEL_TRANSITION_COMPLETE: equ 5

LEVEL_TRANSITION_POLICY_TERRAIN: equ %00000001

level_transition_drain_frames: equ field_bottom+1
level_transition_center_frames: equ 90
level_transition_speed_frames: equ 8
level_transition_hold_frames: equ 90

level_transition_phase: db LEVEL_TRANSITION_DRAIN
level_transition_timer: dl 0
level_transition_speed: db 1
level_transition_frame_speed: db 1
level_transition_frame_policy: db LEVEL_TRANSITION_POLICY_TERRAIN
level_transition_speed_index: db 0
level_transition_error: db 0

level_transition_accel_speeds:
    db 1,2,3,4,6,8,12,16
level_transition_accel_speeds_end:
level_transition_decel_speeds:
    db 12,8,6,4,3,2,1
level_transition_decel_speeds_end:

level_transition_accel_count: equ level_transition_accel_speeds_end-level_transition_accel_speeds
level_transition_decel_count: equ level_transition_decel_speeds_end-level_transition_decel_speeds
level_transition_total_frames: equ level_transition_drain_frames+level_transition_center_frames+[level_transition_accel_count*level_transition_speed_frames]+level_transition_hold_frames+[level_transition_decel_count*level_transition_speed_frames]

    .if level_transition_accel_count-8
        dl level_transition_accel_count_mismatch
    .endif
    .if level_transition_decel_count-7
        dl level_transition_decel_count_mismatch
    .endif
    .if level_transition_total_frames-636
        dl level_transition_total_frames_mismatch
    .endif

level_transition_init:
    xor a
    ld (level_transition_phase),a
    ld (level_transition_speed_index),a
    ld (level_transition_error),a
    ld a,1
    ld (level_transition_speed),a
    ld (level_transition_frame_speed),a
    ld a,LEVEL_TRANSITION_POLICY_TERRAIN
    ld (level_transition_frame_policy),a
    ld hl,level_transition_drain_frames
    ld (level_transition_timer),hl
    ret
; end level_transition_init

; Advance exactly one outer frame.
; outputs:
;   carry clear, A = this frame's scroll speed, frame_policy frozen for caller
;   carry set, A = 0 once all 636 transition frames have completed
; destroys: a,de,hl
level_transition_step:
    ld a,(level_transition_phase)
    cp LEVEL_TRANSITION_COMPLETE
    jp z,@complete
    cp LEVEL_TRANSITION_COMPLETE+1
    jp nc,@invalid
; Freeze outputs before phase mutation so the final frame in every phase keeps
; the old phase's speed and terrain policy; new values begin next frame.
    ld a,(level_transition_speed)
    ld (level_transition_frame_speed),a
    xor a
    ld (level_transition_frame_policy),a
    ld a,(level_transition_phase)
    cp LEVEL_TRANSITION_DRAIN
    jp nz,@dispatch
    ld a,LEVEL_TRANSITION_POLICY_TERRAIN
    ld (level_transition_frame_policy),a
@dispatch:
    ld a,(level_transition_phase)
    cp LEVEL_TRANSITION_DRAIN
    jp z,@drain
    cp LEVEL_TRANSITION_CENTER
    jp z,@center
    cp LEVEL_TRANSITION_ACCEL
    jp z,@accel
    cp LEVEL_TRANSITION_HOLD
    jp z,@hold
; Only DECEL remains after the validated range check above.
    jp @decel

@drain:
    call level_transition_tick_timer
    jp nz,@return_frame
    ld a,LEVEL_TRANSITION_CENTER
    ld (level_transition_phase),a
    ld hl,level_transition_center_frames
    ld (level_transition_timer),hl
    jp @return_frame

@center:
    call level_transition_tick_timer
    jp nz,@return_frame
    ld a,LEVEL_TRANSITION_ACCEL
    ld (level_transition_phase),a
    xor a
    ld (level_transition_speed_index),a
    call level_transition_load_accel_speed
    ld hl,level_transition_speed_frames
    ld (level_transition_timer),hl
    jp @return_frame

@accel:
    call level_transition_tick_timer
    jp nz,@return_frame
    ld a,(level_transition_speed_index)
    inc a
    cp level_transition_accel_count
    jp nc,@begin_hold
    ld (level_transition_speed_index),a
    call level_transition_load_accel_speed
    ld hl,level_transition_speed_frames
    ld (level_transition_timer),hl
    jp @return_frame
@begin_hold:
    ld a,LEVEL_TRANSITION_HOLD
    ld (level_transition_phase),a
    ld a,16
    ld (level_transition_speed),a
    ld hl,level_transition_hold_frames
    ld (level_transition_timer),hl
    jp @return_frame

@hold:
    call level_transition_tick_timer
    jp nz,@return_frame
    ld a,LEVEL_TRANSITION_DECEL
    ld (level_transition_phase),a
    xor a
    ld (level_transition_speed_index),a
    call level_transition_load_decel_speed
    ld hl,level_transition_speed_frames
    ld (level_transition_timer),hl
    jp @return_frame

@decel:
    call level_transition_tick_timer
    jp nz,@return_frame
    ld a,(level_transition_speed_index)
    inc a
    cp level_transition_decel_count
    jp nc,@finish
    ld (level_transition_speed_index),a
    call level_transition_load_decel_speed
    ld hl,level_transition_speed_frames
    ld (level_transition_timer),hl
    jp @return_frame
@finish:
    ld a,LEVEL_TRANSITION_COMPLETE
    ld (level_transition_phase),a
    ld a,1
    ld (level_transition_speed),a
@return_frame:
    ld a,(level_transition_frame_speed)
    or a ; every legal frame speed is nonzero; clear carry
    ret

@invalid:
    ld a,1
    ld (level_transition_error),a
    ld a,LEVEL_TRANSITION_COMPLETE
    ld (level_transition_phase),a
@complete:
    xor a
    scf
    ret
; end level_transition_step

level_transition_tick_timer:
    ld hl,(level_transition_timer)
    dec hl
    ld (level_transition_timer),hl
    ld de,0
    or a
    sbc hl,de
    ret

level_transition_load_accel_speed:
    ld hl,level_transition_accel_speeds
    jp level_transition_load_indexed_speed

level_transition_load_decel_speed:
    ld hl,level_transition_decel_speeds
level_transition_load_indexed_speed:
    ld a,(level_transition_speed_index)
    ld e,a
    ld d,0
    add hl,de
    ld a,(hl)
    ld (level_transition_speed),a
    ret
