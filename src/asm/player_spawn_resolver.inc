; Deterministic safe endpoint selection for the shared player entrance.
;
; The resolver publishes only a bounded candidate result. player_init copies
; that result into named fields in the newly allocated player record before it
; publishes player_spawning.
;
; Candidate order is centre-out. Positions retain the existing X=120 visual
; centre, advance in 16-pixel steps, then test the two legal edge positions.
; Each target tests the final 16x16 ship box plus the 64-pixel wake immediately
; ahead of it (toward decreasing Y), represented as one 16x80 probe.

PLAYER_SPAWN_DIRECTION_POSITIVE_X: equ 0
PLAYER_SPAWN_DIRECTION_NEGATIVE_X: equ 1

player_spawn_fault_no_candidate: equ 0
player_spawn_fault_bad_stack: equ 1
player_spawn_fault_bad_record: equ 2
player_spawn_fault_inactive_record: equ 3

player_spawn_resolved_x: db 0
player_spawn_resolved_y: db 0
player_spawn_resolved_direction: db PLAYER_SPAWN_DIRECTION_POSITIVE_X
; Per-call result, cleared by player_spawn_resolve_endpoint. This is not a
; cross-RUN diagnostic latch and therefore is deliberately not a *_fault_flags
; process-global.
player_spawn_resolver_result_flags: db 0

player_spawn_candidate_x: db 0
player_spawn_candidate_y: db 0
player_spawn_x_remaining: db 0
player_spawn_y_remaining: db 0
player_spawn_tile_remaining: db 0
player_spawn_x_cursor: dl 0
player_spawn_y_cursor: dl 0
player_spawn_tile_cursor: dl 0

; Only the common collision fields are used. Keeping a full record-sized probe
; lets the production build reuse check_collision_box without a second AABB
; implementation or an accidental abbreviated record contract.
player_spawn_probe: blkb actor_record_size,0

player_spawn_candidate_xs:
    db 120,104,136,88,152,72,168,56,184,40,200,24,216,8,232,0,240
player_spawn_candidate_xs_end:
player_spawn_candidate_ys:
    db 224,208,192,176
player_spawn_candidate_ys_end:

player_spawn_candidate_x_count: equ player_spawn_candidate_xs_end-player_spawn_candidate_xs
player_spawn_candidate_y_count: equ player_spawn_candidate_ys_end-player_spawn_candidate_ys

    .if player_spawn_candidate_x_count-17
        dl player_spawn_candidate_x_count_mismatch
    .endif
    .if player_spawn_candidate_y_count-4
        dl player_spawn_candidate_y_count_mismatch
    .endif

; Resolve the first safe endpoint in the frozen centre-out search order.
; outputs:
;   carry set: resolved_x/resolved_y/resolved_direction are valid
;   carry clear: no safe endpoint or corrupt active-tile state; fault is latched
; destroys: everything except SP
player_spawn_resolve_endpoint:
    xor a
    ld (player_spawn_resolver_result_flags),a
    ld hl,player_spawn_candidate_xs
    ld (player_spawn_x_cursor),hl
    ld a,player_spawn_candidate_x_count
    ld (player_spawn_x_remaining),a
@next_x:
    ld hl,(player_spawn_x_cursor)
    ld a,(hl)
    ld (player_spawn_candidate_x),a
    inc hl
    ld (player_spawn_x_cursor),hl
    ld hl,player_spawn_candidate_ys
    ld (player_spawn_y_cursor),hl
    ld a,player_spawn_candidate_y_count
    ld (player_spawn_y_remaining),a
@next_y:
    ld hl,(player_spawn_y_cursor)
    ld a,(hl)
    ld (player_spawn_candidate_y),a
    inc hl
    ld (player_spawn_y_cursor),hl
    call player_spawn_prepare_probe
    call player_spawn_candidate_clear
    jp c,@found
    ld a,(player_spawn_resolver_result_flags)
    or a
    jp nz,@failed
    ld hl,player_spawn_y_remaining
    dec (hl)
    jp nz,@next_y
    ld hl,player_spawn_x_remaining
    dec (hl)
    jp nz,@next_x
    ld hl,player_spawn_resolver_result_flags
    set player_spawn_fault_no_candidate,(hl)
@failed:
    xor a
    ret
@found:
    ld a,(player_spawn_candidate_x)
    ld (player_spawn_resolved_x),a
    cp 120
    ld a,PLAYER_SPAWN_DIRECTION_NEGATIVE_X
    jp c,@direction_ready
    ld a,PLAYER_SPAWN_DIRECTION_POSITIVE_X
@direction_ready:
    ld (player_spawn_resolved_direction),a
    ld a,(player_spawn_candidate_y)
    ld (player_spawn_resolved_y),a
    scf
    ret
; end player_spawn_resolve_endpoint

; Populate the collision probe for the current candidate. Fixed-point actor
; coordinates are little-endian 16.8, so H receives an integer <=255 after
; clearing the full ADL register.
player_spawn_prepare_probe:
    ld ix,player_spawn_probe
    ld hl,0
    ld a,(player_spawn_candidate_x)
    ld h,a
    ld (ix+sprite_x),hl
    ld hl,0
    ld a,(player_spawn_candidate_y)
    sub 64
    ld h,a
    ld (ix+sprite_y),hl
    ld a,16
    ld (ix+sprite_dim_x),a
    ld a,80
    ld (ix+sprite_dim_y),a
    ret
; end player_spawn_prepare_probe

; Check the current probe against every live dense-stack record whose
; tile_shield_damage is nonzero. Corrupt stack/table state aborts the entire
; resolver rather than allowing an endpoint to be published from weak data.
; outputs: carry set if clear; carry reset if blocked or corrupt
player_spawn_candidate_clear:
    ld a,(num_active_tiles)
    ld (player_spawn_tile_remaining),a
    ld hl,tile_stack
    ld (player_spawn_tile_cursor),hl
    or a
    jp z,@clear
@next_tile:
    ld hl,(player_spawn_tile_cursor)
    call tile_stack_pointer_valid
    jp nc,@bad_stack
    ld hl,(player_spawn_tile_cursor)
    ld hl,(hl)
    push hl
    pop iy
    call tile_table_record_pointer_valid
    jp nc,@bad_record
    bit sprite_alive,(iy+tile_collisions)
    jp z,@inactive_record
    ld a,(iy+tile_shield_damage)
    or a
    jp z,@advance
    ld ix,player_spawn_probe
    call check_collision_box
    jp z,@blocked
@advance:
    ld hl,(player_spawn_tile_cursor)
    inc hl
    inc hl
    inc hl
    ld (player_spawn_tile_cursor),hl
    ld hl,player_spawn_tile_remaining
    dec (hl)
    jp nz,@next_tile
@clear:
    scf
    ret
@blocked:
    xor a
    ret
@bad_stack:
    ld hl,player_spawn_resolver_result_flags
    set player_spawn_fault_bad_stack,(hl)
    xor a
    ret
@bad_record:
    ld hl,player_spawn_resolver_result_flags
    set player_spawn_fault_bad_record,(hl)
    xor a
    ret
@inactive_record:
    ld hl,player_spawn_resolver_result_flags
    set player_spawn_fault_inactive_record,(hl)
    xor a
    ret
; end player_spawn_candidate_clear
