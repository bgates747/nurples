player_custom_begin: equ actor_record_custom
player_death_base_frame: equ player_custom_begin
player_death_turn_timer: equ player_death_base_frame+1
player_death_wiggle_timer: equ player_death_turn_timer+1
player_entry_angle: equ player_death_wiggle_timer+1
player_entry_timer: equ player_entry_angle+1
player_grace_timer: equ player_entry_timer+1
player_entry_warp_enabled: equ player_grace_timer+1
; Keep the actor layout stable. Three old timing bytes are now unused.
player_entry_amplitude: equ player_entry_warp_enabled+4 ; per-endpoint lateral cap
player_entry_target_x_actor: equ player_entry_amplitude+1
player_entry_target_y_actor: equ player_entry_target_x_actor+1
; Retain the former entry-direction byte so this actor's layout stays stable.
player_custom_end: equ player_entry_target_y_actor+2
    .if player_custom_end-player_custom_begin-14
        dl player_custom_span_mismatch
    .endif

player_play_ceiling: equ 64
player_entry_start_y: equ 320
player_entry_target_x: equ 120
player_entry_target_y: equ 224
player_grace_frames: equ 60
player_entry_total_frames: equ 112
player_entry_angle_step: equ 4
player_entry_max_amplitude: equ 64
player_entry_ramp_frames: equ 16 ; four pixels of amplitude per frame
player_entry_y_velocity: equ -219 ; -0.855 px/frame; final residue is snapped
player_entry_start_y_offset: equ 96
player_death_lateral_speed: equ 1*256
player_death_forward_speed: equ -64 ; -0.25 pixel/frame in 16.8 fixed

    .if player_entry_ramp_frames*4-player_entry_max_amplitude
        dl player_entry_ramp_amplitude_mismatch
    .endif

player_defaults:
    db player               ; type
    dl BUF_SHIP_0L          ; base_bufferId
    dl do_player            ; move_program
    db %01000000            ; collisions bit 6 signals sprite alive
    db 16                   ; dim_x
    db 16                   ; dim_y
    db 1                    ; num_orientations
    db 1                    ; num_animations
    db 0                    ; animation
    db 0                    ; animation_timer
    db 0                    ; move_timer
    db 0                    ; move_step
    db 0                    ; points
    db 0                    ; shield_damage
    db 0                    ; frame

player_state:  dl player_alive
player_event_none: equ 0
player_event_game_over: equ 1
player_event: db player_event_none
player_death_pending: db 0
player_fault_bad_state: equ 0
player_fault_bad_entry_warp: equ 1
player_fault_flags: db 0 ; latched fail-closed diagnostics
player_dying_phase_initial: equ 0
player_dying_phase_delay: equ 1
player_dying_phase_finale: equ 2
player_dying_phase_finish: equ 3
; The finale spawn frame itself performs lifetime update one later in the
; global loop; FINISH therefore waits only the remaining updates.
player_dying_finish_frames: equ explosion_lifetime_updates-1
player_dying_phase: db player_dying_phase_initial
player_dying_timer: db 0
player_dying_spawns: db 0
player_ships: db 0
player_ships_max: db 3
player_speed: equ 3*256 ; pixels per frame 16.8 fixed

draw_player_ships:
; activate mfd
    ld ix,mfd_ships
    call mfd_activate
; blank out previously drawn ships
    xor a ; color mode 0
    ld c,c_black
    call vdu_gcol
    ld bc,0
    ld de,0
    ld ix,0+79
    ld iy,0+15
    call vdu_plot_rf
; draw the ships remaining
    ld hl,BUF_SHIP_MFD
    call vdu_buff_select
    ld a,(player_ships) ; loop counter
    or a
    ret z ; terminal player state has no remaining ships
    ld bc,0 ; starting x-coordinate
@loop:
    dec a ; we don't count the ship we're flying
    ret z 
    push af ; back up loop counter
    push bc ; save x-coordinate
    ld de,0 ; y-coordinate
    call vdu_plot_bmp
    pop bc ; restore x-coordinate
    ld hl,16 ; bump x-coords
    add hl,bc
    push hl
    pop bc ; bc = new x-coordinate
    pop af
    jp @loop    
    ret
; end draw_player_ships

do_player:
    ld hl,(player_state)
    call player_state_pointer_valid
    jp c,@dispatch
    ld hl,player_fault_flags
    set player_fault_bad_state,(hl)
    ld a,player_event_game_over
    ld (player_event),a
    ret
@dispatch:
    CALL_HL
    ret
; end do_player

