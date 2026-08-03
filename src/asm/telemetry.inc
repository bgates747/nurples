; Purple Nurples stable runtime telemetry ABI, version 1.
;
; Host readers use telemetry_write_seq as a seqlock: read an even sequence,
; copy telemetry_size bytes, then accept the copy only if the sequence is still
; the same even value. All multi-byte values are little-endian.

TELEMETRY_VERSION_MAJOR: equ 1
TELEMETRY_VERSION_MINOR: equ 0
TELEMETRY_FLAG_LEGACY_POINTER_BRIDGE: equ 0
TELEMETRY_FLAG_SESSION_VALID: equ 1
TELEMETRY_FLAG_LEVEL_VALID: equ 2
TELEMETRY_FLAG_ACTORS_VALID: equ 3

TELEMETRY_LIFECYCLE_RESPAWN:     equ %00000001
TELEMETRY_LIFECYCLE_PLAYER_DEAD: equ %00000010
TELEMETRY_DISPATCH_IDLE: equ 0

TELEMETRY_OUTCOME_RUNNING:          equ 0
TELEMETRY_OUTCOME_LEVEL_TRANSITION: equ 1
TELEMETRY_OUTCOME_GAME_OVER:        equ 2
TELEMETRY_OUTCOME_VICTORY:          equ 3
TELEMETRY_OUTCOME_EXIT:             equ 4
TELEMETRY_OUTCOME_FAULT:            equ 5
TELEMETRY_OUTCOME_RESPAWN:          equ 6
TELEMETRY_OUTCOME_PLAYER_DEAD:      equ 7

telemetry_begin:
telemetry_magic:                    db "PNST" ; 00
telemetry_version_major:            db TELEMETRY_VERSION_MAJOR ; 04
telemetry_version_minor:            db TELEMETRY_VERSION_MINOR ; 05
telemetry_size:                     dw telemetry_end-telemetry_begin ; 06
telemetry_write_seq:                db 1 ; 08; invalid until telemetry_init completes
telemetry_flags:                    db 0 ; 09
telemetry_header_reserved:          db 0,0,0,0,0,0 ; 0A

telemetry_global_current:           db STATE_NEW_NONE ; 10
telemetry_global_previous:          db STATE_NEW_NONE ; 11
telemetry_global_pending:           db STATE_NEW_NONE ; 12
telemetry_global_policy:            db 0 ; 13
telemetry_dispatcher_phase:         db TELEMETRY_DISPATCH_IDLE ; 14
telemetry_outcome:                  db TELEMETRY_OUTCOME_RUNNING ; 15
telemetry_player_current:           db STATE_NEW_NONE ; 16
telemetry_player_previous:          db STATE_NEW_NONE ; 17
telemetry_player_pending:           db STATE_NEW_NONE ; 18
telemetry_player_dying_phase:       db 0 ; 19
telemetry_player_dying_timer:       db 0 ; 1A
telemetry_player_dying_spawns:      db 0 ; 1B
telemetry_enemies_current:          db STATE_NEW_NONE ; 1C
telemetry_enemies_previous:         db STATE_NEW_NONE ; 1D
telemetry_enemies_pending:          db STATE_NEW_NONE ; 1E
telemetry_last_lifecycle_events:    db 0 ; 1F

telemetry_total_frames:             dl 0 ; 20
telemetry_global_state_frames:      dl 0 ; 23
telemetry_global_transition_serial: dl 0 ; 26
telemetry_player_state_frames:      dl 0 ; 29
telemetry_player_transition_serial: dl 0 ; 2C
telemetry_enemies_state_frames:     dl 0 ; 2F
telemetry_enemies_transition_serial: dl 0 ; 32
telemetry_pending_events:           db 0 ; 35
telemetry_frame_events:             db 0 ; 36
telemetry_last_transition_events:   db 0 ; 37
telemetry_events_seen:              db 0 ; 38
telemetry_lifecycle_serial:         dl 0 ; 39

telemetry_fault_domains:            dw 0 ; 3C
telemetry_fault_game:               db 0 ; 3E
telemetry_fault_player:             db 0 ; 3F
telemetry_fault_enemies:            db 0 ; 40
telemetry_fault_sprite:             db 0 ; 41
telemetry_fault_tile:               db 0 ; 42
telemetry_fault_timer:              db 0 ; 43
telemetry_fault_vdu:                db 0 ; 44
telemetry_fault_coordinator:        db 0 ; 45
telemetry_fault_player_controller:  db 0 ; 46
telemetry_fault_enemy_controller:   db 0 ; 47
telemetry_fault_telemetry:          db 0 ; 48
telemetry_player_ships:             db 0 ; 49
telemetry_player_shields:           db 0 ; 4A
telemetry_tileset:                  db 0 ; 4B
telemetry_level:                    db 0 ; 4C
telemetry_rows_remaining:           dl 0 ; 4D
telemetry_end:

