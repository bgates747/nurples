# Standalone gameplay map qualification suite

Status: implemented; initial emulator qualification accepted
Date: 2026-08-03

## 1. Purpose

This suite is a developer fixture, not a replacement campaign. It isolates the
gameplay systems that are actually implemented today in short, themed maps.
Each map should be selectable independently so that residual state from an
earlier map cannot invalidate its result. One deliberately dirty transition is
paired with a clean-room map to expose cross-level leakage.

If an integration map proves enjoyable as well as diagnostic, it may later be
promoted into normal progression by an explicit content decision. Test maps do
not enter the campaign merely because they exist.

## 2. Current engine contract

### 2.1 Geometry and time

Maps are 16 columns wide with 16-by-16-pixel cells. Runtime scroll is one pixel
per game update, so one source row enters every 16 updates. At the normal
60 Hz display cadence:

| Rows | Updates | Nominal duration |
| ---: | ---: | ---: |
| 32 | 512 | 8.53 seconds |
| 48 | 768 | 12.80 seconds |
| 64 | 1024 | 17.07 seconds |

The runtime consumes each row left to right. The existing converter reverses
row order: the bottom row in Tiled is runtime row `r0`, and play proceeds
upward through the Tiled document. For a map of height `H`, runtime row `r`
is Tiled row `H - 1 - r`.

An active tile appears around screen y=1, takes approximately 20 rows of time
to reach a player resting near the bottom, and roughly 24 rows to leave the
active-tile boundary at y=384. A normal 32-row fixture should therefore place
its intentional encounters in runtime rows 0 through 7 and reserve 24 upper
Tiled rows as run-out. The dirty-transition fixture deliberately violates that
rule.

Level completion occurs when the last source row is inserted at the top, not
when that row leaves the bottom of the viewport.

### 2.2 Tile vocabulary

Tiled global IDs are intentionally copied directly into the byte-valued map;
`firstgid` is not subtracted. Horizontal/vertical/diagonal Tiled flip bits are
not supported.

| Mark | GID | Implemented runtime behavior |
| :---: | ---: | --- |
| `.` | 0 | Transparent/null cell; starfield remains visible. |
| `p` | 17–255 | Passive visual tile. Final art may use any loaded DG tile in this range. |
| `S` | 1 | Animated small pad; repeatedly attempts to spawn seekers. |
| `T` | 5 | Orienting turret; fires predictively aimed fireballs. |
| `E` | 9 | Left electrode endpoint. |
| `=` | 10 | Animated lightning; blocks lasers and damages the player. |
| `e` | 11 | Mirrored electrode art using the same electrode behavior. |
| `C` | 16 | Directly activated crater lifecycle. |

IDs 2–4, 6–8, and 12–15 have null activation callbacks. In particular, the
shield and fuel graphics at IDs 4 and 8 are not recharge stations. They must
not appear in this suite as behavioral tests. Medium/large pads, laser/twin
laser turrets, shield/fuel charging, pause, victory, cutscene, inventory, map,
and the out-of-fuel consequence are not implemented and are outside current
coverage.

Decorative cells must use loaded IDs 17 or greater. A low-numbered decorative
choice can silently instantiate an active record or exercise a null callback.
Passive art has no collision behavior: a wall-shaped graphic is not a wall.

### 2.3 Capacity and lifecycle facts

The suite intentionally exposes, without exceeding, these finite pools:

| Resource | Capacity | Relevant behavior |
| --- | ---: | --- |
| Player lasers | 4 | Rate limited, two-frame animation, power use/recharge, slot reuse. |
| Seekers | 3 | Pad spawn attempts beyond three must fail safely; killed slots are reusable. |
| Fireballs | 4 | Multiple turrets must tolerate a full projectile pool and reuse freed slots. |
| Explosions | 8 | Enemy kills and the player-death sequence must tolerate exhaustion. |
| Active tiles | 255 | Circuits exercise dense-stack traversal and compaction well below the limit. |

Pads and turrets currently have an initial-delay edge case: random zero is
decremented to 255 before the handler tests it. A fixture must not require every
launcher to fire immediately. This defect is tracked separately as ASM-010;
the multiple launchers in the nursery and range make their primary behavior
observable without concealing the fault.

