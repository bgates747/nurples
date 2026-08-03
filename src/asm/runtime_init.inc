; Process-entry initialization
; ============================
;
; MOS RUN re-enters the resident binary; it does not restore DB/DL initializers.
; Only state whose lifetime is one application invocation belongs here. Session,
; level, life, table, loader, and arithmetic scratch state is initialized by its
; owning routine before first use.

runtime_process_init:
; Process control bytes are ordinary resident RAM. Reset them explicitly on
; every MOS RUN instead of relying on their assembled DB values.
    xor a
    ld (game_exit_requested),a
    ld (player_event),a
    ld (player_dying_phase),a
    ld (player_dying_timer),a
    ld (player_dying_spawns),a

; Diagnostic latches describe this invocation only.
    ld (game_fault_flags),a
    ld (enemies_fault_flags),a
    ld (player_fault_flags),a
    ld (sprite_fault_flags),a
    ld (tile_fault_flags),a
    ld (timer_fault_flags),a
    ld (vdu_timing_fault_flags),a

; Small persistent helpers that otherwise inherit their previous RUN values.
    ld (seekers_count),a
    ld (sprite_screen_edge),a
    ld (oldTimeStamp),a
    ld hl,0
    ld (seed1),hl
    ld (seed2),hl

; Publish valid dispatcher states before the first tick. Their owning init
; routines will republish the same operational states after resources exist.
    ld hl,game_init
    ld (game_state),hl
    ld hl,enemies_alive
    ld (enemies_state),hl
    ld hl,player_alive
    ld (player_state),hl

; Restore the documented random seed and establish a current clock baseline.
    ld a,$50
    ld (r_seed),a
    call timestamp_init
    ret
; end runtime_process_init