; Producer-side latches are deliberately outside the public ABI. Callers can
; post between snapshots without making an even public snapshot incoherent.
telemetry_event_latch:             db 0
telemetry_lifecycle_latch:         db 0
telemetry_forced_outcome:          db 0
telemetry_count_frame:             db 0
telemetry_legacy_phase:            db STATE_NEW_BOOT

; Stable byte offsets exported alongside label symbols.
TELEMETRY_OFFSET_WRITE_SEQ: equ telemetry_write_seq-telemetry_begin
TELEMETRY_OFFSET_GLOBAL_CURRENT: equ telemetry_global_current-telemetry_begin
TELEMETRY_OFFSET_PLAYER_CURRENT: equ telemetry_player_current-telemetry_begin
TELEMETRY_OFFSET_ENEMIES_CURRENT: equ telemetry_enemies_current-telemetry_begin
TELEMETRY_OFFSET_FAULT_DOMAINS: equ telemetry_fault_domains-telemetry_begin

; Inputs: A = one or more STATE_NEW_EVENT_* bits.
; Destroys: A, HL.
telemetry_post_events:
    ld hl,telemetry_event_latch
    or (hl)
    ld (hl),a
    ret

; Inputs: A = one or more TELEMETRY_LIFECYCLE_* bits.
; Destroys: A, HL.
telemetry_post_lifecycle:
    ld hl,telemetry_lifecycle_latch
    or (hl)
    ld (hl),a
    ret

telemetry_request_exit:
    ld a,STATE_NEW_EVENT_QUIT
    call telemetry_post_events
    ld a,TELEMETRY_OUTCOME_EXIT
    ld (telemetry_forced_outcome),a
    ret

; Recreate the complete ABI header and dynamic state on every MOS RUN.
; Destroys: AF, BC, DE, HL, IX.
telemetry_init:
    ld a,1
    ld (telemetry_write_seq),a
    ld hl,telemetry_flags
    ld de,telemetry_flags+1
    ld bc,telemetry_end-telemetry_flags-1
    xor a
    ld (hl),a
    ldir
    ld (telemetry_event_latch),a
    ld (telemetry_lifecycle_latch),a
    ld (telemetry_forced_outcome),a
    ld (telemetry_count_frame),a
    ld a,$50 ; P
    ld (telemetry_magic+0),a
    ld a,$4E ; N
    ld (telemetry_magic+1),a
    ld a,$53 ; S
    ld (telemetry_magic+2),a
    ld a,$54 ; T
    ld (telemetry_magic+3),a
    ld a,TELEMETRY_VERSION_MAJOR
    ld (telemetry_version_major),a
    ld a,TELEMETRY_VERSION_MINOR
    ld (telemetry_version_minor),a
    ld a,telemetry_end-telemetry_begin
    ld (telemetry_size),a
    xor a
    ld (telemetry_size+1),a
    ld a,%00000001
    ld (telemetry_flags),a
    ld a,STATE_NEW_NONE
    ld (telemetry_global_previous),a
    ld (telemetry_global_pending),a
    ld (telemetry_player_previous),a
    ld (telemetry_player_pending),a
    ld (telemetry_enemies_previous),a
    ld (telemetry_enemies_pending),a
    ld a,STATE_NEW_BOOT
    ld (telemetry_global_current),a
    ld (telemetry_legacy_phase),a
    ld a,STATE_NEW_PLAYER_INACTIVE
    ld (telemetry_player_current),a
    ld a,STATE_TELEMETRY_ENEMY_INACTIVE
    ld (telemetry_enemies_current),a
    call telemetry_load_policy
    xor a
    ld (telemetry_write_seq),a
    ret

; Publish one coherent snapshot after an outer scheduler iteration.
; Destroys: AF, BC, DE, HL, IX.
telemetry_tick:
    ld c,1
    jp telemetry_publish_snapshot

; Publish terminal state without claiming another completed/VBlank frame.
telemetry_publish_terminal:
    ld c,0
    jp telemetry_publish_snapshot

