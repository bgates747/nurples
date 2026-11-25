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

- MAP-005-001 [ ] Inventory existing DG marker callbacks and artwork, distinguishing a pad which owns launch/reload/retreat behavior from a one-shot sprite constructor. Record expected visible art, actor counts and activation order before changing data.

- MAP-005-002 [ ] Migrate all campaign TMXs, variants and template to the three named layers. Make existing pad visuals and behavior explicit where needed; a newly placed sprite marker must not implicitly draw a pad. Preserve visible pads in existing encounters and prevent duplicate launches.

- MAP-005-003 [ ] Compile terrain and sprite activation independently. Reuse row-admission timing: dispatch markers once when their row enters; no layer decoding or new per-scanline decision in the drawing loop. Preserve separate background and tile draw toggles. Where fixed active tiles and sprites share a cell, preserve both events; reject unsupported combinations explicitly until qualified.

- MAP-005-004 [ ] Test a DG sprite over a passive road, a fixed active tile with no marker, spawn and active tile in one cell, and existing Earth overlaps. Check art, activation counts, row order, indexed tile moves and map import. Verify older single-layer diagnostic inputs remain usable.

- MAP-005-005 [ ] Build and stage a playable Earth-to-DG FAE review, including a recognizable DG overlap. Confirm independent drawing, spawning, pad behavior and level handoff. No new enemy AI or physical deployment.

Review/freeze before execution. No tank constructor, road tagging, navigation
or new collision behavior is part of this tranche.
