; Actor-owned launch-pad identity and seeker retreat behavior.
;
; Every seeker retains the exact launch-pad lifetime that created it. During
; death it retreats immediately, then docks or exits an edge. Game Over uses
; the same flight callbacks with an immediately selected nearest destination.

seeker_state_retreat_pad: equ 2
seeker_state_retreat_edge: equ 3

seeker_spawn_pad_id: equ seeker_custom_end
seeker_spawn_pad_generation: equ seeker_spawn_pad_id+1
seeker_completed_orbits: equ seeker_spawn_pad_generation+1
seeker_retreat_pad_id: equ seeker_completed_orbits+1
seeker_retreat_pad_generation: equ seeker_retreat_pad_id+1
seeker_retreat_custom_end: equ seeker_retreat_pad_generation+1

SEEKER_PAD_NONE: equ 0ffh
seeker_retreat_orbits: equ 2
seeker_edge_heading_right: equ 64*256
seeker_edge_heading_left: equ 192*256

    .if seeker_retreat_custom_end-seeker_custom_end-5
        dl seeker_retreat_custom_span_mismatch
    .endif
    .if actor_record_size-seeker_retreat_custom_end-14
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
    cp tile_pad_seeker
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
    ld c,a
    ld a,(ix+seeker_spawn_pad_generation)
    jp seeker_resolve_pad_identity
@invalid:
    xor a
    ret
; end seeker_resolve_spawn_pad

; IX = seeker, C = record id, A = generation (zero means no record).
seeker_resolve_pad_identity:
    or a
    ret z
    push af
    ld hl,0
    ld l,c
    ld h,tile_table_record_size
    mlt hl
    ld de,tile_table_base
    add hl,de
    push hl
    pop iy
    call tile_table_record_pointer_valid
    jp nc,@bad_pointer
    pop af
    cp (iy+actor_record_generation)
    jp nz,@invalid
    jp seeker_pad_visible
@bad_pointer:
    pop af
@invalid:
    xor a
    ret

; IY = table record. Carry means an active pad overlaps the playfield.
seeker_pad_visible:
    bit sprite_alive,(iy+tile_collisions)
    jp z,@invalid
    ld a,(iy+tile_type)
    and actor_type_mask
    cp tile_pad_seeker
    jp nz,@invalid
    ld hl,(iy+tile_x)
    ld de,16*256
    add hl,de
    jp m,@invalid
    jp z,@invalid
    ld hl,(iy+tile_x)
    ld de,[field_right+1]*256
    or a
    sbc hl,de
    jp p,@invalid
    ld hl,(iy+tile_y)
    ld de,16*256
    add hl,de
    jp m,@invalid
    jp z,@invalid
    ld hl,(iy+tile_y)
    ld de,[field_bottom+1]*256
    or a
    sbc hl,de
    jp p,@invalid
    scf
    ret
@invalid:
    xor a
    ret

; Player death uses the original pad and preserves a retreat already in
; flight. Game Over and warp may instead choose the nearest visible pad.
seeker_group_enter_death_retreat:
    ld c,1
    jp seeker_group_apply_retreat
seeker_group_enter_game_over_retreat:
seeker_group_enter_retreat:
    ld c,0
seeker_group_apply_retreat:
    ld iy,seekers_base
    ld b,seekers_max
@loop:
    push bc
    push iy
    call sprite_table_record_pointer_valid
    jp nc,@next
    bit sprite_alive,(iy+sprite_collisions)
    jp z,@next
    bit sprite_just_died,(iy+sprite_collisions)
    jp nz,@next
    ld a,(iy+sprite_type)
    and actor_type_mask
    cp enemy_air
    jp nz,@next
    ld a,c ; per-pass policy preserved by the BC save at the top of the loop
    or a
    jp nz,@death
    .if GAME_OVER_REVIEW_EDGE
; Test the edge exit even when a valid pad is under the first visible
; seeker. Do not change warp retreat or invalidate any pad record.
    ld hl,(game_state)
    ld de,game_over
    or a
    sbc hl,de
    jp nz,@nearest
    push iy
    pop ix
    call seeker_enter_retreat_edge_ix
    jp @next
@nearest:
    call seeker_enter_nearest_pad_retreat
    .else
    call seeker_enter_nearest_pad_retreat
    .endif
    jp @next
@death:
    call seeker_enter_death_retreat
@next:
    pop iy
    pop bc
    lea iy,iy+table_record_size
    djnz @loop
    ret

; IY = validated seeker. Don't change the heading/destination of a seeker
; already retreating; death can occur while a warp retreat is still active.
seeker_enter_death_retreat:
    ld a,(iy+seeker_state)
    cp seeker_state_retreat_pad
    jp z,@already
    cp seeker_state_retreat_edge
    jp nz,seeker_enter_retreat
@already:
    scf
    ret
; end seeker_enter_death_retreat

; Scan the fixed table in id order; the first record wins an exact tie.
; Squared integer-pixel deltas fit in 24 bits over this field. Do not change
; the seeker's original launch identity when choosing this alternate target.
seeker_enter_nearest_pad_retreat:
    push iy
    pop ix
    xor a
    ld (ix+seeker_retreat_pad_generation),a
    ld hl,0ffffffh
    ld (seeker_best_pad_distance),hl
    ld iy,tile_table_base