; Publish a legacy startup phase without claiming a completed scheduler frame.
; Input: A = stable STATE_NEW_* phase ID.
telemetry_publish_phase:
    ld (telemetry_legacy_phase),a
    ld c,0

telemetry_publish_snapshot:
    ld a,c
    ld (telemetry_count_frame),a
    ld a,(telemetry_write_seq)
    inc a
    or 1
    ld (telemetry_write_seq),a
    ld a,(telemetry_event_latch)
    ld (telemetry_frame_events),a
    ld hl,telemetry_events_seen
    or (hl)
    ld (hl),a
; The bridge consumes every posted event into this publication. The numeric
; coordinator will publish events it deliberately leaves pending here.
    xor a
    ld (telemetry_pending_events),a
    ld a,(telemetry_lifecycle_latch)
    or a
    jp z,@lifecycle_done
    ld (telemetry_last_lifecycle_events),a
    ld hl,(telemetry_lifecycle_serial)
    inc hl
    ld (telemetry_lifecycle_serial),hl
@lifecycle_done:
    call telemetry_track_global
    call telemetry_update_validity
    call telemetry_track_player
    call telemetry_track_enemies
    call telemetry_copy_runtime
    ld a,(telemetry_count_frame)
    or a
    jp z,@frame_done
    ld hl,(telemetry_total_frames)
    inc hl
    ld (telemetry_total_frames),hl
@frame_done:
    xor a
    ld (telemetry_event_latch),a
    ld (telemetry_lifecycle_latch),a
    ld a,(telemetry_write_seq)
    inc a
    and %11111110
    ld (telemetry_write_seq),a
    ret

telemetry_track_global:
    call telemetry_map_game_state
    ld b,a
    ld a,(telemetry_global_current)
    cp b
    jp z,@same
    ld (telemetry_global_previous),a
    ld a,b
    ld (telemetry_global_current),a
    ld hl,(telemetry_global_transition_serial)
    inc hl
    ld (telemetry_global_transition_serial),hl
    ld hl,0
    ld (telemetry_global_state_frames),hl
    ld a,(telemetry_frame_events)
    ld (telemetry_last_transition_events),a
    call telemetry_load_policy
    ret
@same:
    ld a,(telemetry_count_frame)
    or a
    ret z
    ld hl,(telemetry_global_state_frames)
    inc hl
    ld (telemetry_global_state_frames),hl
    ret

telemetry_track_player:
    call telemetry_map_player_state
    ld b,a
    ld a,(telemetry_player_current)
    cp b
    jp z,@same
    ld (telemetry_player_previous),a
    ld a,b
    ld (telemetry_player_current),a
    ld hl,(telemetry_player_transition_serial)
    inc hl
    ld (telemetry_player_transition_serial),hl
    ld hl,0
    ld (telemetry_player_state_frames),hl
    ret
@same:
    ld a,(telemetry_count_frame)
    or a
    ret z
    ld hl,(telemetry_player_state_frames)
    inc hl
    ld (telemetry_player_state_frames),hl
    ret

telemetry_track_enemies:
    call telemetry_map_enemies_state
    ld b,a
    ld a,(telemetry_enemies_current)
    cp b
    jp z,@same
    ld (telemetry_enemies_previous),a
    ld a,b
    ld (telemetry_enemies_current),a
    ld hl,(telemetry_enemies_transition_serial)
    inc hl
    ld (telemetry_enemies_transition_serial),hl
    ld hl,0
    ld (telemetry_enemies_state_frames),hl
    ret
@same:
    ld a,(telemetry_count_frame)
    or a
    ret z
    ld hl,(telemetry_enemies_state_frames)
    inc hl
    ld (telemetry_enemies_state_frames),hl
    ret

telemetry_copy_runtime:
    ld a,(player_dying_phase)
    ld (telemetry_player_dying_phase),a
    ld a,(player_dying_timer)
    ld (telemetry_player_dying_timer),a
    ld a,(player_dying_spawns)
    ld (telemetry_player_dying_spawns),a
    ld a,(player_ships)
    ld (telemetry_player_ships),a
    ld a,(player_shields)
    ld (telemetry_player_shields),a
    ld a,(tileset_ptr_idx)
    ld (telemetry_tileset),a
    ld a,(cur_level)
    ld (telemetry_level),a
    ld hl,(tiles_row)
    ld (telemetry_rows_remaining),hl

    ld c,0
    ld a,(game_fault_flags)
    ld (telemetry_fault_game),a
    or a
    jp z,@player_fault
    set 0,c
