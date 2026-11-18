; Actor-owned launch-pad identity and non-hostile seeker retreat behavior.
;
; Every seeker retains the exact launch-pad lifetime that created it. During
; non-live player phases it orbits harmlessly, then docks or exits an edge.

seeker_state_retreat_pad: equ 2
seeker_state_retreat_edge: equ 3

seeker_spawn_pad_id: equ seeker_custom_end
seeker_spawn_pad_generation: equ seeker_spawn_pad_id+1
seeker_completed_orbits: equ seeker_spawn_pad_generation+1
seeker_retreat_custom_end: equ seeker_completed_orbits+1

SEEKER_PAD_NONE: equ 0ffh
seeker_retreat_orbits: equ 2
seeker_edge_heading_right: equ 64*256
seeker_edge_heading_left: equ 192*256

    .if seeker_retreat_custom_end-seeker_custom_end-3
        dl seeker_retreat_custom_span_mismatch
    .endif
    .if actor_record_size-seeker_retreat_custom_end-16
        dl seeker_retreat_custom_capacity_mismatch
    .endif

; Capture the exact pad lifetime that launched a newly allocated seeker.
; inputs: IX = seeker record, IY = source active-tile record
; outputs: carry set on a validated live small pad; otherwise identity cleared
; destroys: a,de,hl
seeker_capture_spawn_pad:
    call tile_table_record_pointer_valid
    jp nc,@invalid
    bit sprite_alive,(iy+tile_collisions)
    jp z,@invalid
    ld a,(iy+tile_type)
    and actor_type_mask
    cp tile_pad_small
    jp nz,@invalid
    ld a,(iy+tile_id)
    ld (ix+seeker_spawn_pad_id),a
    ld a,(iy+actor_record_generation)
    or a
    jp z,@invalid
    ld (ix+seeker_spawn_pad_generation),a
    xor a
    ld (ix+seeker_completed_orbits),a
    scf
    ret
@invalid:
    ld a,SEEKER_PAD_NONE
    ld (ix+seeker_spawn_pad_id),a
    xor a
    ld (ix+seeker_spawn_pad_generation),a
    ld (ix+seeker_completed_orbits),a
    ret
; end seeker_capture_spawn_pad

; Resolve a captured id+generation pair to the same still-visible pad lifetime.
; inputs: IX = seeker record
; outputs: carry set and IY = validated pad; carry clear and IY invalid
; destroys: a,de,hl,iy
seeker_resolve_spawn_pad:
    ld a,(ix+seeker_spawn_pad_id)
    cp SEEKER_PAD_NONE
    jp z,@invalid
    ld hl,0
    ld l,a
    ld h,tile_table_record_size
    mlt hl
    ld de,tile_table_base
    add hl,de
    push hl
    pop iy
    call tile_table_record_pointer_valid
    jp nc,@invalid
    ld a,(ix+seeker_spawn_pad_generation)
    or a
    jp z,@invalid
    cp (iy+actor_record_generation)
    jp nz,@invalid
    bit sprite_alive,(iy+tile_collisions)
    jp z,@invalid
    ld a,(iy+tile_type)
    and actor_type_mask
    cp tile_pad_small
    jp nz,@invalid
; A pad is visible if some part of its 16-pixel body intersects the field.
    ld hl,(iy+tile_y)
    ld de,16*256
    add hl,de
    jp m,@invalid
    ld hl,(iy+tile_y)
    ld de,field_bottom*256
    or a
    sbc hl,de
    jp p,@invalid
    scf
    ret
@invalid:
    xor a
    ret
; end seeker_resolve_spawn_pad

; Reset the deterministic orbit budget when entering a non-hostile hold.
; inputs: IY = seeker record
seeker_retreat_reset_orbits:
    xor a
    ld (iy+seeker_completed_orbits),a
    ret

; Call immediately after the existing orbit angle increment. Every complete
; 256-step orbit advances the actor-owned count; the second enters retreat.
; inputs: IY = seeker record
; outputs: carry set iff retreat was entered on this update
seeker_orbit_note_step:
    ld a,(iy+seeker_orbit_angle)
    or a
    jp nz,@continue_orbit
    ld a,(iy+seeker_completed_orbits)
    inc a
    ld (iy+seeker_completed_orbits),a
    cp seeker_retreat_orbits
    jp c,@continue_orbit
    jp seeker_enter_retreat
@continue_orbit:
    xor a
    ret
; end seeker_orbit_note_step

; Prefer the captured pad; otherwise select the nearer horizontal edge.
; inputs: IY = seeker record
; outputs: carry set after publishing one initialized retreat state
; destroys: everything except SP
seeker_enter_retreat:
    push iy
    pop ix
    call seeker_resolve_spawn_pad
    jp nc,seeker_enter_retreat_edge_ix
    push ix
    pop iy
    ld a,seeker_state_retreat_pad
    ld (iy+seeker_state),a
    ld hl,move_enemy_seeker_retreat_pad
    ld (iy+sprite_move_program),hl
    scf
    ret

; inputs: IX = seeker record
seeker_enter_retreat_edge_ix:
    ld a,(ix+sprite_x+1)
    cp 120
    ld hl,seeker_edge_heading_left
    jp c,@heading_ready
    ld hl,seeker_edge_heading_right
@heading_ready:
    ld (ix+sprite_heading),hl
    ld de,(ix+sprite_vel)
    call polar_to_cartesian
    ld (ix+sprite_xvel),bc
    ld (ix+sprite_yvel),de
    ld a,seeker_state_retreat_edge
    ld (ix+seeker_state),a
    ld hl,move_enemy_seeker_retreat_edge
    ld (ix+sprite_move_program),hl
    scf
    ret
; end seeker_enter_retreat_edge_ix

; Home to the captured pad without checking player or weapon collisions. A pad
; that ceases to be the captured visible lifetime causes an edge retreat.
; inputs: IY = seeker record, table_pointer already publishes it
move_enemy_seeker_retreat_pad:
    push iy
    pop ix
    call seeker_resolve_spawn_pad
    jp nc,@lost_pad
    call check_collision_box
    jp z,@docked
    ld bc,(iy+tile_x)
    ld de,(iy+tile_y)
    push ix
    pop iy
    call bearing_to_coord
    ld (iy+sprite_heading),hl
    call sprite_compute_velocities
    call move_sprite
    call sprite_frame_heading
    ret
@docked:
    push ix
    pop iy
    jp table_deactivate_sprite
@lost_pad:
    call seeker_enter_retreat_edge_ix
    ret
; end move_enemy_seeker_retreat_pad

; Fly to the selected edge. The generic mover may clamp/reflect at the first
; crossing, but the actor deactivates in that same bounded update before the
; reflected velocity can be observed on another frame.
; inputs: IY = seeker record, table_pointer already publishes it
move_enemy_seeker_retreat_edge:
    call move_sprite
    ld a,(sprite_screen_edge)
    and collide_left+collide_right
    jp nz,table_deactivate_sprite
    call sprite_frame_heading
    ret
; end move_enemy_seeker_retreat_edge
