# Purple Nurples TODO

This is the project's only authoritative unfinished-work list. Evidence and
decisions are recorded in the dated development log.

- [ ] **PY-001 — Current asset build orchestrator:** audit and rebuild
  `build_00_all_the_things.py` only for assets consumed by the current Purple
  Nurples game. Derive the required stages from the current source and
  deployable asset references. Do not revive dormant experiments, alternate
  asset pipelines, demo applications, or historical Wolf/MapMaker
  stages unless the Author promotes them separately later. Canonical sprite
  and tile output is visually accepted, runtime fonts are now vendored, and
  the failed music experiment is intentionally excluded.
- [ ] **PY-002 — Safe standalone tools:** put the remaining import-time
  generators and broad edit/cleanup scripts behind explicit, project-scoped,
  dry-run-capable CLIs.
- [ ] **PY-003 — MapMaker conversion:** repair zero/full-bank padding, hex CSV
  parsing, sample bounds, header dimensions, and duplicated implementations in
  `tile_png_to_mapmaker.py`.
- [ ] **PY-004 — Tile generators:** reconcile XML schemas, transparent index
  gaps, buffer allocation, source names, and byte-width overflow in the CIV,
  DG, and shared tile scripts.
- [ ] **PY-005 — DG cross-reference:** update `tiles/dg/xref_map.py` for the
  current XML/TMX layout and test one-based tile mapping.
- [ ] **PY-006 — Build/deploy launcher:** split assembly, validated hardware
  deployment, and canonical emulator launch in
  `build_99_asm_assemble.py`; require explicit actions.
- [ ] **PY-007 — Utility correctness:** fix the stationary-velocity collision
  calculation, deterministic fireball generation, and current input paths
  before using those tools.
- [ ] **PY-008 — Script consolidation:** make navball a configuration-only
  caller of the common image builder and consolidate exact/near duplicate tile
  and comparison scripts.
- [ ] **PY-009 — Legacy fonts:** either qualify the preserved scripts in
  `tools/legacy_fonts/` with current paths and safe deletion or remove them
  after the Author confirms they are no longer useful.