; Exact stabilization whitelist for writable legacy player-state pointers.
; Invalid actor state is surfaced as an event for the global coordinator.
player_state_pointer_valid:
    ld de,player_alive
    call @matches
    jp z,@valid
    ld de,player_spawning
    call @matches
    jp z,@valid
    ld de,player_grace
    call @matches
    jp z,@valid
    ld de,player_dying
    call @matches
    jp z,@valid
    ld de,player_dead
    call @matches
    jp z,@valid
    ld de,player_victory
    call @matches
    jp z,@valid
    xor a
    ret
@valid:
    scf
    ret
@matches:
    push hl
    or a
    sbc hl,de
    pop hl
    ret
; end player_state_pointer_valid

; World components consume this published lifecycle policy themselves. The
; coordinator continues calling them. Travel resumes with intangible grace so
; the one-second safety state does not freeze the live world after entrance.
; outputs: carry set while normal world travel is enabled
player_world_travel_enabled:
    ld hl,(game_state)
    ld de,game_over
    or a
    sbc hl,de
    jp z,@game_over
    ld hl,(player_state)
    ld de,player_alive
    or a
    sbc hl,de
    jp z,@travel
    ld hl,(player_state)
    ld de,player_grace
    or a
    sbc hl,de
    jp z,@travel
    ld hl,(player_state)
    ld de,player_victory
    or a
    sbc hl,de
    jp z,@travel
    jp @paused
@game_over:
; Advance both terrain pixels and active-tile coordinates in the scroll control.
    .if GAME_OVER_REVIEW_SCROLL
    jp @travel
    .endif
@paused:
    xor a
    ret
@travel:
    scf
    ret
; end player_world_travel_enabled

; Consume one shared warp-controller frame for the initial entrance only.
; Mid-level respawn keeps this disabled so existing terrain pixels and records
; remain spatially coherent. Carry set means A is a validated 1..16 speed.
player_entry_starfield_speed:
    ld hl,(player_state)
    ld de,player_spawning
    or a
    sbc hl,de
    jp nz,@inactive
    ld iy,player_begin
    ld a,(iy+player_entry_warp_enabled)
    or a
    jp z,@inactive
    call level_transition_step
    jp nc,@frame_ready
; Inconsistent phase must not admit terrain during the swoop.
    ld hl,player_fault_flags
    set player_fault_bad_entry_warp,(hl)
    ld a,1
@frame_ready:
    scf
    ret
@inactive:
    xor a
    ret
; end player_entry_starfield_speed

; Reserve a visible runway above ordinary player input for damaged flight.
; inputs: IY = player record
player_enforce_play_ceiling:
    ld hl,(iy+sprite_y)
    ld de,player_play_ceiling*256
    or a
    sbc hl,de
    ret p
    ld hl,player_play_ceiling*256
    ld (iy+sprite_y),hl
    ld hl,0
    ld (iy+sprite_yvel),hl
    ret
; end player_enforce_play_ceiling

; Select whether the shared entrance owns a starfield-only fast approach.
player_init_level_entry:
    ld a,1
    jp player_init
; end player_init_level_entry

player_init_respawn:
    .if mus_combat_cues
    call cues_cancel_warning ; a new shield charge must not announce the old life
    .endif
    xor a
    jp player_init
; end player_init_respawn

; set initial player position
; inputs: A = nonzero only when no terrain has entered the playfield yet
; outputs: player positioned near the bottom under the resolved endpoint
; destroys: pretty much everything
player_init:
    and 1
    push af ; preserve the requested presentation policy through allocation/VDU setup
; Resolve both level-start and respawn endpoints through the same bounded
; active-tile query. An empty level naturally selects the canonical point.
    call player_spawn_resolve_endpoint
    jp nc,@resolution_failed
    ld a,(player_shields_max)
    ld (player_shields),a
    ld a,(laser_power_max)
    ld (laser_power),a
    ld a,(player_fuel_max)
    ld (player_fuel),a
; Gameplay timers belong to a life, not to the cockpit drawing routines.
    ld a,(charge_laser_timer_max)
    ld (charge_laser_timer),a
    ld a,(discharge_fuel_timer_max)
    ld (discharge_fuel_timer),a
; set all to zero so they will update
    xor a
    ld (laser_power_last),a
    ld (player_shields_last),a
    ld (player_fuel_last),a
; A live player must not expose stale diagnostics from the previous death.
    ld (player_dying_phase),a
    ld (player_dying_timer),a
    ld (player_dying_spawns),a
; set player sprite record with defaults
    ld ix,player_begin
    call player_entry_load_start_coords
    ld hl,player_defaults ; coordinate setup owns HL, not the allocator's defaults
    ld a,1
    call spawn_sprite_from_coords
    jp nc,@allocation_failed ; IX is invalid when the one-record player pool is full
    res 7,(ix+sprite_type)
