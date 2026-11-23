# MAP-003 — Earth terrain tile picker

Give Smith a visual way to curate the entire Earth image for Tiled, without discarding one-off artwork or touching the campaign. The previous near-match-only sheet is a control, not the input.

Owner: Nurples. Smith approved the proposed picker and requested implementation. Work on dev; stage a separate review workspace on Oryx. Recovery: earth.png, the existing earth_tiled folder, sparse checkout edits and game assets remain untouched.

- MAP-003-001 [x] Extract exact distinct padded 16×16 RGBA tiles in first-use order. Store stable source IDs, source layout and deterministic representative-based similarity groups, including singletons. Support different thresholds and report their tradeoffs.

- MAP-003-002 [x] Tkinter: one horizontal row per group, 3× nearest-neighbor previews, one-click independent toggles, any number including zero, selected count and optional multi-member-only view. Keep saved choices separate from groups for the same source.

- MAP-003-003 [x] Export selected IDs in source order to a separate 16-across PNG and index. Test coverage, grouping, selection persistence, export pixels and normal UI interactions. Staged; awaiting Smith's visual review. No automatic game integration.

## Implementation and review

Frozen contract: `005d1adea8dc0376e139234cb03673fd08970814`. Tool: `tools/terrain_picker.py`. Earth source file SHA-256: `b0c8baca6a84e9957b9e69251a06f96a438f2384932fad5a8b205553a6344462`. The 4,000 placements (last two scanlines transparently padded) cover 3,636 distinct tiles. Oryx reconstruction from atlas and layout matched every source pixel. Distances use RMS over premultiplied RGBA in byte units, with no translation, rotation or flipping. Groups compare directly to their first tile, never transitively. RMS 8, 20 and 32 gave 417, 206 and 83 groups respectively (~1 second each on the development host). Each folder has a default-representative reconstruction and a magnified worst pair for inspection.

On Oryx:

```bash
cd /home/smith/Agon/mystuff/nurples-earth-picker
.venv/bin/python terrain_picker.py review rms-20/groups.json --choices earth-picks.json
```

Leave the choices path unchanged to carry exact picks across thresholds; omit it for a separate per-threshold session. Click toggles autosave. Export asks for a **new folder name** and writes only selected tiles and their ID index, not a remapped game level. No live UI or hardware lease needed. Pillow, NumPy and Tk are required (Tk for review only); staged Oryx environment is isolated.

Two regression tests passed under Xvfb, including zero, one and multiple selections, non-transitive grouping, filtering, persistence, export ordering/pixels and overwrite refusal. An independent Oryx virtual-display smoke used the real metadata and a disposable choices file, clicked, filtered, scrolled, reopened and exported. No user selections were created.
