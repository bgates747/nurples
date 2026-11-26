# Purple Nurples TODO

- [x] **MAP-005 — [Three authoring layers and independent sprite spawns](level-design/sprite-spawns.md).** Accepted after Smith's Oryx FAE playthrough.

- [ ] **[SPR-HW-001 — Explicit hardware sprites for gameplay](docs/tasks/SPR-HW-001/TASK.md).**
  Next proposed experiment: [mouse-driven scanline stress fixture](docs/tasks/SPR-HW-001/scanline-lab.md).

- [ ] **[TURRET-CLEAR-001 — Loaded versus cleared to fire](docs/tasks/TURRET-CLEAR-001/TASK.md).** FAE review.

- [x] **TILE-POSE-001 — [Unit-defined entry facing](docs/tasks/TILE-POSE-001/TASK.md).** Accepted on FAE.

- [ ] **EARTH-001 — [Earth City opening level](docs/tasks/EARTH-001/TASK.md).**
  Current audition: [just-in-time Earth background scanlines](docs/tasks/EARTH-001/jit-background.md) (single-row FAE build).
  Current experiment: [stock pose batching, then async, Golem buffered poses and native VDPlets](docs/tasks/EARTH-001/TASK.md#authorized-execution-order).

This is the project's only authoritative unfinished-work list. Evidence and
decisions are recorded in the dated development log.

- [ ] **MAP-003 — [Earth terrain tile picker](docs/tasks/MAP-003/TASK.md).**

- [ ] **MUS-001 — Campaign music and the 50 Hz tracker clock.**
  Console8 accepted 50 Hz music during uploads. The
  [full-game clock](docs/tasks/MUS-001/TASK.md#r05--one-50-hz-music-clock-throughout-a-run)
  passed automated FAE timing and phase checks and the Console8 listening
  check (one slight glitch at the splash-to-game transition).
  [R06 turret launch and shield pre-alerts](docs/tasks/MUS-001/TASK.md#r06--turret-launch-and-shield-pre-alerts-in-the-50-hz-game)
  passed sequencing checks. The [longer klaxon and louder speech passed
  Smith’s FAE and Console8 listening tests](docs/tasks/MUS-001/klaxon-r07.md).
  Later proposal: [queue PT3 ticks ahead and let the VDP keep time](docs/tasks/MUS-001/TASK.md#proposed-vdp-timed-music).

- [ ] **AIR-001 — Side-to-side movement for three new air spawns.**
  [Movement and constructor contract](docs/tasks/AIR-001/TASK.md). Launch
  parameters and live pad integration await design review.

- [ ] **MAP-002 — Move or swap DG tiles by Tiled local ID.**
  [Contract, tool and qualification](docs/tasks/MAP-002/TASK.md).

- [x] **MAP-001 — Tiled level-design workspace on Oryx (ready).**
  [Task and setup checks](docs/tasks/MAP-001-ORYX/TASK.md).

- [ ] **GAME-001-EXIT — Console8 fails to return to MOS:** not reproduced
  with the current VDPlet or stock VDP. Keep the ordinary exit path;
  collect firmware and reset evidence if it recurs. VDPL-059 is plan-only.
  [Hardware exit task](docs/tasks/GAME-001-EXIT/TASK.md).

- [ ] **GAME-001JK-OLIMEX — Verify overlay restoration on production Olimex.**
  [Task and preparation evidence](docs/tasks/GAME-001JK-OLIMEX/TASK.md).

- [ ] **GAME-001JK-INTEGRATED — Review Game Over in the full game.**
  [Draft task](docs/tasks/GAME-001JK-INTEGRATED/TASK.md).

- [ ] **GAME-001 — Complete-game presentation and flow:** turn the existing
  playable systems into a cohesive finished-game experience without regressing
  the accepted splash/loading/start transition. Maintain the running design
  notes in `devlog/2026-11-09-game-completion.md`, implement each sequence as a
  bounded addition to the cooperative state architecture, and obtain visual
  acceptance for each presentation slice before treating it as complete.

  - [ ] **GAME-001A — Player death through restart:** stop background travel
    when death begins while the damaged ship continues forward under an
    erratic but controlled path. Preserve the last player-commanded
    left/centre/right ship frame, then make the dying ship wiggle around that
    frame. Preserve the existing explosion trail, which already follows behind
    the damaged ship satisfactorily, and preserve the accepted terminal
    explosion rather than redesigning either effect. Explosion particles that
    reach any playfield boundary must deactivate instead of inheriting the
    generic sprite mover's bounce. Reserve a top-of-field death runway that
    ordinary player input cannot enter but the dying controller may use. Make
    the path screen-position-sensitive: reaching either side must reverse the ship
    toward the middle and sustain a broad zig-zag rather than jittering at the
    boundary. Acceptance: the normal-play ceiling always leaves enough visible
    forward travel for the complete death choreography; the background remains
    spatially fixed; the ship remains visibly in motion and retains its
    commanded pose; side-edge encounters produce an unmistakable inward sweep;
    the accepted trailing and terminal explosion effects remain visually
    intact; no explosion reverses direction at the top, bottom, left or right
    boundary; and the complete sequence reaches respawn or game over without
    blocking the outer scheduler or inheriting stale death state.

  - [ ] **GAME-001B — Enemy death-phase behavior:** give the enemy controller
    explicit PLAYER_DYING and PLAYER_DEAD policies. On death entry, seekers
    stop attacking and begin retreat immediately, with no holding orbits.
    Hold pad launch timers through death, respawn and grace; surviving
    retreaters finish their exit even when control resumes. Preserve
    `move_enemy_seeker_orbit` for other phases that need a holding behavior.
    Each seeker
    must retain a validated identity for the launch pad that spawned it rather
    than searching for an interchangeable nearest pad. When told to retreat,
    it returns to that active, visible `tile_pad_small`; if its own pad has
    already scrolled off, been destroyed or been reused, it heads for the nearer
    left or right playfield edge and disappears before crossing it. GAME_OVER
    shares the retreat flight and docking logic with its own destination and
    immediate-entry policy under GAME-001J.
    Acceptance: no seeker continues player-targeting or collision damage after
    death begins; the first death update does not spend a frame in orbit;
    named per-seeker fields in the existing record hold its orbit and validated
    pad association without a parallel side table; pad identity cannot alias a
    recycled active-tile record; destroyed, inactive, reused or off-screen pads
    are never selected; docking or reaching the playfield edge releases the
    seeker record without drawing over the bezel or cockpit;
    and respawn starts without stale orbit,
    pad-association or retreat state.

  - [ ] **GAME-001B-EDGE — Seeker sprites escape the playfield:** use the
    entire rendered sprite footprint at the next position as the visibility
    boundary, not the physical screen or the sprite's origin alone. Hide the
    sprite before it would touch bezel/cockpit artwork; edge-retreat seekers
    must also release their actor slot. This replaces the earlier
    fully-off-screen rule. See the
    [bug report and acceptance cases](docs/tasks/GAME-001JK-INTEGRATED/TASK.md#seeker-boundary-bug-report).

  - [ ] **GAME-001C — Enemy movement variety:** retain
    `move_enemy_seeker_orbit` as a reusable behavior for game phases or enemy
    classes that need a holding pattern. Add enemy types whose movement is more
    predictable and easier to intercept than the current seeker, while keeping
    seekers as the dangerous evasive pressure enemy. Study the retained
    Xevious maps/art experiments and the lawful reverse-engineered technical
    references identified in `devlog/2026-11-09-game-completion.md`; extract
    movement principles rather than copying unlicensed code or shipped art.
    Acceptance: each new type has a visually distinct, learnable path; offers
    recurring clear firing windows; owns its lifecycle and player-death
    behavior; and can be mixed with seekers without every encounter collapsing
    into continuous pursuit.

  - [ ] **GAME-001D — Concealed Xevious mode:** design a playable Xevious-style
    port within the Nurples executable, either as a normal campaign level or a
    deliberately hidden Easter egg. Reuse the Nurples engine and clean-room
    behavioral understanding rather than importing unlicensed ROM code; settle
    the status of retained original-looking art before release. Acceptance:
    the selected entry path is intentional and replayable; the mode has its own
    coherent player, weapon, enemy-director and completion rules; entry and
    exit cannot contaminate normal campaign state; and the result feels like a
    compact playable game rather than a tileset demonstration.

  - [ ] **GAME-001E — Player entrance and safe respawn:** replace the immediate
    lower-left appearance with an actor-owned `player_spawning` phase. At level
    start and after a mid-level death, use the same entrance animation to bring
    the ship visibly into the horizontal centre and approximately the lower
    third of the playfield. Mid-level death must preserve level/map progress
    rather than restarting the level: before beginning the shared animation,
    resolve its preferred endpoint to the nearest placement that is neither on
    top of nor immediately behind a ground collidable. Keep the player
    noncollidable during entry, then transition through a distinct
    `player_grace` state that visibly blinks the ship after control begins.
    The entrance is a banked rising swoop with eased acceleration and settling;
    grace lasts one real-time second. Losing a ship retains the score earned so
    far, matching the decision to retain level progress.
    Acceptance: life reset, sprite preparation, entrance motion, collision
    activation and ordinary play are distinct bounded steps; the ship never
    pops into existence at the lower-left corner; initial level entry reads as
    deliberate choreography; both contexts visibly use the same motion;
    mid-level respawn cannot end inside a ground hazard or with inadequate
    forward clearance; no death rewinds the level to its beginning; control
    begins only after the entrance has visibly settled; collision remains
    disabled throughout `player_grace`; and timer expiry explicitly transitions
    the actor to `player_alive`. A mid-level death must not restore
    `player_score` from `player_starting_score`. The dedicated off-centre
    placement check is closed unverified: grace appears effective in play,
    and no further placement test is required.

  - [ ] **GAME-001E-DEATH-KEY — On-demand death fixture for safe respawn.**
    [Task and FAE acceptance cases](docs/tasks/GAME-001E-DEATH-KEY/TASK.md).

  - [ ] **GAME-001E-START-WARP — Opening starfield speed profile:** start at
    normal speed, accelerate to fast scroll, then slow down before admitting
    terrain. [Task and timing decision](docs/tasks/GAME-001E-START-WARP/TASK.md).

  - [x] **GAME-001F — Live hyperwarp level transition (accepted):** after admitting the
    final map row, keep normal gameplay active until its terrain scrolls fully
    off the bottom. Then enter bounded `LEVEL_TRANSITION` and send surviving
    seekers into their existing retreat movement. Preserve player steering,
    laser and fireball movement, and normal combat collision checks. Ramp the
    starfield from normal travel into a fast hyperwarp, hold briefly, and ease
    it back to normal speed before admitting any row of the next level. Preserve
    the optimized terrain scroller as a strictly one-scanline-per-frame path.
    Add separate starfield-only warp code for the 256x256 background images;
    that code resizes the exposed-strip graphics viewport to its current
    multi-scanline displacement so fast scrolling does not redraw the full
    field. Acceptance: no level data is read beyond its final
    row; remaining terrain and active records leave before the warp;
    starfield approach, acceleration, fast hold and deceleration remain distinct
    bounded phases; bezel/cockpit art does not scroll; every newly exposed
    scanline is painted without seams at every selected speed; the next level
    cannot appear until speed is normal; and a one-level tileset loops through
    the complete transition rather than restarting immediately. No transition
    phase may attempt fast terrain scrolling or generalize the existing terrain
    row-ingestion loop to multiple scanlines per frame. The automated player
    victory flight remains available for another state but is not used here.

    - [x] **Level exit:** the final terrain and active ground tiles clear at
      normal speed before warp; the final map row is the read boundary.
    - [x] **Warp and arrival:** only the starfield accelerates, holds and
      slows; the bezel and cockpit stay fixed, no scanlines are exposed, and
      the next terrain enters at one pixel per frame.
    - [x] **Single-level wraparound:** the level repeats only after the full
      transition. No separate fixture is needed.
    - [x] **Combat during between-level warp — closed, probably working.**
      Player laser fire looked correct. A seeker collision appeared to have
      the expected outcome, but was not confirmed. No dedicated retest required.

  - [x] **GAME-001G — Original Z80 campaign level port (accepted):** retain the modern
    Agon campaign as level zero and follow it with the recovered original
    Z80 game's 255-row main level. Import only the old level definition and its
    five passive tile images; do not import executable Z80 code. Map the old
    landing-pad and laser-turret IDs to the modern `tile_pad_small` and
    `tile_turret_fireball` IDs so current animation, spawning, firing,
    destruction and crater behavior remain authoritative. Acceptance: the
    checked-in TMX decodes all 255 original rows in their runtime order; its
    mapped cells have the frozen expected digest; the passive art is rebuilt
    reproducibly through Agon64 and canonical `agonutils`; the production map
    manifest deterministically emits both levels; the complete asset build and
    assembly pass; and an Author play-through confirms the old level's layout,
    artwork, pads, turrets and crater transformations before this item closes.

  - [x] **GAME-001H — Fireball directional rendering:** make each turret
    fireball's visual orientation come from the same launch heading that
    produces its X/Y velocity. Apply the bitmap's half-turn artwork correction
    only after choosing that launch heading. Select the correct directional
    frame immediately when a pooled fireball is spawned or reused, then
    alternate its two animation banks without losing the orientation.
    Acceptance: stationary-target, moving-target lead, cardinal and diagonal
    shots all point along their observed travel; no projectile appears on stale
    frame zero for its first animation interval; the animation alternates both
    banks while retaining direction; every selected frame stays within the
    64-frame fireball set; and reuse cannot inherit heading, frame or timer
    state from the previous occupant.

  - [ ] **GAME-001I — Rate-limited turret tracking:** separate the targeting
    computer's desired lead heading from each turret's physical barrel heading.
    Advance the barrel by a tunable maximum angular step along the shortest
    wrapped path instead of snapping whenever target position or velocity
    changes. A fired projectile's velocity and displayed orientation must both
    derive from the physical barrel heading at the instant of firing, so a
    sufficiently close lateral pass can outrun the turret and cause a miss.
    Preserve the current reload cadence initially: a ready turret fires along
    its current barrel heading rather than silently substituting the ideal
    solution or waiting indefinitely for perfect alignment. While the player
    is DYING, DEAD, RESPAWNING or in GAME_OVER, disable turret fire and replace
    player targeting with one shared search behavior: latch clockwise or
    counter-clockwise motion for a bounded random duration, flip direction,
    choose another duration and repeat through the same physical slew limiter.
    Give desired heading, search direction and search duration explicit named
    fields in the unused tail of each existing active-tile record; do not
    overload unrelated legacy fields or create a parallel turret-state table.
    Acceptance: motion is visibly bounded and continuous across the angle wrap;
    changing between stationary and moving-target solutions never snaps the
    barrel; close passes create a useful but risky tracking-rate defense;
    ordinary distant targets remain threatening; shot motion and artwork agree;
    search direction changes remain rate-limited; returning to live play does
    not inherit a search timer as a reload timer or snap to the new solution;
    and deterministic tests cover clockwise, counter-clockwise, equal-heading,
    half-turn and wraparound cases. Keep the legacy internal laser/turret names
    during stabilization; any terminology cleanup belongs to the later
    reviewed consistency pass.

  - [ ] **GAME-001J — Animated game-over sequence:** replace the blocking
    text-mode `game_over` handler with a bounded `GAME_OVER` lifecycle. Freeze
    the starfield and terrain behind a centred, stylized `GAME OVER` title
    whose colours alternate. Smith approved plotting the title as a regular
    bitmap on the frozen backdrop. Use the clean ordinary exit on current
    firmware as the baseline and verify it again after the renderer change.
    Keep the smaller `Press any key to continue` hardware sprite and its
    separate blink cadence for the first comparison. On entry, remove the
    player and hostile projectiles but make the remaining world non-hostile
    rather than empty. Suppress new launches and send existing seekers directly
    into shared retreat. Each chooses the nearest active, visible launchpad
    and despawns when it docks. If no usable pad remains, it flies through the
    nearer horizontal playfield edge and despawns before its sprite would
    cross into the cockpit or bezel (GAME-001B-EDGE).
    Visible laser turrets reuse the shared non-firing search
    behavior already active during player death and respawn. Require a complete
    key-up gate followed by a fresh keypress so the input associated with the
    final death cannot skip the sequence. Acceptance: title animation, prompt
    flash, seeker retreat, turret search, input polling,
    active-scenery animation and one VBlank all
    remain bounded to one outer frame; turret search never fires and never
    snaps; the overlay stays fixed over the frozen playfield without corrupting
    the bezel; no actor can damage or score after game over; held input cannot
    dismiss the sequence; continuation tears down every overlay resource and
    enters clean new-game initialization without reloading already resident
    assets; and repeated game-over/new-game cycles neither leak hardware sprites
    nor inherit stale enemy, turret, timer or input state. Freeze and qualify
    the exact hardware-sprite VDP API and required firmware before
    implementation or deployment. Retain a one-scanline scrolling diagnostic
    comparison in [GAME-001JK-INTEGRATED](docs/tasks/GAME-001JK-INTEGRATED/TASK.md).

  - [ ] **GAME-001K — Living attract mode:** replace the post-load static,
    opaque text prompt and blocking `mos_getkey` wait with a bounded TITLE/
    ATTRACT lifecycle. Preserve the accepted loader and begin attract mode only
    after all resident assets are available. At minimum, present a continuously
    moving starfield at the ordinary one-scanline pace with a fixed,
    transparent, stylized hardware-sprite `Press any key to start` prompt.
    Require key release followed by a fresh press, then tear down attract-only
    resources and enter new-game initialization without reloading assets or
    leaking attract state. Move the current blocking joystick choice out of the
    autonomous attract update and give configuration its own bounded start-flow
    boundary. Later, consume a deterministic replay captured by `TEST-001` so
    the classical title/demo loop can show one of the autonomous player's
    better complete games, return to the title after replay completion or
    failure, and remain interruptible by player input on every frame.
    Acceptance: the minimum attract screen never blocks, scrolls exactly once
    and waits for one VBlank per outer frame, keeps the prompt fixed and
    transparent over the moving playfield, cannot be skipped by a held loader
    key, and starts an ordinary clean game repeatedly. Replay mode must pin the
    exact build/map/seed and input trace, isolate demo score/lives/RNG from the
    real session, detect completion/death/stall/fault, loop cleanly, and yield
    immediately to a fresh human start input. Reuse the qualified overlay-
    sprite ownership and cleanup conventions from `GAME-001J`. Stretch goal:
    admit a live nondeterministic AI input provider through the same per-frame
    control interface, with bounded decision time and the same interruption,
    isolation and terminal watchdogs as replay mode; this is not a gate for the
    minimum attract screen or deterministic replay.

- [ ] **MATH-001 — Review the production 16.8 trigonometry contract:** local
  owner task for central survey `VDPL-037` identifiers `NUM-01`, `FUP-001`,
  `FUP-004`, `WIRE-001`, `WIRE-002`, `WIRE-004` and `WIRE-005`; see
  `/Agon/mystuff/agon-vdplet/docs/tasks/VDPL-037/NUMERIC-METHODS-SURVEY.md`.
  Treat the survey finding as an unverified defect report and review
  `src/asm/fixed168.inc` independently of `GAME-001`. In particular, determine
  whether the active `sin168` path consumes the fractional angle byte or
  overwrites it while constructing a three-byte table index, whether its
  documented interpolation and endpoint samples are reachable, and whether
  the behavior is intentional, defective or different from the intended or
  historical implementation. Assume neither a winning method nor a repair.

  **Execution hold:** this entry is task preparation only. Do not begin its
  source inventory, audit, experiment, code change, build, test, benchmark,
  reservation, device/firmware work, adoption or peer dispatch until Smith
  discusses and directly releases the review. Existing retained evidence may
  be named by the task, but it is not authority to refresh or extend it.

  When released, freeze the exact current source, binary, generated listing,
  lookup tables and calling conventions before analysis. Inventory every
  active `sin168`, `cos168` and `atan2_168fast` caller and its input/output
  representation, required precision, range, wrap/quadrant/negative-angle
  semantics, register/flag ABI and conversion boundary. Record provenance and
  licences for the current tables, any recovered historical implementation and
  any candidate reusable method; comparison does not imply permission to copy.

  Compare lookup-only execution with the intended interpolation and with
  relevant central-survey candidates using actual generated eZ80 code and
  complete caller costs. Accuracy evidence must cover the full angle domain or
  an equivalently complete proof, cardinal and quadrant boundaries, negative
  angles, wrap, discontinuities, rounding, overflow, upper-byte safety and
  `atan2_168fast` zero-axis cases. Proposed matched measurements must keep
  emulator correctness distinct from real-processor timing and account for
  table/code bytes, cycles, conversions, caller frequency and any future
  explicit angle256 wire encoding on both processors; isolated microkernel
  timing cannot select a winner.

  Failure controls: retain the frozen baseline; make no in-place production
  replacement during review; fail closed on out-of-domain inputs or ABI drift;
  require deterministic reference vectors and independent output comparison;
  and keep any eventual migration separately reviewed and reversible. Return
  to `VDPL-037` a concise source/evidence inventory, measured contract and
  error/range results, provenance/licence findings, consumer-impact matrix,
  reusable-method assessment, and repair/migration recommendation—or a finding
  that no change is warranted. No production math code or consumer changes
  before that return is reviewed.

- [ ] **FW-002 — Pinned-default runtime qualification:** preserve the frozen
  historical emulator profiles, then prepare a separately reviewed Alexe
  profile whose VDP runtime representation is proven to match VDPlet
  `v0.1.2-alpha`, selection `0.1.2-alpha-async-r1`
  (`dc43ecb8b1c324ffd1a032e6cc0c56d8c05dd33a355ce170c23adb35ffb533ab`),
  and whose MOS is Eirene `3.1.0_uart0_rc_3`
  (`d7a51147f3381cd68d56f3fbe4605434a897dfae1b049269afeba09ec2b7eb50`).
  Qualify the current production bundle there before map or UART continuation.
  For Olimex procedures and current production baseline, use only the
  [canonical bench guide](/Agon/mystuff/agon-extender/docs/operations/olimex-bench/README.md).
  Do not run UART-001 there until a reviewed variant handles the EMOS
  syscall-25h admission result. For Console8, verify both installed release
  images before the separately authorized human run. Record exact profile,
  firmware, bundle and result identities; no older profile or prior hardware
  acceptance may stand in for the pinned pair.

- [ ] **UART-001 — RC3 asynchronous render qualification:** integrate the
  tagged Eirene MOS `3.1.0_uart0_rc_3` syscall-25h sender behind a controlled
  game fixture. First compare an output-silent `move_sprite_group` cohort using
  legacy fragmented synchronous output, packetized synchronous output, and the
  byte-identical packetized asynchronous path. Use dedicated immutable static
  packet buffers, a flag-only ADL callback, one outstanding submission, and an
  explicit source/TEMT/receiver drain before ordinary output or exit. If that
  integration proof passes, extend the same controls to a repeatable full-frame
  scene with matched logical frame/input pacing. Record bytes and command-stream
  equality, actor/collision telemetry, frame-time tails, retained foreground
  work, input/quit latency, callbacks, timer drift, and clean repeated `RUN`.
  RC3 is pinned at commit `97311c437093e416342a6c836cf9914f2ced027d`,
  ROM CRC32 `B08C80B8`, 100879 bytes, SHA256
  `d7a51147f3381cd68d56f3fbe4605434a897dfae1b049269afeba09ec2b7eb50`.
  Preserve the original RFC image/results as historical evidence only; do not
  use the affine-sprite firmware or begin hardware deployment without a
  separate reviewed gate.

- [ ] **ART-001 — Cel UI assets:** prepare the accepted splash, logo and two
  bezels at their existing runtime dimensions and IDs using canonical Agon64
  conversion; rebuild and verify the UI container, then deploy to the existing
  local emulator for the Author's visual acceptance before committing.
  [Full-frame vector splash experiment](docs/tasks/ART-001/TASK.md).
  See also the [original starfield rollback](docs/tasks/ART-001/original-starfield-rollback.md),
  which supersedes the ART-002 sparse-starfield adoption.

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
- [ ] **ASM-011 — Actor-record schema and lifecycle audit:** review the complete
  sprite and active-tile record framework before adding the GAME-001 actor
  states. Preserve the Author's intended on-rails model: one shared 64-byte
  record header, a schema declared in each actor type's own include, generic
  allocation/spawn/transform/deactivation routines, and actor-owned
  initialization plus validated behavior selection. Inventory every current
  pool and actor type; map bytes 0 through 63 as shared, type-specific, unused
  or accidentally overloaded; and trace every path that allocates, copies,
  clears, reuses, transforms, activates, changes behavior or releases a record.
  Document carry/index-register/table-pointer contracts, copy bounds, failure
  behavior and which fields each layer owns. Identify stale-tail hazards,
  semantically overloaded fields, unvalidated behavior pointers, duplicated
  initialization and departures from the common framework without beginning a
  broad rewrite. Acceptance: publish a canonical schema/lifecycle document and
  machine-checked offset/bounds assertions; define the required per-type header
  pattern and actor initializer/behavior-transition contracts; add sentinel
  tests for fresh allocation, record reuse, pool exhaustion, cross-domain spawn
  and in-place tile transformation; prove every current actor reaches a fully
  initialized valid behavior or fails without exposing a record; and present a
  separately reviewable remediation sequence that gates `ASM-001I`,
  `ASM-001J`, `GAME-001B`, `GAME-001I` and `GAME-001J`. This is a correctness
  and ownership audit, not the deferred stylistic rewrite in `ASM-001M`.
- [ ] **TEST-001 — Autonomous qualification player:** after deterministic
  full-machine input is available, implement a project-owned RAM-driven agent
  that can complete the short qualification maps, emit a replayable input and
  state trace, and distinguish completion, death, stall, and runtime faults.
  Framebuffer-only play is a separate later experiment, not a prerequisite.
- [ ] **AI-001 — Autonomous gameplay laboratory:** build a research layer on
  the headless emulator without coupling experimental training to the stable
  `TEST-001` qualification player.

  - [ ] **AI-001A — Episode and step interface:** expose a versioned structured
    RAM observation, normalized keyboard action, deterministic reset/seed/map
    selection, bounded frame advance, snapshots, score delta, terminal reason
    and fault state. The controller may inject input but must not patch gameplay
    RAM. Acceptance: the same build, initial snapshot, seed and action trace
    reproduce the same observations and score; parallel episodes cannot share
    mutable state; and no-render fast-forward is byte-equivalent at admitted
    observation boundaries to canonical headless execution.
  - [ ] **AI-001B — Deliberative agent pilot:** let the development agent read
    one structured game observation, explain its tactical assessment, choose a
    keyboard action plus bounded hold duration, advance the emulator and repeat.
    Preserve the complete observation/action/reasoning trace so play can be
    reviewed and replayed without another model call. This explicitly realizes
    “teach Codex to play Nurples”; it is a bounded interactive experiment, not
    the mechanism used for millions of episodes.
  - [ ] **AI-001C — Local mass-training bot:** benchmark and select a compact
    policy/search representation, then run budgeted parallel headless episodes
    locally with no language-model calls, network dependency or per-turn token
    use. Train across an admitted map/seed curriculum while maintaining a raw-
    score hall of fame. Episode limits and terminal oracles must reject hangs,
    faults and unbounded score farming. Final ranking uses the game's actual
    score; completion, progress, lives and elapsed frames are recorded as
    tie-breakers rather than silently replacing the objective. Checkpoint the
    trainer, policy, build identity and champion input trace so long searches
    are resumable and auditable.
  - [ ] **AI-001D — Champion qualification and promotion:** replay the maximum-
    earning candidate on the canonical emulator with exact build/map/seed/
    policy hashes, compare every admitted state checkpoint, and preserve its
    run as a versioned artifact. Maintain a separate across-seed scorecard for
    robust policies so a spectacular single-seed game is not mistaken for
    general competence. After human review, export either the champion trace
    for deterministic attract playback or a bounded compact policy for live
    nondeterministic attract mode; hardware promotion remains a separate gate.
  - [ ] **AI-001E — MechAgon portability proof:** coordinate the reusable
    episode/training layer with MechAgon, whose ordinary gameplay is already
    bot-driven. Keep reset/step orchestration, isolated parallel workers,
    deterministic seeding, snapshots, checkpoints, policy provenance,
    tournaments and champion storage game-neutral. Nurples and MechAgon each
    retain their own observation adapter, action vocabulary, objectives and
    terminal rules. Acceptance: one versioned runner contract executes admitted
    episodes for both games without either importing the other's game logic;
    deterministic traces reproduce independently; MechAgon can run bot-versus-
    bot populations while Nurples can run score-maximizing single-player
    populations; and fixes discovered by either consumer are regression-tested
    against both adapters before the shared interface changes.
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