## 3. Common operator protocol

Each map has a primary observable purpose. Free play is welcome, but its basic
qualification pass should take only one or two attempts.

1. Start the selected fixture from freshly initialized game state unless its
   card explicitly requires a preceding map.
2. Exercise only the actions named on the card before improvising.
3. Treat a freeze, malformed sprite, stale actor, corrupted HUD, or fault latch
   as a failure even if the map later completes.
4. Record whether the expected score, shield, life, transition, and crater
   outcomes occurred.
5. For maps involving destruction, allow explosions and craters enough time to
   complete their visible lifecycles.

Until ASM-008 supplies a level-scoped world reset, ordinary isolation maps
should be launched one at a time. Only `PN-MAP-06` followed immediately by
`PN-MAP-07` is intended to measure the current transition boundary.

## 4. Map cards

Event rows below are runtime rows. Unlisted rows contain only transparent or
passive art. Strings are exactly 16 columns; passive artwork may change without
changing the behavioral layout.

### PN-MAP-00 — Flight Deck

Theme: a quiet launch corridor.
Size: 16 by 32; approximately 8.5 seconds.
Primary test: baseline scroll, player control, lasers, HUD cadence, and clean
level completion with no active-map actors.

Behavioral rows: none.

Operator card:

1. Move to all four field edges and both upper and lower limits.
2. Hold fire long enough to consume laser power and exercise all four laser
   slots; release and observe recharge.
3. Verify passive terrain and starfield scroll continuously and the level ends
   without an enemy, explosion, or active tile appearing.

This map is also the baseline for startup, repeated launch, and frame-rate
observations, although those are broader than map behavior.

### PN-MAP-01 — Seeker Nursery

Theme: an automated drone hatchery.
Size: 16 by 32; approximately 8.5 seconds.
Primary test: pad animation, seeker spawn saturation and slot reuse, homing,
zig-zagging, edge recovery, collision, scoring, and explosion lifecycle.

```text
r00  ..S..........S..
r04  .......S........
r07  ....S......S....
```

Operator card:

1. Do not fire until three seekers have appeared; additional pad attempts must
   not create a fourth or corrupt another sprite.
2. Draw a seeker toward a side and bottom edge and confirm it recovers without
   freezing or moving twice in one update.
3. Destroy one seeker for 10 points, then confirm a pad can reuse the freed
   seeker slot.
4. Destroy one pad for 5 points and observe its in-place crater transition.
5. Optionally collide with one seeker and verify four shield points of damage
   and an explosion.

### PN-MAP-02 — Fire-Control Range

Theme: a predictive-gunnery calibration lane.
Size: 16 by 32; approximately 8.5 seconds.
Primary test: turret rotation, positive and negative lead angles, fireball pool
saturation/reuse, projectile collision, scoring, and turret-to-crater change.

```text
r00  ..T..........T..
r04  .......T........
r07  ....T......T....
```

Operator card:

1. Hold still for the first volley to establish direct-bearing behavior.
2. Traverse left-to-right for one volley, then right-to-left for another; shots
   should lead in opposite directions rather than always aiming head-on.
3. Let the turrets fill the four-fireball pool; later spawn attempts must fail
   safely, and freed slots must be reused.
4. Take one fireball hit for two shield points. Player lasers do not destroy
   fireballs; evasion is the defensive behavior under test.
5. Destroy a turret for 5 points and watch it become a crater.

### PN-MAP-03 — Circuit Breaker

Theme: three isolated power buses.
Size: 16 by 32; approximately 8.5 seconds.
Primary test: electrode/lightning activation, animation, laser absorption,
shield damage, row-local circuit destruction, dense active-tile compaction,
and crater conversion.

```text
r00  .E============e.
r04  ....E======e....
r07  ......E==e......
```

Only one circuit appears on a row. This makes the current same-y lightning
removal rule observable without one endpoint ambiguously disabling a second
circuit.

Operator card:

1. Fire into the middle of the first lightning span; the laser should vanish
   while the circuit remains live.