; load sprite frames
    ld a,(ix+sprite_id)
    call vdu_sprite_select
    call vdu_sprite_clear_frames
    ld hl,BUF_SHIP_0L
    call vdu_sprite_add_buff
    ld hl,BUF_SHIP_1C
    call vdu_sprite_add_buff
    ld hl,BUF_SHIP_2R
    call vdu_sprite_add_buff
@draw:
    call player_entry_load_start_coords
    call vdu_sprite_move_abs168
; The shared entrance owns first visibility and collision publication.
    res sprite_alive,(ix+sprite_collisions)
    ld a,(player_spawn_resolved_x)
    ld (ix+player_entry_target_x_actor),a
    ld a,(player_spawn_resolved_y)
    ld (ix+player_entry_target_y_actor),a
    call player_entry_init_lateral
    ld a,1 ; start unbanked at the centre of this entrance
    ld (ix+sprite_frame),a
    call vdu_sprite_select_frame
    call vdu_sprite_show
    ld a,player_entry_total_frames
    ld (ix+player_entry_timer),a
    xor a
    ld (ix+player_grace_timer),a
    pop af
    ld (ix+player_entry_warp_enabled),a
    or a
    call nz,level_transition_init_entrance
    ld hl,0
    ld (ix+sprite_xvel),hl
    ld hl,player_entry_y_velocity
    ld (ix+sprite_yvel),hl
    ld hl,player_spawning
    ld (player_state),hl
    scf ; publish successful initialization to the caller
    ret
@allocation_failed:
    pop af
    xor a
    ret
@resolution_failed:
    pop af
    xor a
    ret
; end player_init

; Reconstruct the resolved bottom entrance in 16.8 coordinates.
; outputs: BC = start X, DE = start Y
player_entry_load_start_coords:
    ld a,(player_spawn_resolved_x)
    ld bc,0
    ld b,a
    ld a,(player_spawn_resolved_y)
    ld de,0
    ld d,a
; 224 + 96 = 320: carry must reach the upper integer byte, not wrap at 256.
    ld hl,player_entry_start_y_offset*256
    add hl,de
    ex de,hl
    ret
; end player_entry_load_start_coords

; IX = new player record. Start the sine at its centre; on a right-hand
; respawn start to the left instead. The amplitude cap is no wider than the
; distance to the nearer legal sprite edge, so even forced off-centre
; respawns can return to their safe endpoint without clipping or bouncing.
player_entry_init_lateral:
    ld a,(ix+player_entry_target_x_actor)
    ld c,0
    cp player_entry_target_x+1
    jr c,@cap
    ld c,128
    ld b,a
    ld a,sprite_right
    sub b
@cap:
    ld (ix+player_entry_angle),c
    cp player_entry_max_amplitude
    jr c,@store
    ld a,player_entry_max_amplitude
@store:
    ld (ix+player_entry_amplitude),a
    ret
; end player_entry_init_lateral

; Draw the player record after an actor-owned scripted update.
; inputs: IY = player record
player_draw_record:
    ld a,(iy+sprite_id)
    call vdu_sprite_select
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    call vdu_sprite_move_abs168
    ld a,(iy+sprite_frame)
    call vdu_sprite_select_frame
; Scripted player states pause terrain drawing, so no later plot is guaranteed
; to flush queued sprite positions. Publish the move now; otherwise the VDP
; holds every entrance/death update until grace resumes world drawing and the
; ship appears to teleport to its final position.
    call vdu_sprite_update
    ret
; end player_draw_record

player_spawning:
    ld iy,player_begin
    ld (table_pointer),iy
    call move_sprite
    call player_entry_damped_swoop
    call player_draw_record
    ld iy,player_begin
    dec (iy+player_entry_timer)
    ret nz
; Snap the final subpixel residue to the shared, reviewed endpoint.
    ld hl,0
    ld h,(iy+player_entry_target_x_actor)
    ld (iy+sprite_x),hl
    ld hl,0
    ld h,(iy+player_entry_target_y_actor)
    ld (iy+sprite_y),hl
    ld hl,0
    ld (iy+sprite_xvel),hl
    ld (iy+sprite_yvel),hl
    ld a,1
    ld (iy+sprite_frame),a
    ld a,player_grace_frames
    ld (iy+player_grace_timer),a
    call player_draw_record
    ld hl,player_grace
    ld (player_state),hl
    ret
; end player_spawning

