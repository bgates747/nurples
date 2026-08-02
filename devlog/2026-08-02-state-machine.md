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
5. The `player_dying` finale enters its animation loop with B already zero from
   the preceding `djnz`. The next `djnz` wraps and executes 256 iterations. The
   full death handler performs 528 internal animation/VBlank iterations rather
   than one bounded update per outer frame.
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

The current `CALL_HL` macro was checked against the assembler listing. Its
synthetic return address points to the instruction after `jp (hl)`; the macro
itself is not an identified defect.

The keyboard-map branches were also checked against the official MOS keyboard
contract. MOS keyboard-map bits are one when pressed, so their NZ branches are
not inverted. The joystick UART bits are separately active-low as documented in
`player_input.inc`.

## Responsiveness hypotheses

The first level's mandatory six-second busy countdown begins after the startup
keypress and can look unresponsive. A later player death monopolizes roughly
528 VBlanks, including the erroneous 256-iteration finale. Both are strong
explanations for apparent freezes at their respective points, but neither has
been correlated with a trace of the reported emulator run.

`vdu_vblank` and `multiPurposeDelay` also contain unbounded polling loops. If
the MOS time source stops advancing, either can become a true hang. No evidence
currently shows that this happened in the observed run.

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
