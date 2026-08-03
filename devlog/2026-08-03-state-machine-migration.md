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

## ASM-001A implementation slice

The first observable-baseline slice adds version 1 of the contiguous `PNST`
runtime telemetry record documented in `docs/runtime-telemetry.md`. It uses an
eight-bit seqlock, stable numeric state/event IDs, per-domain transition serials
and state ages, global event snapshots, player-death progress, level progress,
and component plus aggregate fault latches. A temporary bridge maps the three
legacy writable state pointers to numeric IDs; the record explicitly flags
that mode so a consumer cannot mistake it for completed migration.

ABI 1.0 is an append-only 80-byte record with major/minor version bytes and a
16-bit size. Producer latches remain outside the public range. Startup now
publishes BOOT, LOADING, TITLE, NEW_GAME, LEVEL_ENTER, then PLAYING; session,
level, and actor fields remain invalid with actors reported INACTIVE until the
PLAYING snapshot. Ordinary snapshots count only completed VBlank frames, while
startup and pre-VBlank terminal publications neither increment frame totals nor
age stable states. PLAYER_DEAD and RESPAWN have explicit lifecycle markers.
LOAD_COMPLETE marks the TITLE edge, while START marks initial NEW_GAME and a
key-confirmed GAME_OVER restart, keeping bridge traces causally comparable with
the proposed numeric coordinator.
The assembled sequence byte is odd, and `telemetry_init` is the first call in
`main`, so a cold image is not readable before BOOT is rebuilt. Repeated `RUN`
observers must wait for the new LOADING transition because resident RAM can
briefly retain the preceding invocation's terminal snapshot before entry runs.

The normal and map-suite builders now stage fresh assemblies with symbol export
and publish binary-matched `.bin`/`.symbols` pairs under `tgt/`. Baseline trace
capture remains outstanding, so `ASM-001A` is not yet complete.

The finalized static gate passed all 72 project tests, Ruff, `pip check`, the
canonical `agonutils` smoke test, and the current-asset determinism check. The
campaign telemetry range is `$0487E2..$048831`; the map-suite range is
`$048D32..$048D81`. Their binary SHA-256 values are respectively
`e56f725dbf5a7c32c9496560bce6a6ab925e66cfbd4cb6c4dfa69da435245ccd`
and `129f5552d975f1446562a901946ebfca0dbfbc5eb30d6e940fcbc07fc4d0012a`.

The current Headless AHT-004 direct runner cannot capture baseline traces: it
has no MOS, VDP, SD, keyboard, checkpoints, or RAM-read interface. Runtime
evidence therefore remains gated by AHT-005/AHT-006 for observation and schema,
then AHT-010/AHT-011 for the full-machine startup and deterministic input path.
The ABI and exact artifact identities were published to the accepted shared
mailbox in message `2026-08-03_15-29-26_nurples_telemetry-abi-1-0`.

Static review of the cooperative death cadence found that its FINISH timer
waited one quiet frame beyond the last possible explosion update. The named
`player_dying_finish_frames` interval now accounts for the spawn frame's update,
so the final explosion deactivates later in the same outer frame that publishes
the player DEAD state. The review otherwise found the allocation-failure and
register-preservation paths sound. It reaffirmed that simultaneous level
completion can suspend a DYING actor and must be settled by the explicit event
priority work in `ASM-001E`.

## MOS display-state handoff

Exit previously restored only the screen mode, then printed the farewell using
whatever text colours survived the preceding VDP operations. Process entry now
queries logical text foreground and background through `VDU 23,0,&94,128/129`,
using a bounded MOS-result poll and an invocation-scoped VDU fault latch. Exit
restores the original mode, explicitly selects the permanent system font,
clears a black text screen, enables the cursor, prints the farewell in white,
then restores each successfully captured logical colour for subsequent MOS
text. A query timeout or invalid pre-2.7-style response fails closed to the
white-on-black defaults instead of hanging startup.

The lingering violet pixels were traced to VDP mode-switch ordering: the mode
operation clears using the old violet context before it resets that context,
and a single-buffered destination is not then cleared again. Exit therefore
selects the text cursor and system font, sets black, and issues its own CLS only
after the caller's mode has been restored.

Exact arbitrary custom-font restoration is not implementable through the
documented public interface. The selected font's buffer ID is not queryable;
screen-mode changes destroy every saved VDP context; and Nurples intentionally
clears all VDP buffers on each launch, which also destroys any caller-owned
custom font data. Other inspected Agon applications restore a mode and explicit
defaults rather than recovering a caller font. `ASM-009` tracks emulator
qualification of the achievable colour and cursor handoff.

The Author playtested the committed build in the durable emulator profile on
2026-08-03 and reported correct behavior with no crashes. This closes the
`ASM-009` visual acceptance gate; exact arbitrary custom-font restoration
remains outside the documented VDP interface as described above.