; Damped sinusoidal approach shared by level entry and mid-level respawn.
; Ease in over 16 frames, make one broad left-right sweep, then taper the
; envelope to one pixel by the final update. The generic sine stays unchanged.
; inputs: IY = player record
player_entry_damped_swoop:
    ld hl,(iy+sprite_x)
    push hl
    ld a,player_entry_total_frames
    sub (iy+player_entry_timer) ; elapsed frames
    cp player_entry_ramp_frames
    jr nc,@full_amplitude
    add a,a
    add a,a
    jr @taper
@full_amplitude:
    ld a,player_entry_max_amplitude
@taper:
    cp (iy+player_entry_timer)
    jr c,@endpoint_cap
    ld a,(iy+player_entry_timer)
@endpoint_cap:
    cp (iy+player_entry_amplitude)
    jr c,@position
    ld a,(iy+player_entry_amplitude)
@position:
    ld bc,0
    ld b,a
    ld de,0
    ld d,(iy+player_entry_target_x_actor)
    ld a,(iy+player_entry_angle)
    call motion_damped_sine_position
    ld (iy+sprite_x),hl
; Bank from the actual published integer-pixel motion, not the sine alone.
    pop de
    ld a,h
    cp d
    ld a,1
    jr z,@bank_ready
    ld a,0
    jr c,@bank_ready
    ld a,2
@bank_ready:
    ld (iy+sprite_frame),a
    ld a,(iy+player_entry_angle)
    add a,player_entry_angle_step
    ld (iy+player_entry_angle),a
    ret
; end player_entry_damped_swoop

player_grace:
; Control begins only after the entrance settles. Collision remains disabled
; for exactly one second at the game's 60 Hz outer-frame cadence.
    call player_input
    call player_move_weapons
    ld iy,player_begin
    dec (iy+player_grace_timer)
    jp z,@finish
    ld a,(iy+sprite_id)
    call vdu_sprite_select
    ld a,(iy+player_grace_timer)
    bit 2,a
    jp z,vdu_sprite_show
    jp vdu_sprite_hide
@finish:
    ld a,(iy+sprite_id)
    call vdu_sprite_select
    call vdu_sprite_show
    set sprite_alive,(iy+sprite_collisions)
    ld hl,player_alive
    ld (player_state),hl
; Only orbiters resume pursuit. Departing seekers finish their retreat.
    ld hl,(game_state)
    ld de,game_level_transition
    or a
    sbc hl,de
    call nz,seeker_group_enter_seek
    ret
; end player_grace

player_alive:
    call player_input
    call player_move_weapons
    ret
; end player_alive

kill_player:
; update_shields callers still own their collision-record index registers
    push af
    push bc
    push ix
    push iy
; set player collision flag to dead
    ld ix,player_begin
    res sprite_alive,(ix+sprite_collisions)
; Preserve the commanded pose and initialize the actor-owned damaged flight.
    ld a,(ix+sprite_frame)
    ld (ix+player_death_base_frame),a
    call rand_8
    and %00011111
    add a,24
    ld (ix+player_death_turn_timer),a
    ld a,1
    ld (ix+player_death_wiggle_timer),a
    call rand_8
    and %00000001
    ld hl,player_death_lateral_speed
    jp z,@death_velocity_ready
    call neg_hlu
@death_velocity_ready:
    ld (ix+sprite_xvel),hl
    ld hl,player_death_forward_speed
    ld (ix+sprite_yvel),hl
; set player state to dying
    xor a
    ld (player_dying_phase),a
    ld (player_dying_timer),a
    ld (player_dying_spawns),a
    ld hl,player_dying
    ld (player_state),hl
; Publish death before notifying the seekers. The actor-owned entry preserves
; any retreat already in flight, including during a live warp.
    call seeker_group_enter_death_retreat
    pop iy
    pop ix
    pop bc
    pop af
    ret
; end kill_player

player_dying:
; This actor update is deliberately bounded to one frame. The global playing
; loop, not the actor, advances enemies, explosions, tiles, and VBlank.
    call player_move_weapons
    ld a,(player_dying_phase)
    or a
    jp z,@initial
    cp player_dying_phase_delay
    jp z,@delay
    cp player_dying_phase_finale
    jp z,@finale
; Finish only after the finale explosions have had their full lifetime.
    ld a,(player_dying_timer)
    inc a
    ld (player_dying_timer),a
    cp player_dying_finish_frames
    ret c
    ld a,STATE_NEW_EVENT_PLAYER_DEAD
    call telemetry_post_events
    ld a,TELEMETRY_LIFECYCLE_PLAYER_DEAD
    call telemetry_post_lifecycle
    ld hl,player_dead
    ld (player_state),hl
    ret

