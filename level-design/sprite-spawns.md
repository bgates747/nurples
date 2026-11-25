# MAP-005 — Three authoring layers and independent sprite spawns

Give every campaign level, playable variant and starter map the same three
layers, bottom to top. A sprite spawn must not replace the terrain under it.
Owner: Nurples. Starting point: accepted MAP-004, dev `eddb916`. Scope
approved by Smith. Freeze before editing maps or runtime.

| Tiled layer | Responsibility |
| --- | --- |
| Background | Earth image or an empty layer for runtime starfields. No dummy bitmap upload. |
| Tiles | Passive scenery and fixed active tiles: e.g. turrets, electrodes and functional pads. |
| Spawns | Editor markers for independently moving sprites, regardless of whether they fly or travel on the ground. Markers paint no terrain. |

- MAP-005-001 [x] Inventory existing DG marker callbacks and artwork, distinguishing a pad which owns launch/reload/retreat behavior from a one-shot sprite constructor. Record expected visible art, actor counts and activation order before changing data.

- MAP-005-002 [x] Migrate all campaign TMXs, variants and template to the three named layers. Make existing pad visuals and behavior explicit where needed; a newly placed sprite marker must not implicitly draw a pad. Preserve visible pads in existing encounters and prevent duplicate launches.

- MAP-005-003 [x] Compile terrain and sprite activation independently. Reuse row-admission timing: dispatch markers once when their row enters; no layer decoding or new per-scanline decision in the drawing loop. Preserve separate background and tile draw toggles. Where fixed active tiles and sprites share a cell, preserve both events; reject unsupported combinations explicitly until qualified.

- MAP-005-004 [x] Test a DG sprite over a passive road, a fixed active tile with no marker, spawn and active tile in one cell, and existing Earth overlaps. Check art, activation counts, row order, indexed tile moves and map import. Verify older single-layer diagnostic inputs remain usable.

- MAP-005-005 [x] Build and stage a playable Earth-to-DG FAE review, including a recognizable DG overlap. Confirm independent drawing, spawning, pad behavior and level handoff. No new enemy AI or physical deployment.

Review/freeze before execution. No tank constructor, road tagging, navigation
or new collision behavior is part of this tranche.

### Implementation and review — 2026-11-25

Frozen on `dev` at `90a60f3`. All 17 tracked maps (including the starter,
staggered Earth, standalone qualification maps and DG design studies) exported
successfully with system Tiled on Oryx. Verified all three layer names/order
in those exports; the Oryx session files were not replaced.

The old DG seeker-pad ID 1 owns launch/reload/retreat. Its 11 modern DG and
20 recovered Z80 placements are now on Tiles only. DG IDs 2/3/15 were
one-shot constructors displaying art 121. The proving ground therefore keeps
its 109 sprite markers with explicit passive pad artwork underneath. A *new*
ID 1 on Spawns calls the independent seeker constructor; it does not install
a ground pad.

Production terrain and ordered callbacks were compared cell-by-cell with
the freeze: Earth 72 events, proving ground 110, modern DG 139, recovered
Z80 45 (callback requests, not a promise of free actor slots). No art or
encounter changed. The five linked binaries (ordinary, aligned control,
aligned bombing, staggered and review) were checked against compiled terrain
bytes and row streams. The existing map import and tile move/swap tests pass.
Full suite: 251 tests; same five baseline failures (four starfield hash
assertions and the bomb defaults assertion), two skips.

**Accepted by Smith after the Oryx FAE playthrough (005).** Rebuild with
`build/scripts/build_00_all_the_things.py --sprite-spawn-review` using the
project Python. This is the playable campaign with diagnostic drawing toggles
and invulnerability. Only in the audition, the first small ship on the DG
proving ground (zero-based column 8, fourth incoming row) has a horizontal
conduit underneath instead of pad art. Check Earth to DG, the conduit, and
ordinary pad behavior; 1 toggles background, 2 tiles. Oryx's existing
`map002-electrode-game-r01` card was backed up and the four guest files
read back; review executable SHA-256
`1d9bdd50326c989df14c27d8c2a4b6356980154dc9594a91ebbea20b859a6b30`.