2. Shoot one endpoint; it should award 5 points, become a crater, and remove
   the lightning belonging to that row.
3. Fly through the cleared lane.
4. Use a later, shorter circuit to sample one-point lightning damage or
   four-point electrode damage, then verify no unrelated row is removed.

### PN-MAP-04 — Demolition Yard

Theme: a scarred weapons proving ground.
Size: 16 by 32; approximately 8.5 seconds.
Primary test: direct crater activation versus in-place transformation, crater
animation/settling, simultaneous destruction, score updates, and active-table
record churn.

```text
r00  ..C...C..C...C..
r04  ....S......T....
r07  .C.C.C.C.C.C.C.C
```

Operator card:

1. Observe map-authored craters animate and settle without blocking lasers or
   damaging the player.
2. Destroy the pad and turret and compare their transformed crater lifecycle
   with the directly authored craters.
3. Fire through the dense crater field while both destruction explosions are
   active; no record should disappear incorrectly or acquire another tile's
   behavior.

### PN-MAP-05 — Damage Control

Theme: a failing reactor deck with a marked bypass lane.
Size: 16 by 64; approximately 17.1 seconds.
Primary test: shield HUD, scoring rollback, player death animation while the
world continues, seeker orbit-on-death, life decrement, table reset, respawn,
and eventual game over.

```text
r00  .......S........
r04  ....T......T....
r08  ....E======e....
r12  ..E==========e..
r14  ..E==========e..
```

The two outermost columns on each side form the deliberately marked bypass.
The extra clearance matters because edge-touching rectangles count as a
collision. The long run-out keeps the level alive while the blocking legacy
death sequence completes.

Operator card:

1. Destroy one object to establish a nonzero level score.
2. Move to the center, stop firing, and remain in the two full-width hazard
   spans until shields reach zero.
3. During the explosion sequence, verify passive terrain, active craters, and
   surviving actors continue to advance; live seekers should switch to their
   death-phase orbit behavior.
4. Verify one ship is consumed, the score returns to its value at level entry,
   runtime tables are clean, and the map restarts with a controllable player.
5. On the next life, use column 0 or 15 to complete the map safely. Repeating
   deliberate deaths may be used to qualify game over and new-session restart.

### PN-MAP-06 — Dirty Handoff

Theme: abandoning a still-active weapons platform.
Size: 16 by 32; approximately 8.5 seconds.
Primary test: level completion with enemies, projectiles, explosions, and
active tiles still live. This is the focused qualification map for ASM-008.

```text
r00  ..S....T....S...
r06  .......T........
r18  ....S......T....
r26  .E============e.
r30  ..C..........C..
```

Operator card:

1. Leave at least one pad, one turret, one seeker, and one fireball alive.
2. Near the end, destroy one actor so an explosion is active during completion.
3. Do not interpret persistence as success: the required post-ASM-008 result is
   a level transition that removes every level-owned enemy, projectile,
   explosion, and active tile without erasing the player or player-owned state.
4. Continue directly into `PN-MAP-07`.

This map is expected to expose the present incomplete world-reset behavior; it
does not redefine that behavior as acceptable.

### PN-MAP-07 — Clean Room

Theme: an unmistakable empty diagnostics grid.
Size: 16 by 32; approximately 8.5 seconds.
Primary test: detection of state leaked by `PN-MAP-06`, followed by clean
tileset wrap into `PN-MAP-00`.

Behavioral rows: none. Passive art should form a high-contrast calibration grid
that cannot be mistaken for the previous map.

Operator card:

1. Enter only by completing `PN-MAP-06`.
2. Any inherited seeker, fireball, explosion, crater, turret, pad, electrode, or
   lightning is a visible failure.
3. Verify the player remains present and controllable with the intended score,
   shields, laser power, fuel, and lives.
4. Complete the map and verify suite wrap returns to a clean `PN-MAP-00`.

### PN-MAP-08 — Orbital Shipyard

Theme: a compact, readable assault on a working shipyard.
Size: 16 by 48; approximately 12.8 seconds.
Primary test: combined-system play and promotion potential.

