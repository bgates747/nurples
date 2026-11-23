# MAP-003 — Earth terrain tile picker

Give Smith a visual way to curate the entire Earth image for Tiled, without discarding one-off artwork or touching the campaign. The previous near-match-only sheet is a control, not the input.

Owner: Nurples. Smith approved the proposed picker and requested implementation. Work on dev; stage a separate review workspace on Oryx. Recovery: earth.png, the existing earth_tiled folder, sparse checkout edits and game assets remain untouched.

- MAP-003-001 [ ] Extract exact distinct padded 16×16 RGBA tiles in first-use order. Store stable source IDs, source layout and deterministic representative-based similarity groups, including singletons. Support different thresholds and report their tradeoffs.

- MAP-003-002 [ ] Tkinter: one horizontal row per group, 3× nearest-neighbor previews, one-click independent toggles, any number including zero, selected count and optional multi-member-only view. Keep saved choices separate from groups for the same source.

- MAP-003-003 [ ] Export selected IDs in source order to a separate 16-across PNG and index. Test coverage, grouping, selection persistence, export pixels and normal UI interactions. Stage for Smith on Oryx and stop for visual review; no automatic game integration.
