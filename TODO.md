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

  Ordered migration stages and acceptance gates:

  - [ ] **ASM-001A — Observable baseline:** publish a stable, documented RAM
    telemetry block for global phase, actor states, pending events, transition
    serials, state ages, and latched faults. Produce binary-matched symbols and
    record baseline traces for startup, ordinary play, level completion,
    player death, respawn, and game over. Acceptance: the same build yields
    repeatable symbol addresses and each trace identifies its terminal outcome
    without interpreting writable code pointers.
  - [x] **ASM-001B — Coordinator core qualification:** extract the generic
    numeric-ID dispatcher, legal-edge validation, event arbitration, policy
    publication, and actor-controller primitives from `state_new.inc` into a
    buildable fixture with inert callbacks. Acceptance: deterministic tests
    cover every legal edge, invalid current/target recovery, first-request
    conflict behavior, reentrancy rejection, event snapshotting, and actor
    reconciliation before ENTER or UPDATE.
  - [ ] **ASM-001C — Single-owner outer scheduler:** make the main loop the
    only owner of input snapshotting, one coordinator tick, actor/world cadence,
    rendering, one VBlank, and orderly exit. No state or actor callback may
    wait for VBlank or advance another subsystem. Acceptance: instrumentation
    records exactly one scheduler frame and at most one committed global
    transition per iteration through both ordinary play and a state change.
  - [ ] **ASM-001D — Bounded startup lifecycle:** migrate BOOT, LOADING, TITLE,
    NEW_GAME, LEVEL_ENTER, and COUNTDOWN to numeric phases whose callbacks
    perform bounded work and return. Asset loading may advance in explicit
    chunks while preserving breadcrumb feedback. Acceptance: startup reaches
    PLAYING, Escape remains observable between steps, the countdown advances
    by frame age, and two consecutive MOS `RUN` cycles start from identical
    initialized state.
  - [ ] **ASM-001E — Playing coordinator and event priority:** migrate PLAYING
    without allowing the coordinator to write actor-local states. Define the
    priority and consumption rules for simultaneous QUIT, fault, player-death,
    level-complete, pause, and victory events. Acceptance: table-driven tests
    exercise every single event and relevant simultaneous pair, including the
    player-death/level-complete collision, with exactly one documented result
    and no stale event on the following frame.
  - [ ] **ASM-001F — Cooperative player-death sequence:** qualify the bounded
    INITIAL, DELAY, FINALE, and FINISH implementation introduced at `566acc0`.
    Acceptance: deaths at multiple heights keep passive and active tiles,
    enemies, projectiles, explosions, HUD, Escape polling, and VBlank advancing
    once per outer frame; pool exhaustion does not alter the first tile record;
    respawn and final-life game over both complete with no fault latch.
  - [ ] **ASM-001G — Player-owned controller:** migrate INACTIVE, READY,
    ACTIVE, DYING, and DEAD to validated local IDs. The player's reconciliation
    callback must read global phase/policy before every local handler and own
    all player-state changes; player events may request later global action.
    Acceptance: corrupt local IDs recover safely, ENTER/EXIT occur once per
    transition, global transitions never directly assign player state, and all
    player lifecycle scenarios preserve accepted behavior.
  - [ ] **ASM-001H — Global transition lifecycles:** migrate PAUSED,
    LEVEL_TRANSITION, RESPAWN, GAME_OVER, VICTORY, EXIT, and FAULT, incorporating
    the level-scoped reset from `ASM-008`. Acceptance: every phase is bounded,
    every legal incoming and outgoing edge is exercised, illegal edges fail
    closed, and transitions never resume work from the obsolete phase.
  - [ ] **ASM-001I — Enemy-owned controller:** replace the enemy-domain raw
    state pointer with validated local IDs and actor-owned reconciliation.
    Preserve individual seeker and fireball strategy callbacks unless a
    demonstrated defect requires migration. Acceptance: enemies reconcile
    correctly across PLAYING, player death, level transition, respawn, game
    over, and fault; invalid state cannot become an indirect jump.
  - [ ] **ASM-001J — Sprite and active-tile strategy boundary:** validate
    strategy identity and ownership at the remaining per-record indirect
    dispatch boundaries without mechanically rewriting proven callbacks.
    Acceptance: every current sprite and active-tile type dispatches only to a
    permitted callback, invalid type/program data fails closed with a latched
    diagnostic, and qualification maps exercise each implemented strategy.
  - [ ] **ASM-001K — Integrated deterministic regression:** run the complete
    standalone map suite from fresh machine state with scripted input and RAM
    assertions. Acceptance: repeated runs produce the same ordered phase/event
    trace; startup, pause/resume, level completion, death/respawn, game over,
    restart, and injected corrupt-state recovery meet their oracles with no
    unexpected fault bits or timeout.
  - [ ] **ASM-001L — Human acceptance and legacy retirement:** perform the
    Author's emulator play-through, compare visual/audio behavior, and remove
    only legacy state machinery made unreachable by the proven replacement.
    Acceptance: the Author approves runtime behavior, no obsolete writable
    state pointers remain on reachable paths, cold-start documentation matches
    the final architecture, and a local fallback commit precedes cleanup.
  - [ ] **ASM-001M — Separately reviewed idiom pass:** only after `ASM-001L`,
    review legacy and migrated dispatch, naming, register contracts, and table
    idioms for consistency. Acceptance: the proposed style changes are reviewed
    independently, assemble cleanly, and pass the full `ASM-001K` regression
    without changing accepted gameplay.
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
- [ ] **ASM-008 — Level-transition world reset:** define and implement a
  level-scoped reset that removes live enemies, projectiles, explosions, and
  active tiles at a natural level boundary without erasing the player record or
  player-owned state. Qualify a transition with each actor type still live.
- [x] **ASM-009 — MOS display-state handoff:** qualify bounded capture and
  restoration of the caller's text foreground/background colours around a
  system-font, white-on-black farewell in the restored screen mode. Acceptance:
  normal exit, Escape, asset-load failure, and repeated MOS `RUN` all return
  with the cursor visible and subsequent MOS text using the captured logical
  colours, or deterministic defaults after a timed-out/invalid VDP response.
  Exact restoration of an arbitrary caller-selected custom font is unavailable:
  the public VDP query surface does not report its buffer ID, MODE destroys
  saved contexts, and this application deliberately clears VDP buffers.
- [ ] **ASM-010 — Launcher zero-delay underflow:** normalize or explicitly
  handle an initial randomized delay of zero in small seeker pads and fireball
  turrets. Both handlers currently decrement before testing, so a nominal
  immediate activation wraps to 255 updates and makes spawn timing
  unexpectedly nondeterministic. Qualify initial and subsequent spawn cadence
  without changing the established actor-pool limits.
- [ ] **TEST-001 — Autonomous qualification player:** after deterministic
  full-machine input is available, implement a project-owned RAM-driven agent
  that can complete the short qualification maps, emit a replayable input and
  state trace, and distinguish completion, death, stall, and runtime faults.
  Framebuffer-only play is a separate later experiment, not a prerequisite.
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