```text
r00  ..S..........S..
r04  .......T........
r07  ..E====e........
r12  ...T........T...
r16  .C....C..C....C.
r20  ....S....T......
```

The passive art should tell a small visual story—approach lights, drone bays,
power distribution, gun deck, damaged hull, then open space—while preserving
obvious safe lanes around every mandatory hazard.

Operator card:

1. Play for survival rather than following a scripted diagnostic sequence.
2. Confirm pads, seekers, turrets, predictive fireballs, circuits, destruction,
   craters, scoring, resources, and level completion coexist without a freeze
   or malformed transition.
3. Judge pacing and readability separately from correctness. This is the map
   most suitable for later campaign promotion if it is genuinely fun.

## 5. Coverage matrix

| Behavior | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 |
| --- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| Baseline movement/fire/HUD | Primary |  |  |  |  |  |  | Verify | Integrated |
| Passive scrolling and completion | Primary | Verify | Verify | Verify | Verify | Verify | Boundary | Verify | Integrated |
| Laser power/rate/pool/recharge | Primary | Verify | Verify | Absorption | Verify |  |  |  | Integrated |
| Pad and seeker behavior |  | Primary |  |  | Transform | Death setup | Live handoff | Leak check | Integrated |
| Predictive turret/fireballs |  |  | Primary |  | Transform | Damage setup | Live handoff | Leak check | Integrated |
| Electrode/lightning circuit |  |  |  | Primary |  | Damage | Live handoff | Leak check | Integrated |
| Direct and transformed craters |  | Transform | Transform | Transform | Primary | Observe | Live handoff | Leak check | Integrated |
| Score and shields |  | Primary | Primary | Primary | Primary | Rollback/death | Preserve | Verify | Integrated |
| Player death/respawn/game over |  |  |  |  |  | Primary |  |  | Optional |
| Level-owned world reset |  |  |  |  |  | Life reset | Stimulus | Oracle | Verify |
| Full gameplay integration |  |  |  |  |  |  |  |  | Primary |

## 6. Standalone build boundary

The implementation produces a separate map-test binary and never overwrites
or silently replaces `tiles/dg/dg0_03.tmx` or the checked-in campaign level
include. Source and output ownership is:

```text
tests/maps/dg/*.tmx                    test-owned Tiled sources
tests/maps/dg/suite.toml               ordered fixture metadata and operator IDs
src/asm/levels_test_maps.inc           deterministic generated test-level data
tgt/nurples-map-tests.bin              standalone assembled application
```

The dormant `tiles/dg/tile.py` entry point must not be run to generate this
suite. It couples tile chopping, obsolete loose-file output, image include
generation, and a single-map writer that overwrites its destination. The
canonical production build remains separate from this fixture. The map-only
compiler validates:

1. One orthogonal 16-by-H CSV layer, with H from 1 through 256.
2. The expected DG external tileset and `firstgid=1`.
3. Exactly 16 byte-valued cells per row and exactly H rows.
4. No Tiled flip bits and no negative or greater-than-255 GIDs.
5. Every nonzero GID has a loaded DG bitmap.
6. Deterministic bottom-to-top emission into a multi-level pointer table.

Run `.venv/bin/python build/scripts/build_maps.py` to generate only the test
include, followed by `build_map_tests.py` to assemble through a temporary
source-tree overlay. `build_map_tests.py --start-map N` rotates the nine-entry
pointer table so fixture N runs first without changing its source definition.
The deliberate runtime order is 00, 01, 02, 03, 04, 05, 08, 06, 07 so the
dirty/clean pair is contiguous and Clean Room wraps directly to Flight Deck.
These tools do not authorize refactoring the campaign or reviving the dormant
MapMaker/Wolf pipelines.

## 7. Existing material worth reusing

The repository already contains useful short visual sources:

1. `tiles/dg/dg0_00.tmx` — 16 by 32, historically a small pad/turret test map.
2. `tiles/dg/dg0_01.tmx` — 16 by 64, a larger pad/turret test map.
3. `tiles/dg/dg0_bg_green_plates.tmx` — 16 by 32 passive green plating.

Their passive motifs can seed the new themes, but their active placements must
be rebuilt against these test cards rather than copied blindly.