@player_fault:
    ld a,(player_fault_flags)
    ld (telemetry_fault_player),a
    or a
    jp z,@enemies_fault
    set 1,c
@enemies_fault:
    ld a,(enemies_fault_flags)
    ld (telemetry_fault_enemies),a
    or a
    jp z,@sprite_fault
    set 2,c
@sprite_fault:
    ld a,(sprite_fault_flags)
    ld (telemetry_fault_sprite),a
    or a
    jp z,@tile_fault
    set 3,c
@tile_fault:
    ld a,(tile_fault_flags)
    ld (telemetry_fault_tile),a
    or a
    jp z,@timer_fault
    set 4,c
@timer_fault:
    ld a,(timer_fault_flags)
    ld (telemetry_fault_timer),a
    or a
    jp z,@vdu_fault
    set 5,c
@vdu_fault:
    ld a,(vdu_timing_fault_flags)
    ld (telemetry_fault_vdu),a
    or a
    jp z,@fault_mask
    set 6,c
@fault_mask:
    ld a,c
    ld (telemetry_fault_domains),a
    xor a
    ld (telemetry_fault_domains+1),a
    ld a,c
    or a
    jp nz,@fault

    ld a,(telemetry_forced_outcome)
    or a
    jp nz,@store_outcome

    ld a,(telemetry_global_current)
    cp STATE_NEW_LEVEL_TRANSITION
    jp z,@level_transition
    cp STATE_NEW_GAME_OVER
    jp z,@game_over
    cp STATE_NEW_VICTORY
    jp z,@victory
    cp STATE_NEW_EXIT
    jp z,@exit
    ld a,(telemetry_lifecycle_latch)
    and TELEMETRY_LIFECYCLE_RESPAWN
    jp nz,@respawn
    ld a,(telemetry_frame_events)
    and STATE_NEW_EVENT_PLAYER_DEAD
    jp nz,@player_dead
    xor a
    ld (telemetry_outcome),a
    ret
@level_transition:
    ld a,TELEMETRY_OUTCOME_LEVEL_TRANSITION
    ld (telemetry_outcome),a
    ret
@game_over:
    ld a,TELEMETRY_OUTCOME_GAME_OVER
    ld (telemetry_outcome),a
    ret
@victory:
    ld a,TELEMETRY_OUTCOME_VICTORY
    ld (telemetry_outcome),a
    ret
@exit:
    ld a,TELEMETRY_OUTCOME_EXIT
    jp @store_outcome
@respawn:
    ld a,TELEMETRY_OUTCOME_RESPAWN
    jp @store_outcome
@player_dead:
    ld a,TELEMETRY_OUTCOME_PLAYER_DEAD
@store_outcome:
    ld (telemetry_outcome),a
    ret
@fault:
    ld a,TELEMETRY_OUTCOME_FAULT
    ld (telemetry_outcome),a
    ret

; Convert the current legacy handler pointers to stable numeric IDs.
telemetry_map_game_state:
    ld hl,(game_state)
    ld de,game_init
    push hl
    or a
    sbc hl,de
    pop hl
    jp nz,@map_pointer
    ld a,(telemetry_legacy_phase)
    ret
@map_pointer:
; If a legacy handler schedules game_init, publish NEW_GAME until game_init
; itself announces LOADING. Never reuse the prior startup's final phase.
    ld a,STATE_NEW_NEW_GAME
    ld (telemetry_legacy_phase),a
    ld ix,telemetry_game_state_map
    ld b,telemetry_game_state_count
    jp telemetry_map_pointer

telemetry_map_player_state:
    ld a,(telemetry_flags)
    bit TELEMETRY_FLAG_ACTORS_VALID,a
    jp nz,@map_pointer
    ld a,STATE_NEW_PLAYER_INACTIVE
    ret
@map_pointer:
    ld hl,(player_state)
    ld ix,telemetry_player_state_map
    ld b,telemetry_player_state_count
    jp telemetry_map_pointer

telemetry_map_enemies_state:
    ld a,(telemetry_flags)
    bit TELEMETRY_FLAG_ACTORS_VALID,a
    jp nz,@map_pointer
    ld a,STATE_TELEMETRY_ENEMY_INACTIVE
    ret
@map_pointer:
    ld hl,(enemies_state)
    ld ix,telemetry_enemies_state_map
    ld b,telemetry_enemies_state_count
    jp telemetry_map_pointer