@initial:
    call player_dying_move
; Preserve the old cadence: one rising explosion every sixteen outer frames.
    ld a,(player_dying_timer)
    or a
    jp nz,@tick_initial
    call spawn_explosion_player
    jp nc,@initial_spawned
    ld hl,0
    ld (ix+sprite_xvel),hl
    ld hl,1*256
    ld (ix+sprite_yvel),hl
@initial_spawned:
    ld a,(player_dying_spawns)
    inc a
    ld (player_dying_spawns),a
@tick_initial:
    ld a,(player_dying_timer)
    inc a
    and 15
    ld (player_dying_timer),a
    ret nz
    ld a,(player_dying_spawns)
    cp 16
    ret c
    ld a,player_dying_phase_delay
    ld (player_dying_phase),a
    xor a
    ld (player_dying_spawns),a
    ret

@delay:
    ld a,(player_dying_timer)
    inc a
    ld (player_dying_timer),a
    cp 16
    ret c
    xor a
    ld (player_dying_timer),a
    .if mus_campaign
    call mus_queue_blast
    .endif
    .if mus_single
    ld a,1
    ld (mus_blast),a
    .endif
    ld a,player_dying_phase_finale
    ld (player_dying_phase),a
    ret

@finale:
; Spread allocations across frames so pool pressure can never turn this actor
; into a blocking burst. Allocation failure remains a harmless skipped effect.
    call spawn_explosion_player
    jp nc,@finale_spawned
    call @rand
    ld (ix+sprite_xvel),hl
    call @rand
    ld (ix+sprite_yvel),hl
@finale_spawned:
    ld a,(player_dying_spawns)
    inc a
    ld (player_dying_spawns),a
    cp explosions_max
    ret c
    xor a
    ld (player_dying_timer),a
    ld a,player_dying_phase_finish
    ld (player_dying_phase),a
    ret
@rand:
    ld hl,0
    call rand_8
    ld b,a
    and %00000001
    ld h,a
    ld l,b
    bit 7,b
    ret z
    call neg_hlu
    ret
; end player_dying

; Actor-owned damaged flight. Random changes are latched for a useful duration;
; a side collision always commits to the generic mover's inward reflection.
player_dying_move:
    ld iy,player_begin
    ld (table_pointer),iy
    call move_sprite
    ld a,(sprite_screen_edge)
    and collide_left+collide_right
    jp nz,@edge_turn
    dec (iy+player_death_turn_timer)
    jp nz,@wiggle
    ld hl,(iy+sprite_xvel)
    call neg_hlu
    ld (iy+sprite_xvel),hl
    call rand_8
    and %00011111
    add a,24
    ld (iy+player_death_turn_timer),a
    jp @wiggle
@edge_turn:
    ld a,32
    ld (iy+player_death_turn_timer),a
@wiggle:
    dec (iy+player_death_wiggle_timer)
    jp nz,@draw
    call rand_8
    and %00000011
    add a,4
    ld (iy+player_death_wiggle_timer),a
    ld a,(iy+player_death_base_frame)
    or a
    jp z,@base_left
    cp 2
    jp z,@base_right
; A centred command wiggles to either bank without losing the saved base pose.
    call rand_8
    and %00000001
    jp z,@frame_left
    ld a,2
    jp @store_frame
@frame_left:
    xor a
    jp @store_frame
@base_left:
    call rand_8
    and %00000001 ; frames 0 or 1
    jp @store_frame
@base_right:
    call rand_8
    and %00000001
    inc a ; frames 1 or 2
@store_frame:
    ld (iy+sprite_frame),a
@draw:
    call player_draw_record
    ret
; end player_dying_move

get_ready: asciz "Get Ready!"

player_dead:
; decrement lives
    ld a,(player_ships)
    dec a
    ld (player_ships),a
    jp nz,@respawn
; actors publish outcomes; the global playing state owns the phase transition
    ld a,player_event_game_over
    ld (player_event),a
    ret
@respawn:
    ld a,TELEMETRY_LIFECYCLE_RESPAWN
    call telemetry_post_lifecycle
    call draw_player_ships
; Preserve score, terrain position, active tiles and the current enemy world.
; The player alone begins a fresh logical lifetime and shared entrance.
    call player_init_respawn
    jp c,@player_ready
; allocation should be impossible after table_init; recover without touching IX
    ld a,player_event_game_over
    ld (player_event),a
    ret
@player_ready:
; Keep the enemy scheduler live; individual actors reconcile with the player
; lifecycle and return from holding behavior when grace expires.
    ld hl,enemies_alive
    ld (enemies_state), hl
    ret
; end player_dead
