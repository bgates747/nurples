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
player_fault_bad_state: equ 0
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
    ld de,player_dying
    call @matches
    jp z,@valid
    ld de,player_dead
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

; set initial player position
; inputs: none,everything is hardcoded
; outputs: player_x/y set to bottom-left corner of screen
; destroys: pretty much everything
player_init:
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
    ld hl,player_defaults
    ld bc,0*256
    ld de,sprite_bottom*256
    ld a,1
    call spawn_sprite_from_coords
    ret nc ; IX is invalid when the one-record player pool is full
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
    ld bc,0*256
    ld de,sprite_bottom*256
    call vdu_sprite_move_abs168
    call vdu_sprite_show
    ld hl,player_alive
    ld (player_state),hl
    set sprite_alive,(ix+sprite_collisions)
    scf ; publish successful initialization to the caller
    ret
; end player_init

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
; change seeker move program to move_enemy_seeker_orbit
    ld iy,seekers_base
    ld hl,move_enemy_seeker_orbit
    ld b,seekers_max
    call change_sprite_group_move_program
; set player state to dying
    xor a
    ld (player_dying_phase),a
    ld (player_dying_timer),a
    ld (player_dying_spawns),a
    ld hl,player_dying
    ld (player_state),hl
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

get_ready: asciz "Get Ready!"

player_dead:
; reset player score to what it was at the start of the level
    ld hl,(player_starting_score)
    ld (player_score),hl
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
    call reset_runtime_tables
; ; reset player score to what it was at the start of the level
;     ld hl,(player_starting_score)
;     ld (player_score),hl
; restart the level
    call init_level
; spawn player sprite at starting position
    call player_init
    jp c,@player_ready
; allocation should be impossible after table_init; recover without touching IX
    ld a,player_event_game_over
    ld (player_event),a
    ret
@player_ready:
; set game playing substates
    ld hl,enemies_alive
    ld (enemies_state), hl
    ret
; end player_dead
