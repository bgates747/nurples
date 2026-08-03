# Purple Nurples TODO

This is the project's only authoritative unfinished-work list. Evidence and
decisions are recorded in the dated development log.

- [ ] **ASM-001 — Cooperative state-machine migration:** review and integrate
  `src/asm/state_new.inc` incrementally. The global coordinator must publish
  phase and policy without assigning actor-local states; every actor-owned mini
  state machine must reconcile itself against the global phase before its
  local handler runs. Replace blocking lifecycle handlers with bounded frame
  updates and qualify each migrated phase in the emulator. Define and test an
  explicit priority for simultaneous player-death and level-complete events;
  the legacy death animation can currently publish both across different
  scheduler boundaries. Preserve legacy dispatch and behavior unless a
  specific site causes a demonstrated unsafe
  state, unintended branch, or is the reviewed boundary being migrated. After
  the new architecture is proven working, conduct a separately reviewed
  stylistic-consistency pass across legacy and migrated dispatch idioms.
- [x] **ASM-002 — Player-death memory corruption:** repair and regression-test
  the position-dependent but readily reproducible explosion-pool exhaustion
  path in `player_dying`, which writes through `IX == table_end` into the first
  active-tile record. Replace the unreloaded 256-iteration finale counter and
  make every failed spawn path return without dereferencing its failed
  allocation pointer.
- [x] **ASM-003 — ADL lookup-index safety:** zero-extend every `MLT` result
  used in 24-bit pointer arithmetic for level, tile-table, activation-table,
  and trigonometric lookups; verify the resulting addresses in the assembler
  listing.
- [x] **ASM-004 — Active-tile dispatch bounds:** align the activation callback
  table with tile IDs 0 through 16 and derive its dispatch bound from the
  table size so tile 16 cannot interpret following opcodes as a callback.
- [x] **ASM-005 — Definition-copy bounds:** copy only the 19 initialized bytes
  in sprite/tile defaults instead of reading 23 bytes past each blob into
  adjacent data or code.
- [x] **ASM-006 — Tile activation domains:** use tile-domain type IDs and split
  map crater spawning from in-place active-tile transformation so neither path
  consumes a stale context pointer.
- [ ] **ASM-007 — Stabilization runtime qualification:** after review, run the
  isolated repair build in the Nurples emulator profile and exercise startup,
  repeated seeker edge contact, level completion, player death at multiple
  heights, respawn, and game over. Inspect all new latched fault bytes after
  the run. Do not merge or push the repair copy before the Author accepts it.
- [x] **PY-001 — Current asset build orchestrator:** audit and rebuild
  `build_00_all_the_things.py` only for assets consumed by the current Purple
  Nurples game. Derive the required stages from the current source and
  deployable asset references. Do not revive dormant experiments, alternate
  asset pipelines, demo applications, or historical Wolf/MapMaker
  stages unless the Author promotes them separately later. Canonical sprite
  and tile output is visually accepted, the sole referenced runtime font
  (`Lat38-VGA8_8x8.font`) is vendored, and the failed music experiment is
  intentionally excluded.
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
