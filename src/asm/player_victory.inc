; Actor-owned automated victory flight for GAME-001F.
;
; The global transition owns terrain/starfield speed; this controller owns only
; the player record and reports ready once its loop reaches screen centre.

PLAYER_VICTORY_GATHER: equ 0
PLAYER_VICTORY_LOOP: equ 1
PLAYER_VICTORY_CENTER: equ 2
PLAYER_VICTORY_HOLD: equ 3

player_victory_phase: equ player_custom_end
player_victory_timer: equ player_victory_phase+1
player_victory_custom_end: equ player_victory_timer+1

player_victory_anchor_x: equ 120
player_victory_anchor_y: equ 224
player_victory_center_x: equ 120
player_victory_center_y: equ 168
player_victory_approach_speed: equ 3*256
player_victory_loop_speed: equ 4*256
player_victory_loop_heading: equ 192*256
player_victory_loop_heading_step: equ 4*256
player_victory_loop_frames: equ 64
player_victory_approach_timeout: equ 120

player_victory_target_x: dl 0
player_victory_target_y: dl 0
player_victory_error: db 0

    .if player_victory_custom_end-player_custom_end-2
        dl player_victory_custom_span_mismatch
    .endif
    .if actor_record_size-player_victory_custom_end-6
        dl player_victory_custom_capacity_mismatch
    .endif

; inputs: IY = validated player record
player_victory_init:
    res sprite_alive,(iy+sprite_collisions)
    xor a
    ld (iy+player_victory_phase),a
    ld (player_victory_error),a
    ld a,player_victory_approach_timeout
    ld (iy+player_victory_timer),a
    ld hl,player_victory_approach_speed
    ld (iy+sprite_vel),hl
    ld (table_pointer),iy
    ret
; end player_victory_init

player_victory:
    call player_victory_update
    ret
; end player_victory

; Advance one bounded actor update.
; outputs: carry set only while the player is centred in HOLD
; destroys: everything except SP
player_victory_update:
    ld iy,player_begin
    ld (table_pointer),iy
    ld a,(iy+player_victory_phase)
    cp PLAYER_VICTORY_GATHER
    jp z,@gather
    cp PLAYER_VICTORY_LOOP
    jp z,@loop
    cp PLAYER_VICTORY_CENTER
    jp z,@center
    cp PLAYER_VICTORY_HOLD
    jp z,@hold
; Unknown actor state fails closed at the centre rather than dispatching data.
    ld a,1
    ld (player_victory_error),a
    call player_victory_snap_center
    jp @begin_hold

@gather:
    ld bc,player_victory_anchor_x*256
    ld de,player_victory_anchor_y*256
    call player_victory_move_toward
    jp c,@begin_loop
    dec (iy+player_victory_timer)
    ret nz
    ld a,1
    ld (player_victory_error),a
    call player_victory_snap_anchor
@begin_loop:
    ld a,PLAYER_VICTORY_LOOP
    ld (iy+player_victory_phase),a
    ld a,player_victory_loop_frames
    ld (iy+player_victory_timer),a
    ld hl,player_victory_loop_speed
    ld (iy+sprite_vel),hl
    ld hl,player_victory_loop_heading
    ld (iy+sprite_heading),hl
    xor a ; bank left through the first half of the loop
    ld (iy+sprite_frame),a
    xor a
    ret

@loop:
    ld a,(iy+player_victory_timer)
    cp 33
    ld a,0
    jp nc,@loop_frame_ready
    ld a,2
@loop_frame_ready:
    ld (iy+sprite_frame),a
    call sprite_compute_velocities
    call move_sprite
    call player_draw_record
    ld hl,(iy+sprite_heading)
    ld de,player_victory_loop_heading_step
    add hl,de
; Keep only the 16-bit angle256 domain even when ADL arithmetic carries into
; HLU. DEC followed by short INC repairs the upper byte without changing HL.
    dec hl
    inc.s hl
    ld (iy+sprite_heading),hl
    dec (iy+player_victory_timer)
    ret nz
    ld a,PLAYER_VICTORY_CENTER
    ld (iy+player_victory_phase),a
    ld a,player_victory_approach_timeout
    ld (iy+player_victory_timer),a
    ld hl,player_victory_approach_speed
    ld (iy+sprite_vel),hl
    xor a
    ret

@center:
    ld bc,player_victory_center_x*256
    ld de,player_victory_center_y*256
    call player_victory_move_toward
    jp c,@begin_hold
    dec (iy+player_victory_timer)
    ret nz
    ld a,1
    ld (player_victory_error),a
    call player_victory_snap_center
@begin_hold:
    ld a,PLAYER_VICTORY_HOLD
    ld (iy+player_victory_phase),a
    ld hl,0
    ld (iy+sprite_xvel),hl
    ld (iy+sprite_yvel),hl
    ld (iy+sprite_vel),hl
    ld a,1
    ld (iy+sprite_frame),a
    call player_draw_record
@hold:
    scf
    ret
; end player_victory_update

; Move toward a fixed point and snap only after both remaining axis distances
; are within one approach step.
; inputs: IY = player, BC = target X 16.8, DE = target Y 16.8
; outputs: carry set after exact arrival; carry clear after one movement update
player_victory_move_toward:
    ld (player_victory_target_x),bc
    ld (player_victory_target_y),de
    push bc
    pop hl
    ld de,(iy+sprite_x)
    or a
    sbc hl,de
    call player_victory_axis_within_step
    jp nc,@move
    ld hl,(player_victory_target_y)
    ld de,(iy+sprite_y)
    or a
    sbc hl,de
    call player_victory_axis_within_step
    jp nc,@move
    ld hl,(player_victory_target_x)
    ld (iy+sprite_x),hl
    ld hl,(player_victory_target_y)
    ld (iy+sprite_y),hl
    ld hl,0
    ld (iy+sprite_xvel),hl
    ld (iy+sprite_yvel),hl
    ld a,1
    ld (iy+sprite_frame),a
    call player_draw_record
    scf
    ret
@move:
    ld a,(player_victory_target_x+1)
    cp (iy+sprite_x+1)
    ld a,0
    jp c,@bank_ready
    jp z,@bank_center
    ld a,2
    jp @bank_ready
@bank_center:
    ld a,1
@bank_ready:
    ld (iy+sprite_frame),a
    ld bc,(player_victory_target_x)
    ld de,(player_victory_target_y)
    call bearing_to_coord
    ld (iy+sprite_heading),hl
    call sprite_compute_velocities
    call move_sprite
    call player_draw_record
    xor a
    ret
; end player_victory_move_toward

; inputs: HL = signed 16.8 axis delta
; outputs: carry set iff abs(delta) <= approach speed
player_victory_axis_within_step:
    jp p,@positive
    call neg_hlu
@positive:
    ld de,player_victory_approach_speed
    or a
    sbc hl,de
    jp c,@within
    jp z,@within
    xor a
    ret
@within:
    scf
    ret

player_victory_snap_anchor:
    ld hl,player_victory_anchor_x*256
    ld (iy+sprite_x),hl
    ld hl,player_victory_anchor_y*256
    ld (iy+sprite_y),hl
    ret

player_victory_snap_center:
    ld hl,player_victory_center_x*256
    ld (iy+sprite_x),hl
    ld hl,player_victory_center_y*256
    ld (iy+sprite_y),hl
    ret