; Inputs: HL = candidate pointer, IX = {dl pointer, db id} table, B = entries.
; Returns: A = mapped ID or STATE_NEW_NONE. Destroys: AF, B, DE, IX.
telemetry_map_pointer:
@loop:
    ld de,(ix)
    push hl
    or a
    sbc hl,de
    pop hl
    jp z,@found
    lea ix,ix+4
    djnz @loop
    ld a,STATE_NEW_NONE
    ret
@found:
    ld a,(ix+3)
    ret

; Session-owned values become meaningful only after game_init has successfully
; published PLAYING. This runs before actor mapping in the same snapshot.
telemetry_update_validity:
    ld a,(telemetry_global_current)
    cp STATE_NEW_PLAYING
    jp c,@startup
    ret nz ; later phases retain the most recently valid session payload
    ld hl,telemetry_flags
    set TELEMETRY_FLAG_SESSION_VALID,(hl)
    set TELEMETRY_FLAG_LEVEL_VALID,(hl)
    set TELEMETRY_FLAG_ACTORS_VALID,(hl)
    ret
@startup:
; BOOT through COUNTDOWN invalidate any previous session-facing payload.
    ld hl,telemetry_flags
    res TELEMETRY_FLAG_SESSION_VALID,(hl)
    res TELEMETRY_FLAG_LEVEL_VALID,(hl)
    res TELEMETRY_FLAG_ACTORS_VALID,(hl)
    ret

telemetry_load_policy:
    ld a,(telemetry_global_current)
    cp STATE_NEW_COUNT
    jp nc,@legacy
    ld ix,telemetry_policy_table
    ld de,0
    ld e,a
    add ix,de
    ld a,(ix)
    ld (telemetry_global_policy),a
    ret
@legacy:
    ld a,STATE_NEW_POLICY_INPUT+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    ld (telemetry_global_policy),a
    ret

telemetry_game_state_map:
    dl game_main_menu
    db STATE_NEW_TITLE
    dl game_paused
    db STATE_NEW_PAUSED
    dl game_level_transition
    db STATE_NEW_LEVEL_TRANSITION
    dl game_over
    db STATE_NEW_GAME_OVER
    dl game_victory
    db STATE_NEW_VICTORY
    dl game_loading
    db STATE_NEW_LOADING
    dl game_cutscene
    db STATE_LEGACY_CUTSCENE
    dl game_inventory
    db STATE_LEGACY_INVENTORY
    dl game_map
    db STATE_LEGACY_MAP
    dl game_exit
    db STATE_NEW_EXIT
    dl game_playing
    db STATE_NEW_PLAYING
telemetry_game_state_map_end:
telemetry_game_state_count: equ [telemetry_game_state_map_end-telemetry_game_state_map]/4

telemetry_player_state_map:
    dl player_alive
    db STATE_NEW_PLAYER_ACTIVE
    dl player_dying
    db STATE_NEW_PLAYER_DYING
    dl player_dead
    db STATE_NEW_PLAYER_DEAD
telemetry_player_state_map_end:
telemetry_player_state_count: equ [telemetry_player_state_map_end-telemetry_player_state_map]/4

telemetry_enemies_state_map:
    dl enemies_alive
    db STATE_TELEMETRY_ENEMY_ACTIVE
    dl enemies_despawning
    db STATE_TELEMETRY_ENEMY_DESPAWNING
    dl enemies_dead
    db STATE_TELEMETRY_ENEMY_DEAD
    dl enemies_respawning
    db STATE_TELEMETRY_ENEMY_RESPAWNING
telemetry_enemies_state_map_end:
telemetry_enemies_state_count: equ [telemetry_enemies_state_map_end-telemetry_enemies_state_map]/4

telemetry_policy_table:
    db STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_INPUT+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_ACTOR_RENDER+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_INPUT+STATE_NEW_POLICY_ACTOR_RENDER+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_INPUT+STATE_NEW_POLICY_ACTOR_UPDATE+STATE_NEW_POLICY_ACTOR_RENDER+STATE_NEW_POLICY_WORLD_UPDATE+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_INPUT+STATE_NEW_POLICY_ACTOR_RENDER+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_ACTOR_RENDER+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_ACTOR_RENDER+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_INPUT+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db STATE_NEW_POLICY_INPUT+STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
    db 0
    db STATE_NEW_POLICY_WORLD_RENDER+STATE_NEW_POLICY_STATE_RENDER
