; Bounded phase/timing controller for GAME-001F.
;
; It owns no VDU or actor calls. Entry requires a drained playfield. The
; global state consumes each speed with the separate starfield scroller.

LEVEL_TRANSITION_APPROACH: equ 0
LEVEL_TRANSITION_ACCEL: equ 1
LEVEL_TRANSITION_HOLD: equ 2
LEVEL_TRANSITION_DECEL: equ 3
LEVEL_TRANSITION_COMPLETE: equ 4

level_transition_approach_frames: equ 90
level_transition_speed_frames: equ 8
level_transition_hold_frames: equ 90

level_transition_phase: db LEVEL_TRANSITION_APPROACH
level_transition_timer: dl 0
level_transition_speed: db 1
level_transition_frame_speed: db 1
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
level_transition_total_frames: equ level_transition_approach_frames+[level_transition_accel_count*level_transition_speed_frames]+level_transition_hold_frames+[level_transition_decel_count*level_transition_speed_frames]

    .if level_transition_accel_count-8
        dl level_transition_accel_count_mismatch
    .endif
    .if level_transition_decel_count-7
        dl level_transition_decel_count_mismatch
    .endif
    .if level_transition_total_frames-300
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
    ld hl,level_transition_approach_frames
    ld (level_transition_timer),hl
    ret
; end level_transition_init

; Advance exactly one outer frame.
; outputs:
;   carry clear, A = this frame's scroll speed
;   carry set, A = 0 once all 300 starfield transition frames have completed
; destroys: a,de,hl
level_transition_step:
    ld a,(level_transition_phase)
    cp LEVEL_TRANSITION_COMPLETE
    jp z,@complete
    cp LEVEL_TRANSITION_COMPLETE+1
    jp nc,@invalid
; Freeze speed before phase mutation so every phase keeps its full duration.
    ld a,(level_transition_speed)
    ld (level_transition_frame_speed),a
    ld a,(level_transition_phase)
    cp LEVEL_TRANSITION_APPROACH
    jp z,@approach
    cp LEVEL_TRANSITION_ACCEL
    jp z,@accel
    cp LEVEL_TRANSITION_HOLD
    jp z,@hold
; Only DECEL remains after the validated range check above.
    jp @decel

@approach:
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