@scan:
    call seeker_pad_visible
    jp nc,@advance
    call tile_table_record_pointer_valid
    jp nc,@advance
    ld a,(iy+actor_record_generation)
    or a
    jp z,@advance
    ld hl,(ix+sprite_x)
    ld de,(iy+tile_x)
    call seeker_pad_square_delta
    push hl
    ld hl,(ix+sprite_y)
    ld de,(iy+tile_y)
    call seeker_pad_square_delta
    pop de
    add hl,de
    push hl
    ld de,(seeker_best_pad_distance)
    or a
    sbc hl,de
    pop hl
    jp nc,@advance
    ld (seeker_best_pad_distance),hl
    ld a,(iy+tile_id)
    ld (ix+seeker_retreat_pad_id),a
    ld a,(iy+actor_record_generation)
    ld (ix+seeker_retreat_pad_generation),a
@advance:
    lea iy,iy+tile_table_record_size
    push iy
    pop hl
    ld de,tile_table_end
    or a
    sbc hl,de
    jp c,@scan
    ld a,(ix+seeker_retreat_pad_generation)
    or a
    jp z,seeker_enter_retreat_edge_ix
    jp seeker_publish_pad_retreat

; HL and DE are signed 16.8 coordinates. Square the absolute pixel distance.
seeker_pad_square_delta:
    or a
    sbc hl,de
    call m,neg24
    ld (seeker_pad_delta),hl
    ld hl,0
    ld a,(seeker_pad_delta+1)
    ld l,a
    ld a,(seeker_pad_delta+2)
    ld h,a
    push hl
    pop de
    jp umul24

seeker_best_pad_distance: dl 0
seeker_pad_delta: dl 0

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
    ld a,(ix+seeker_spawn_pad_id)
    ld (ix+seeker_retreat_pad_id),a
    ld a,(ix+seeker_spawn_pad_generation)
    ld (ix+seeker_retreat_pad_generation),a
seeker_publish_pad_retreat:
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
    ld hl,(ix+sprite_x)
    ld de,120*256 ; 256-pixel field, 16-pixel seeker, tie goes right
    or a
    sbc hl,de
    ld hl,seeker_edge_heading_left
    jp m,@heading_ready
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

; Home to the selected pad. If it ceases to be the same visible lifetime,
; finish through the nearer edge.
; inputs: IY = seeker record, table_pointer already publishes it
move_enemy_seeker_retreat_pad:
    push iy
    pop ix
    ld c,(ix+seeker_retreat_pad_id)
    ld a,(ix+seeker_retreat_pad_generation)
    call seeker_resolve_pad_identity
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
    jp seeker_retreat_draw_or_collide
@docked:
    push ix
    pop iy
    jp table_deactivate_sprite
@lost_pad:
    call seeker_enter_retreat_edge_ix
    ret
; end move_enemy_seeker_retreat_pad

; Bypass the bouncing mover. Once the proposed image would no longer fit
; the playfield, hide and free the seeker before any VDP position is sent.
; inputs: IY = seeker record, table_pointer already publishes it
move_enemy_seeker_retreat_edge:
    ld hl,(iy+sprite_x)
    ld de,(iy+sprite_xvel)
    add hl,de
    ld (iy+sprite_x),hl
    jp seeker_retreat_draw_or_collide
; end move_enemy_seeker_retreat_edge

; IY = sprite record, positions relative to the playfield. Carry iff the
; whole rendered footprint fits. VDP truncates 16.8 to integer pixels; the
; last legal pixel remains drawable, including from a fractional position.
; Width and height are counts; compare the exclusive end to limit+1.
; Destroys A, DE, HL; preserves IY and the traversal counter BC.
seeker_within_playfield:
sprite_footprint_within_playfield:
    ld hl,(iy+sprite_x)
    ld de,field_left*256
    or a
    sbc hl,de
    jp m,@outside
    ld l,0
    ld de,0
    ld d,(iy+sprite_dim_x)
    ld a,d
    or a
    ret z
    add hl,de
    ld de,[field_right-field_left+2]*256
    or a
    sbc hl,de
    ret nc
    ld hl,(iy+sprite_y)
    ld de,field_top*256
    or a
    sbc hl,de
    jp m,@outside
    ld l,0
    ld de,0
    ld d,(iy+sprite_dim_y)
    ld a,d
    or a
    ret z
    add hl,de
    ld de,[field_bottom-field_top+2]*256
    or a
    sbc hl,de
    ret
@outside:
    or a
    ret
; end seeker_within_playfield

; Retreat can collide only in a live-player warp. If the player dies during
; the warp, the same exit route continues harmlessly through respawn.
seeker_retreat_draw_or_collide:
    call seeker_within_playfield
    jp nc,table_deactivate_sprite
    ld hl,(game_state)
    ld de,game_level_transition
    or a
    sbc hl,de
    jp nz,sprite_frame_heading
    ld hl,(player_state)
    ld de,player_alive
    or a
    sbc hl,de
    jp z,move_enemy_seeker
    jp sprite_frame_heading
; end seeker_retreat_draw_or_collide
