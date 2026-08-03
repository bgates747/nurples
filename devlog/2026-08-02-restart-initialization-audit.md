# 2026-08-02 — Restart initialization and VDP ownership audit

## Status

The implementation is complete in the persistent `nurples-repair` repository
on `dev`. Automated verification passes. On 2026-08-03 the Author visually
qualified the restart cycle and authorized this commit. No push or integration
into `main` is authorized.

## Reproduction and root cause

An isolated headless emulator reproduced the failure without reloading the
binary. The first `RUN` loaded and played normally, Escape returned to MOS, and
the second `RUN` re-entered the resident program image. MOS did not restore the
assembled `db` and `dl` initializers. The stale `game_assets_loaded` byte was
still nonzero, so the second invocation skipped screen-mode, font, UI, bitmap,
and cockpit initialization while the VDP was in the restored MOS mode. The
result looked non-playable even though gameplay setup continued to emit VDU
commands. Trace evidence is retained under
`/tmp/nurples-second-run.hXBCna/`.

The Author rejected application RAM as an authority for VDP residency. AGNB
containers have removed the original loose-file performance reason for the
cache, so `game_assets_loaded` and its alternate path have been removed from
active assembly entirely.

## Implemented lifetime ownership

1. True process entry calls `runtime_process_init`, then captures the caller's
   screen mode exactly once. An in-process GAME OVER restart cannot overwrite
   the saved MOS mode with gameplay mode 20.
2. Every `game_init` selects the permanent system font, resets VDP sprites,
   clears all VDP buffers, reloads the loose custom font, reloads both AGNB
   containers, rebuilds sprites and UI, and initializes a new game session.
3. Normal exit selects the system font before deleting the custom-font buffer,
   resets sprites before deleting their bitmap buffers, clears every VDP
   buffer, and restores the mode captured at process entry.
4. Fresh-RUN process state now explicitly resets the exit and player-event
   bytes; all seven game, enemy, player, sprite, tile, timer, and VDU timing
   fault latches; all three dispatcher pointers; seeker and sprite-edge helper
   state; deterministic random seeds; and the timestamp baseline.
5. Session initialization owns score, starting score, ships, background
   selection, sprite and active-tile tables, tileset, level, and player setup.
6. `player_init` now owns laser-charge and fuel-discharge timers. They no
   longer depend on cockpit drawing having happened earlier in the process.
7. Delay timing now captures its clock phase on every call, and the global
   timestamp starts from the current MOS clock instead of stale resident RAM.

## Writable-memory audit disposition

1. Sprite records, active-tile records, their pointer stack, allocation
   counters, and physical record IDs are cleared and rebuilt by
   `reset_runtime_tables`, `table_init`, `sprites_init`, and
   `tile_table_init` before gameplay consumes them.
2. Level and scrolling fields are established by `init_tileset`, `init_level`,
   and the explicit new-session background selection before use.
3. Player resources, repaint caches, firing timer, and actor state are
   established by `player_init`. The shield, laser, and fuel bar values are
   24-bit fields; the audit found that laser and fuel initialization wrote only
   their low byte. Both now zero-extend the byte maxima and write all three
   bytes, matching the repaired shield path.
4. AGNB reader state, progress state, callbacks, sizes, and last-error state are
   initialized by each loader operation. `agnb_file_is_open` is deliberately
   not blindly zeroed at process entry because it owns close/recovery semantics;
   successful and handled-failure paths already close it.
5. Font-file scratch and the fixed SRAM staging area are overwritten before
   use. Arithmetic, targeting, collision, debug, movement, and self-modifying
   VDU command-packet scratch are likewise write-before-read within their
   owning routines.
6. The active VDU resource universe is external state. Buffered blocks,
   bitmaps, fonts, samples, matrices, character mappings, sprites, origins,
   viewports, colors, cursors, and screen mode are never inferred from eZ80
   flags on game initialization; the required resources and contexts are
   reconstructed explicitly.
7. Dormant standalone tests and legacy PRT/experimental code are outside the
   active `nurples.asm` call graph and were not revived during stabilization.

No additional active 24-bit-register write into an unbuffered 16-bit field was
found. Existing VDU wrappers that patch 16-bit command fields either use
16-bit stores or include a padding byte that is overwritten before emission.

## Separate level-transition defect

`game_level_transition -> next_level -> init_level` does not clear live enemy,
projectile, explosion, or active-tile records. Old-world actors can therefore
cross a natural level boundary. Calling the full new-session reset there would
also erase the player, so this requires a deliberately scoped world reset
rather than a blind reuse of `reset_runtime_tables`. The work is promoted as
`ASM-008` in `TODO.md` and is not folded into the restart fix.

## Automated verification

1. The integrated `--all` build regenerated both AGNB containers, verified
   every payload, and assembled `tgt/nurples.bin` successfully.
2. All 52 unit tests pass, including new restart, VDP teardown ordering,
   process-state, timer, and 24-bit bar-initialization regressions.
3. Ruff, `pip check`, canonical `agonutils` verification, and the independent
   deterministic asset check pass.
4. A headless same-session regression passed against the exact rebuilt binary
   (`SHA-256 52088256aa47cf4387a09e488d2de14517ecbb2e216bf009279694d646d30d21`).
   The first run entered gameplay, Escape returned to MOS, a typed `RUN`
   performed a second system-font selection, sprite reset, clear-all, font/UI/
   game reload, 350-of-350 progress sequence, mode-20 setup, bezel/cockpit
   rebuild, sprite activation, and continuing gameplay. The first and second
   full-load command ranges were byte-identical. Evidence is under
   `/tmp/nurples-second-run-fixed.z0A5tL/`.
5. On 2026-08-03 the Author visually confirmed that the restart cycle works.
   Transient video-mode changes remain visible at several points, but the game
   consistently reaches the required final mode. This is retained as an
   observation for later VDP-context cleanup, not a failure of this repair.
