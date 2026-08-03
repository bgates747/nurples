# 2026-08-02 — State-machine architecture audit

## Scope

Trace the state architecture that evolved in Purple Nurples, identify definite
control-flow defects and plausible responsiveness hazards, and create a
standalone migration scaffold in `src/asm/state_new.inc`. The scaffold is not
included by `nurples.asm` and has no runtime effect on the current game.

## Intended architecture recovered from code and history

The code is a deliberate hierarchical dispatcher rather than one accidental
state pointer. Its three layers are:

1. `game_state` selects the global game phase through `do_game`.
2. `player_state` and `enemies_state` select actor/domain lifecycle handlers
   inside the playing phase.
3. Each active sprite or tile has a `sprite_move_program` or
   `tile_move_program` strategy callback.

Commits `7a3fa28` ("begin implementing state-based main loop branching
beginning with player") and `039ff0d` ("basic gamestate working") confirm that
this hierarchy was intentional. The earlier enemy respawn code in `039ff0d`
also demonstrates the original cooperative model: one state set a timer and
returned, later states polled it once per frame, and the dispatcher remained in
control.

The current outer loop still expresses that intended cadence:

1. update the global timestamp;
2. dispatch one game-state handler;
3. wait for one VBlank; and
4. poll Escape.

The architecture became unstable as initialization, simulation, rendering,
blocking presentation sequences, and transition side effects accumulated
inside the same handlers.

## Author decision: actor-owned reconciliation

The global coordinator owns only the global game phase and its published
policy. Every actor owns its mini state machine. Before an actor-local handler
runs, an actor-supplied reconciliation callback reads the global phase and
adjusts that actor if necessary. The master scheduler may tick actors, but it
must never inspect the player/enemy state and assign an actor-local state.

For new indirect dispatch, the operating paradigm is a validated numeric ID
and fixed callback table reached through a real `call` and tail `jp (hl)`. This
uses the ordinary return address already on the stack and avoids the legacy
macro's unconditional BC cost. The legacy `CALL_HL` implementation is not
itself defective and will not be rewritten merely for consistency. Legacy code
stays as-is unless it produces a known unsafe state, an unintended branch, or
must be replaced at a specifically reviewed migration boundary.

This restraint applies during stabilization, not forever. Once the new state
architecture is proven working, the Author wants a deliberate stylistic review
back on the table so the codebase can converge on consistent idioms. That work
is a distinct reviewed phase rather than part of functional migration.

This produces a one-way coordination boundary:

```text
global phase/policy -> actor reconcile -> actor-local state/update
actor event         -> global coordinator on a later safe boundary
```

For example, a dying player finishes its own bounded local sequence and posts
`PLAYER_DEAD`; the global coordinator chooses `RESPAWN` or `GAME_OVER`; the
player observes the resulting global phase on its next preflight and chooses
its own `READY` or `INACTIVE` local state. No global handler writes
`player_state`.

## Definite legacy defects

These are source-proven defects. They are not all proven causes of the observed
post-start unresponsiveness.

1. `state.inc` retains placeholder input/display routines that only `ret`, but
   their callers branch on Z/NZ. Menu, pause, victory, cutscene, inventory, and
   map decisions therefore consume unrelated incoming flags.
2. Several handlers `jp` directly into another handler without updating
   `game_state`. A paused handler can execute one playing frame while remaining
   stored as paused; victory, cutscene, inventory, and map have the same stale
   state behavior.
3. `player_dead` tail-jumps into `game_over` while the stored global state is
   still `game_playing`. When the blocking game-over handler returns, the old
   playing frame continues through enemies, explosions, tiles, and HUD work
   before the next dispatch observes `game_init`.
4. State handlers block the outer scheduler by construction. `game_init` waits
   for a key, `init_level` busy-waits through a six-second countdown,
   `game_over` delays and waits for a key, and `player_dying` performs hundreds
   of internal world/VBlank iterations. During those spans the outer timestamp,
   transition arbitration, and Escape polling do not run.
5. Before repair, the `player_dying` finale entered its animation loop with B
   already zero from the preceding `djnz`. The next `djnz` wraps and executes
   256 iterations. The full death handler performed 528 internal
   animation/VBlank iterations rather than one bounded update per outer frame.
6. The same finale has a position-dependent but readily reproducible memory
   corruption path. If the latest preliminary explosion survives (normally
   when the player dies roughly 32 pixels or more above the bottom boundary),
   only seven slots are free when eight finale explosions are requested. On
   failure, `table_get_next_id` leaves IX one record past the explosion group,
   which is `table_end`; `player_dying` ignores carry and writes x/y velocities
   through IX. Because `tile_table_base == table_end`, those stores overwrite
   fields in tile-table record zero, whether or not that record is active. A
   pool filled by any other route reaches the same unsafe write.
7. `player_init` treats a failed player-sprite allocation as a draw path. Its
   one-record allocation failure also leaves IX past the group, after which the
   routine draws and sets the alive bit through an invalid pointer.
8. Natural level completion bypasses `game_level_transition`: `tiles_plot`
   tail-jumps directly into `next_level`, whose initialization/countdown blocks,
   and then execution resumes in the old playing frame.
9. New-game and respawn paths duplicate table, sprite, tile, player, and three
   state-pointer resets. `player_init` writes its own state and its callers write
   it again, making reset ownership and ordering implicit.
10. Global, player, enemy, sprite, and tile dispatch all use writable raw code
    pointers without ID/range validation. Any adjacent memory corruption can
    become an arbitrary indirect jump rather than a recoverable fault.
11. `init_bar_shields` performs an ADL 24-bit load from the byte-sized
    `player_shields_max`, unintentionally incorporating the following two bytes.
    The initial layout currently masks the value error, but the access violates
    the field-width contract and is unsafe lifecycle initialization code.
12. Several lookup indexes use `MLT` and then add the result to a 24-bit
    pointer without clearing the register's upper byte. The eZ80 operation is
    explicitly `rr[15:0] <- rr[15:8] * rr[7:0]`, so `DEU` or `HLU` survives.
    This affects tileset and level lookup, active-tile allocation/stack
    arithmetic, tile activation dispatch, and the sine lookup. A stale `04h`
    can therefore move a lookup roughly `0x040000` bytes away from its table.
13. `do_active_tiles` accepts tile IDs 1 through 16, but
    `tile_activation_routines` contains only indices 0 through 15 and places
    `activate_crater` at index 15 despite `tile_crater == 16`. Tile 16 reads the
    first three opcode bytes following the table as an indirect callback.
14. Warm new-game setup calls `init_level`, which prints the score and performs
    the countdown, before resetting `player_score`. It also never resets
    `player_starting_score`, so the first death in a later session can restore
    a score from the previous session.
15. Sprite and tile spawners copy 42 bytes from defaults blobs that define only
    19 bytes (record offsets 22 through 40). Each spawn therefore reads 23
    bytes of adjacent variables or code into the record's reserved tail.
16. `turret_fireball_defaults` stores sprite-domain `enemy_ground == 2` in a
    tile record whose tile-domain type is `tile_turret_fireball == 5`.
17. `activate_crater` serves two incompatible contexts. Map activation calls it
    without allocating a tile and with an unrelated `tile_table_pointer`, while
    kill handlers intentionally use that pointer to transform an existing tile
    in place.
18. Tileset and level selectors trust writable indices and discover table ends
    by dereferencing sentinel records. An out-of-range selector can therefore
    address outside either lookup before the terminator test runs.
19. The active-tile allocator advances a ring index without checking whether
    that physical record is still alive. With churn below the byte-sized stack
    limit, it can overwrite a live record and publish the same physical slot
    more than once. Deactivation also leaves the physical alive/just-died flags
    set, preventing safe reuse.
20. Active-tile traversals use pointer contents and inactive records as implicit
    terminators. A malformed dense-stack slot can be dereferenced before it is
    validated, and one inactive record can silently truncate the remainder of
    the update.
21. `kill_electrode` saves its source tile pointer on the machine return stack,
    then has an early `ret` inside its scan. That path consumes the saved data
    as a return address. Its later source rediscovery is also unbounded and can
    walk beyond the active stack if compaction or corruption loses the source.
22. The lethal `update_shields` path enters `kill_player`, which overwrites the
    collision caller's AF, BC, IX, and IY before control returns to the same
    collision handler.
23. A seek-zag enemy that reaches the bottom edge can loop forever inside one
    frame. `move_sprite` clamps and bounces it, but the edge branch immediately
    selects another downward heading and moves again, repeating without ever
    returning to the scheduler.
24. `fonts_load` keeps its font-list cursor in IX across `mos_fread`, although
    that MOS call does not preserve IX. Separately, a failed `mos_fopen` returns
    handle zero; the old loader continued and eventually passed zero to
    `mos_fclose`, whose API meaning is close all files.
25. `vdu_vblank` and `multiPurposeDelay` poll the MOS clock without a bound. A
    stalled clock can trap the whole application permanently inside either
    helper.
26. Dormant `game_exit` tail-jumps to a no-op helper and returns with the stored
    state unchanged, so reaching it repeats the exit state forever instead of
    returning control to the application's shutdown path.
27. The first emulator qualification of checkpoint `73bc35f` reached the
    post-load keypress, rendered the gameplay starfield, and then appeared to
    freeze. The next initialization routine, `sprites_init`, loads `BC = 3`
    before a `DJNZ` player-frame loop. In ADL mode that leaves `B = 0`, so the
    three-frame loop executes 256 times and floods the selected VDP sprite's
    frame list. The source now loads `B = 3` explicitly. Visual testing of that
    correction still stalled after the starfield, so this was a definite defect
    but not the complete cause of the post-keypress hang. The next diagnostic
    build bypasses `start_level_countdown` from a tagged checkpoint.

The current `CALL_HL` macro was checked against the assembler listing. Its
synthetic return address points to the instruction after `jp (hl)`; the macro
itself is not an identified defect.

The keyboard-map branches were also checked against the official MOS keyboard
contract. MOS keyboard-map bits are one when pressed, so their NZ branches are
not inverted. The joystick UART bits are separately active-low as documented in
`player_input.inc`.

## Responsiveness hypotheses

The first level's mandatory six-second busy countdown begins after the startup
keypress and can look unresponsive. After correcting the wrapped finale count,
a player death still monopolizes 313 internal animation/VBlank iterations. Both
are strong explanations for apparent freezes at their respective points, but
neither has been correlated with a trace of the reported emulator run.

The old `vdu_vblank` and `multiPurposeDelay` implementations also contained
unbounded polling loops. No evidence shows that a stalled MOS clock caused the
observed run, but both loops are now bounded and latch a diagnostic bit before
failing open.

## Isolated stabilization repair

The stabilization work is intentionally confined to the persistent Git copy at
`/home/smith/Agon/mystuff/nurples-repair` on `dev`. The canonical Nurples tree
has not been edited, merged, or pushed.

Local repairs completed in the copy:

1. Allocation failures now stop before invalid IX use; the death finale has an
   explicit 41-update lifetime, and player initialization publishes success.
2. Every active MLT-derived pointer offset is zero-extended. Activation and
   lookup counts are derived from their tables, selector indices are clamped,
   and level transitions no longer dereference terminators.
3. Sprite and tile defaults copy exactly their 19 initialized bytes and clear
   retained motion state before a physical record is reused.
4. The active-tile allocator searches all 256 physical records, the dense stack
   remains capped at 255 entries, and deactivation clears lifecycle flags.
   Stack slots and physical records are range-, alignment-, and identity-checked
   before dereference; malformed slots are quarantined through bounded
   compaction.
5. Electrode removal is bounded and stack-balanced. Crater map spawning and
   in-place tile transformation now have separate entry points, and tile
   defaults use tile-domain types.
6. Global, player, enemy, sprite, and tile legacy indirect dispatches use exact
   stabilization whitelists. Corrupt entries fail closed and latch domain fault
   bits; callback traversal restores its IY cursor.
7. Level completion and game-over outcomes return to the global coordinator
   before the old playing frame advances enemies or the world. Shared table
   reset code removes duplicated lifecycle setup, and new-session score values
   are initialized before level presentation.
8. The seek-zag edge path performs at most one movement update per outer frame.
   The clock polls are bounded, file-open failure cannot read or close handle
   zero, font iteration preserves IX, image creation is skipped on load failure,
   and the dormant exit state requests shutdown through the main loop.

The file loader's success carry means that the file opened, reads reached a
zero-byte EOF, and the handle closed. It does not yet compare bytes read against
the expected size passed by image callers; that is an integrity-strengthening
opportunity, not a demonstrated cause of the current failure.

### Verification evidence

- `ez80asm` completed both passes and regenerated the tracked listing/binary.
  A second disposable assembly matched both artifacts byte-for-byte.
- The final binary is 41,251 bytes with SHA-256
  `5972820b56ceeaa5882180667b3355db34c1ed20cca043c4daac2421d497b166`.
  The final listing SHA-256 is
  `2d923f1ef5c1d55acea7ed740cac87189f51c9693537a46b93f3fcfe423f92f1`.
- All 35 unit tests pass. The suite includes source invariants, a disposable
  assembly, current font generation, and canonical `agonutils` adapter checks.
- `pip check`, the canonical native `agonutils` test, and `ruff check .` pass.
- Independent listing review confirms one tileset, 17 activation callbacks,
  19-byte definition copies, the 41-update explosion lifetime, sprite record
  counts 20/7, exact handler whitelists, record/alignment guards, both bounded
  clock loops, and the orderly exit handoff.
- No emulator run has been used to qualify gameplay behavior at this checkpoint.

## Architectural boundary not crossed

The remaining problems are coupled lifecycle-policy changes rather than local
safety repairs:

1. `player_dying` can call `tiles_plot` internally and latch level completion.
   The next outer frame consumes that global result before `player_dead`, so a
   death at the end of a level can initialize the next level and only then
   respawn or enter game over. Correcting this needs an explicit priority rule
   and central arbitration for simultaneous actor/world outcomes.
2. Player death, level initialization/countdown, game over, and the initial key
   wait still monopolize the scheduler. Converting them to bounded updates is
   the cooperative migration described by ASM-001, not a local branch repair.
3. Legacy `game_init` and `player_dead` still assign `enemies_state`, and
   `player_dead` still owns world reset/level initialization. Moving that
   responsibility requires the agreed actor-owned reconciliation boundary.
4. Exact pointer whitelists contain arbitrary-jump hazards but remain a bridge.
   Numeric IDs, fixed registries, legal-transition validation, and event
   arbitration remain the intended endpoint in `state_new.inc`.

No architectural migration has been started. Runtime emulator qualification is
recorded as ASM-007 and awaits Author review of this checkpoint.

## `state_new.inc` scaffold

The standalone scaffold establishes these rules:

1. Global and actor states use validated byte IDs and fixed 24-bit descriptor
   tables with mandatory ENTER, UPDATE, RENDER, and EXIT callbacks.
2. UPDATE callbacks request transitions; a central boundary performs
   EXIT -> state swap -> ENTER. Direct state-to-state jumps and current-state
   writes are forbidden.
3. The first request wins and transition edges are explicit. Invalid requests
   are rejected; corrupt current or commit-time IDs recover to a fault/safe
   state. Error bits remain latched for diagnosis.
4. Input and actor outcomes are posted as events. The coordinator processes a
   snapshot once per global tick with explicit priority.
5. Loading, new-game setup, level setup, countdown, level transition, respawn,
   and presentation sequences advance through bounded frame updates.
6. A published policy tells the scheduler whether actor/world update and render
   work is permitted in the current global phase.
7. Every generic actor tick invokes its actor-owned reconcile callback before
   any local update. Reconciliation may select a new local state; the dispatcher
   enters it and defers its first update until the next outer frame.
8. Actors post global events but cannot assign a global phase. The global
   coordinator cannot assign an actor-local state.
9. State age, total frame count, transition serial, last transition events, and
   latched errors provide basic emulator diagnostics.

A reference player controller demonstrates `INACTIVE`, `READY`, `ACTIVE`,
`DYING`, and `DEAD` local phases. It is intentionally inert and is not connected
to the current player sprite or legacy handlers.
