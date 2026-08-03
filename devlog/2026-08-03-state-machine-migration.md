# 2026-08-03 — Cooperative state-machine migration begins

## Mission and repository boundary

`nurples-repair` on `dev` is the authoritative game-repair workspace. Generic
execution, observation, input injection, and reporting remain owned by
`agon-headless-test`; Purple Nurples owns its telemetry, scenarios, assertions,
and eventual autonomous player. All work remains local until the Author
explicitly authorizes a push.

The secondary demonstration target is a RAM-driven agent completing a short
qualification map unattended and leaving a replayable trace. This is tracked
as `TEST-001`; it does not displace the `ASM-001` state-machine migration.

## First migration boundary: player death

The legacy `player_dying` handler was selected first because it violated the
new scheduler contract in a source-proven way: a single actor call advanced the
entire world and waited for hundreds of VBlanks before returning. That blocked
the global dispatcher, input arbitration, diagnostics, and any deterministic
frame-level observer.

The handler is now a bounded actor-owned sequence with explicit INITIAL,
DELAY, FINALE, and FINISH phases. Each call performs at most one explosion
allocation and returns. The existing global `game_playing` loop remains the
sole owner of enemy, explosion, tile, HUD, and VBlank cadence. `kill_player`
initializes all sequence state, and process initialization resets the backing
bytes so repeated MOS `RUN` invocations do not inherit a partial death.

The original broad timing is retained: sixteen initial effects at sixteen-frame
intervals, a sixteen-frame pause, eight finale attempts, then one explosion
lifetime before publishing `player_dead`. Finale allocations are intentionally
spread across frames, removing the old same-frame pool burst.

## Qualification still required

1. Assemble and inspect the changed symbols/listing.
2. Exercise deaths at multiple heights and verify passive and active tiles,
   enemies, projectiles, explosions, HUD, Escape handling, and VBlank continue
   once per outer frame.
3. Observe `player_dying_phase`, `player_dying_timer`, `player_dying_spawns`,
   dispatcher fault bytes, and the first active-tile record throughout death.
4. Confirm respawn and final-life game-over transitions before accepting this
   migration slice.

This is not yet a proven emulator change and must not be committed as accepted
runtime behavior before the Author's visual gate.

## Authoritative roadmap

`TODO.md` now decomposes `ASM-001` into stable stages `ASM-001A` through
`ASM-001M`. Each stage names a concrete migration boundary and its acceptance
evidence. The order deliberately establishes telemetry and qualifies the inert
coordinator before connecting it to the outer scheduler, then migrates global
and actor ownership incrementally. Integrated deterministic regression and the
Author's visual gate precede retirement of legacy paths; stylistic convergence
remains a separately reviewed final stage.

This devlog records why the roadmap was selected. The checkboxes in `TODO.md`
remain the sole authoritative statement of unfinished work.
