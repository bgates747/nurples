; Actor-owned physical tracking for fireball turrets.
;
; Desired targeting-computer headings and non-hostile search both pass through
; this one physical barrel rate limiter. The active-tile record owns all state.

turret_tracking_custom_begin: equ actor_record_custom
turret_desired_heading: equ turret_tracking_custom_begin ; 3-byte 16.8 angle256
turret_search_direction: equ turret_desired_heading+3     ; one of values below
turret_search_timer: equ turret_search_direction+1        ; actor-owned duration
turret_tracking_custom_end: equ turret_search_timer+1

TURRET_SEARCH_POSITIVE: equ 0
TURRET_SEARCH_NEGATIVE: equ 1
turret_tracking_rate: equ 2*256
turret_search_offset: equ 64*256
turret_search_min_frames: equ 24
turret_search_random_mask: equ %00011111

    .if turret_tracking_custom_end-turret_tracking_custom_begin-5
        dl turret_tracking_custom_span_mismatch
    .endif
    .if actor_record_size-turret_tracking_custom_end-17
        dl turret_tracking_custom_capacity_mismatch
    .endif

; Initialize the custom fields for a newly allocated turret lifetime.
; inputs: IX = turret active-tile record
; destroys: a,hl
turret_tracking_init:
    ld hl,(ix+tile_heading)
    ld (ix+turret_desired_heading),hl
    call rand_8
    and %00000001
    ld (ix+turret_search_direction),a
    call rand_8
    and turret_search_random_mask
    add a,turret_search_min_frames
    ld (ix+turret_search_timer),a
    ret
; end turret_tracking_init

; Slew the physical barrel toward a desired targeting-computer result.
; inputs: IY = turret record, HL = desired 16.8 angle256
; outputs: HL = updated physical heading, carry set iff desired reached
; destroys: a,bc,de
turret_track_desired_heading:
    ld (iy+turret_desired_heading),hl
    push hl
    pop de
    ld hl,(iy+tile_heading)
    ld bc,turret_tracking_rate
    call heading_slew_168
    ld (iy+tile_heading),hl
    ret
; end turret_track_desired_heading

; Perform one non-firing search update. Direction and duration are independent
; of the reload timer. Expiry flips direction, chooses a new bounded duration,
; and continues through the same physical slew limiter used for live tracking.
; inputs: IY = turret record
; outputs: HL = updated physical heading
; destroys: a,bc,de,hl
turret_search_update:
    ld a,(iy+turret_search_timer)
    or a
    jp nz,@advance
    ld a,(iy+turret_search_direction)
    xor %00000001
    ld (iy+turret_search_direction),a
    call rand_8
    and turret_search_random_mask
    add a,turret_search_min_frames
    ld (iy+turret_search_timer),a
@advance:
    ld hl,(iy+tile_heading)
    ld de,turret_search_offset
    ld a,(iy+turret_search_direction)
    or a
    jp z,@desired_ready
    or a
    sbc hl,de
    jp @slew
@desired_ready:
    add hl,de
@slew:
    call turret_track_desired_heading
    dec (iy+turret_search_timer)
    ret
; end turret_search_update

; Freeze projectile globals from the physical barrel, not the ideal lead angle.
; Call immediately before enemy_spawn_fireball while IY still owns the turret.
; inputs: IY = turret record
; outputs: Heading_p/Vp_x/Vp_y describe the physical barrel at this instant
; destroys: bc,de,hl
turret_prepare_physical_shot:
    ld hl,(iy+tile_heading)
    ld (Heading_p),hl
    ld de,speed_fireball
    call polar_to_cartesian
    ld (Vp_x),bc
    ld (Vp_y),de
    ret
; end turret_prepare_physical_shot
