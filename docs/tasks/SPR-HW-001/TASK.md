# SPR-HW-001 — Explicit hardware sprites for gameplay

## Decision and rollback

All real VDP sprites owned by Nurples use the hardware sprite API. All active
terrain tiles remain regularly plotted bitmaps. Freeze and push this task on
`dev` before implementation. The task-only commit is the rollback marker;
its game code is identical to `90010fd` (loaded/cleared turret build).

## Implementation boundaries

- At sprite setup, explicitly select and mark each gameplay slot as hardware
  (VDU 23,27,19) after its frame bank is defined and before activating the
  prefix. Cover the ship, lasers, seekers, other air enemies, fireballs and
  explosions. Derive the range from the actor table, not a literal count.
- Do not depend on the user's MOS startup or global automatic-hardware setting.
  Use the existing feature-variable transaction and preserve its incoming
  presence/value. Keep reserved presentation slots separate, including the
  existing hardware prompt and bitmap Game Over title.
- Keep frame-list changes and compositor changes off active scanout. Audit
  spawn, death, respawn, level/session reset, overlay transitions and exit.
  Preserve physical coordinates, pool limits, movement, collisions and timing.
- No changes to the active-tile draw loop, terrain art, map format, MOS, VDP
  firmware or hardware card as part of implementation.

## Acceptance

1. Assemble the ordinary game and run regression checks covering slot range,
   frame-definition/mark/activation order, lifetime and cleanup. No per-frame
   promotion or dependence on auto-hardware mode.
2. FAE review: verify ship placement, laser, enemies, fireballs, explosion
   frames, respawn, Game Over, restart and Escape. Turrets must continue to
   scroll and rotate as bitmap plots.
3. Console8 review separately before calling the conversion hardware-qualified:
   watch a busy scene for scanout artifacts, missing sprites or bezel bleed;
   confirm overlays, restart and return to a responsive MOS prompt.
   Neither source checks nor FAE can establish physical scanout performance.

## R01 — FAE reviewed; Console8 qualification failed

Rollback is `31d0497`. Setup defines the complete frame banks, opens or
borrows the existing feature transaction, marks slots 0–27 explicitly, then
activates them. Upstream checks feature variable 2 when handling command 19;
the hardware bit lives on the sprite itself. No change to auto-hardware mode,
overlay ownership, active-tile plotting, or the game clock. Reset detaches the
active prefix before frame lists are cleared. Respawn reuses the ship bank;
scripted movement no longer sends a software refresh.

Ordinary build: 243 tests passed, two optional tests skipped. FAE card
readback matched executable SHA-256
`a97a3e0f95d9301fee9721f2a98a9e7f07d78c3b71cc8b18a08e1420247804db`
and the unchanged game artwork. No Console8 deployment in this tranche.

**Separate discussion: baseline frame timing.** Before this conversion Smith
observed several brief episodes on Console8 in which scrolling and actor
movement slowed together, then recovered. This is consistent with a missed
game-loop update; the observation does not distinguish eZ80 computation from
VDP/UART waits or other causes. Preserve the baseline for a like-for-like
comparison; no timing instrumentation or mitigation added in R01.

Smith reported that R01 looked better on FAE. The same executable was then
deployed to Console8 SD at `/mystuff/nurples`; all six payload files matched
on readback and the root autoexec hash was unchanged.

### Console8 R01 — Smith's observations

- Gameplay sprites did not flicker. With many visible, the first one or two
  scanlines sometimes looked corrupted or missing and the entire picture
  shifted down by that amount for a frame. Similar jumps on Game Over;
  coincidence with flashing was not established. Do not conflate this with
  the baseline report of actor and terrain motion slowing together.
- On this R01 run Smith saw no obvious missed 60 Hz gameplay updates. This
  is a non-reproduction by eye, not a measured zero or proof of an eZ80 fix.
  Ordinary gameplay scheduling is unchanged. Scripted ship drawing omits an
  explicit sprite refresh, and respawn no longer rebuilds the ship frame list.
- After one full game, starting a new game gave a black screen and flashing
  cursor. Collision sounds continued; firing produced no audible cue.
  Neither a CPU halt nor a return to MOS was verified.
- The ordinary 240×64 Game Over title remains a bitmap plot (color changes
  every eight updates). The 208×16 continue prompt already used hardware
  sprites before R01 and toggles every 30 updates. Same-scanline sprite
  workload is a hypothesis, not an established timing diagnosis.
- Do not promote. First isolate restart: compare Game Over exit, display
  verification/redraw, gameplay sprite bank reinitialization and first playable
  update. Separately compare equal sprite counts on shared versus staggered
  scanlines, and prompt hidden versus visible with other draws held constant.

### R02 — hardware sprites with M accepted on FAE

The clean fallback with the previous renderer and Smith's FAE-approved M
switch is `6f6ce06201bbb09a4dd0df1721d530ff4cf04e25` on dev. Built that
revision with the pending R01 hardware sprite changes. Verified in the linked
game that the input handler tests M and stops PRT1, and that sprite setup calls
the hardware-mark routine. Combined binary SHA-256:
`5e43616288861273e4caad7811b8c6a1551381b7a6f5782e02d36c9fe7a2b6c1`.
245 tests, 2 skipped, no failures. The Oryx FAE card readback matched the
combined binary. Smith approved the build for commit on dev. No new Console8
test or rendering controls; R01's hardware scanout and restart findings remain open.

### R03 — isolate rendering costs in the live game

Build a fixture from accepted hardware-sprite dev `4865f04`: top-row 1
inhibits background texture plots, 2 inhibits the ordinary terrain-row plots,
and 3 inhibits the playfield scroll command. All start enabled and toggle
independently on fresh presses. Keep game time, row admission, active tile
behavior and M independent. Do not clear or rebuild existing screen pixels
on a toggle. This is a workload discriminator, not a new production display
mode. Physical scanout still requires a Console8 observation.

Built with `build/scripts/build_00_all_the_things.py --scanout-fixture`
under the project venv. Guest readback on Oryx FAE:
`013d7184b277f958ad2ddd100330e97f42d9b4b34d5c43a3acbc6a6400a0bac9`.
Ordinary target unchanged; suite 245 tests, 2 skipped, no failures. Inspected
the linked key sampler and terrain bypass past the draw loop but before
active-tile admission. Awaiting playtest; no hardware deployment.

Console8 follow-up: zero incoming shield damage only in the fixture assembly
stage. Contact checks, collision audio and actor responses stay live. Built
and checked the linked damage instruction; 245 tests, 2 skipped, no failures.
Replaced the six-file runtime on AGONC8 at `/mystuff/nurples` and verified
all six by readback; executable `2f6f40a7cdce130659996ce95424cc83cbe41919691fac51440616dc089b7946`.
Autoexec unchanged. Prior install and MUSIC50.BIN retained on Oryx in
`~/Agon/mystuff/nurples/scanout-c8-r02/previous-install`.

Console8 observation: Smith found that switching off background drawing,
ordinary tile drawing and scrolling together made little difference to the
visible jumps. Sprite count and size appeared to have the greatest effect.
This is a visual observation, not a measured compositor deadline miss.
Next proposed test: [R04 — interactive scanline lab](scanline-lab.md).

### Live-game comparison: software formation enemies

In the invulnerable 1/2/3/M fixture only, mark small/medium/large air slots
as software sprites at bank setup. The linked selector routes slots 12–19
to command 20, and all other gameplay slots to command 19. No change to
spawn height, movement, refresh cadence, or active tiles. Built fixture
`82d6e1654f2e16f704338eb4136d248464ca9b4ca200b1ebcea760fb0cdf349b`;
245 tests, 2 skipped, no failures. Hardware visibility threshold unmeasured.

### Olimex formation draw order — research, not an implementation

Smith reports that as more formation ships appear, flicker persists farther
down the screen. No pixel threshold or timing trace yet. This concerns software
bitmap drawing racing the beam, distinct from dense hardware scanline composition.

Evidence: in the Olimex v0.1.3-alpha release firmware-source archive, under
`firmware-src/.pio/libdeps/esp32dev/vdp-gl/src/`, `displaycontroller.cpp`
(`showSprites`, line 726) walks the active prefix in ascending ID, drawing
eligible software bitmaps; `hideSprites` (700) restores saved backgrounds in
reverse order when single buffered. `dispdrivers/vgapalettedcontroller.cpp`
(`drawSpriteScanLine`, 533) walks the prefix separately and composites only
eligible hardware sprites. No requirement to put software below hardware;
shuffling a fixed active prefix does not shorten either traversal.

Nurples currently allocates the first free record upwards (`air_launch` →
`spawn_sprite_from_coords` → `table_get_next_id`). Proposed A/B: keep the
same eight records, stable physical IDs, original entry heights, formation
and toggles; change only the formation free-slot search to highest-free first.
On a fresh wave this gives the younger, higher rank earlier bitmap draws.
Reused slots and crossing trajectories need not preserve that relationship.
Compare rank visibility at the same positions/count with animation running;
ordering can redistribute missed deadlines, not eliminate the total draw cost.

Firmware alternative, undecided: opt-in software draw-order list by sprite
top Y, then ID; never reorder the sprite records or change hardware ordering.
Keep ID layering as default. Snapshot at redraw; when backgrounds are saved,
restore in reverse of the *previous actual draw list*, not a new Y sort.
Equal-Y ties stay deterministic; different-Y overlapping sprites can still
change layering. Test overlap, crossing, hide/despawn and restore before
considering a firmware change. No runner, game or firmware change in this review.

**Shipping constraint (Smith): bone-stock VDP and MOS.** Firmware Y-sorting
is a separate research idea, not a Nurples dependency or shipping task.
The source trace above is for the Olimex VDPlet pin, not a stock-firmware
hardware qualification.

Proposed assignment: reserve an eight-slot software block for formations.
It need only precede *other software sprites* that can tolerate later draws,
not start at ID zero. Keep player lasers and turret fireballs hardware for
visibility high on the playfield; their narrow footprints are a better use
of scanline capacity than a wide alien rank. Seekers could instead be software
and stay no higher than the player. Measure using the top visible pixel, not
the actor center. Open alternative for high launch/return pads: keep logical
flight and docking intact but hide the seeker while its bitmap crosses the
unsafe band, rather than clamping steering short of its pad.

Proposed test order: reverse only formation allocation within 12–19; compare
both ranks (does the upper improve without the lower flickering?) with eight
aliens alive. Only then try software seekers and repeat at higher load. Do
not adopt a constant safe Y from one formation count. Validate on Console8
with stock firmware before choosing a production policy. Longer-term option:
[assign stable slots to authored waves at build time](../../design/enemies/README.md).

### F40 — Earth formation execution contract

Deliver one formation of up to 40 real software sprites, using physical VDP
IDs 0–39, on stock MOS/VDP. No framebuffer formation art or firmware sort.
Smith authors the ranks in Earth City's Spawns layer; do not edit or deploy
over the Oryx design checkout. Preserve the compile-time terrain/spawn split.

- F40-01 [x] The first admitted small/medium/large Earth marker starts the
  formation; each successfully admitted ship owns a distinct slot, highest
  available first so later ranks get lower IDs. Never overwrite a live ship.
  Seeker markers and other levels' ordinary side-to-side spawns do not join.
- F40-02 [x] Track live membership once per successful admission/removal.
  Last kill or scroll-out returns the counter to zero; teardown/restart clears
  it too. If there are no survivors, the next eligible spawn starts anew.
  Preserve individual movement, damage, collision and visible-edge behavior.
- F40-03 [ ] Keep all other actor pools and overlay IDs disjoint, and avoid
  visiting 40 unused records each frame when no formation is active. Assemble
  and check capacity/order, full-pool refusal, cleanup and reuse. A real
  40-ship visibility/performance verdict requires Smith's layout and playtest.
  No new emulator launch, bench operation, or production promotion in this step.

Implementation evidence against freeze `0a38fa27c55fb673c0fb30027d690f9fd03a9457`:
ordinary and diagnostic games assembled. The linked bank is 40 × 64 bytes,
other actor IDs are 40–67 and overlay IDs 68–69. The empty-bank branch skips
formation movement. A linked eZ80 CPU check (`tests/formation_bank_cpu.rs`)
ran on **both** executables: fills in descending order, refuses the 41st
without changing the actor table, clears a mix of pending deaths and bottom
exits, tolerates repeated removal, excludes an ordinary non-Earth air launch,
restarts at slot 39, and resets at table initialization. Display I/O was
stubbed; this measures neither frame time nor flicker. Python suite: 245 run,
2 skipped, no failures. F40-03 remains open for the authored map and visual
review. No Oryx sparse checkout, live bench or deployment changes.

F40 Earth march audition: imported Smith's Oryx TMX (SHA-256
`6fb049ddbf3de2280d02dd0dd18362ac41f1b6297ec0f2c1343e0750be006a5a`).
The compiler groups eligible ranks separated by at most one spacer row;
two consecutive empty rows separate waves. It emits rank wait and leg lengths
from row spacing and centered sprite footprints. During the entrance, ranks
follow ground scroll until the last rank arrives, then march at 1 px/tick
horizontally and descend at half scroll speed. Wave 1 waits 64/48/32/16/0
steps and traverses 96 pixels each way. Wave 2 waits 32/0 and traverses 64.
Only wave 2's compiled admissions shift two rows later, so a no-kill wave 1
can leave before the next admission. No edits to Oryx authoring files.
Both assembled binaries passed the CPU position checks through a full sweep
and reversal; the Python suite passed (245 run, two skipped). The staged
mixed-renderer/invulnerable FAE card binary read back as
`16cae72cbbad231450eba9b2682ab2951dc490c0d4f04702ef4bb9fb82e89493`.
Smith reported the formation worked in FAE and requested the identical build
on Olimex. Olimex visibility and pacing remain unqualified; the previous
mixed-air packet pins an older executable. Follow-on commissioning requested.

Olimex F40 visual result (2026-11-24): Smith observed heavy flicker until
the formation reached approximately halfway down the playfield; from there
it rendered cleanly. Early visibility failed with this build and layout.
The midpoint is a visual estimate, not a measured scanline or a general
spawn-height limit. This run does not isolate draw order from total draw time.

### F40-HW: same ranks, hardware renderer audition

- F40-HW-01 [ ] Freeze the tested software control, then switch only Earth
  formation IDs 0–39 to hardware in the diagnostic build. Preserve both
  waves, timing, art, other slots and controls. Compare linked executables;
  stage as nurples.bin for Smith on Oryx FAE first. No Olimex access.
- Design observation, not part of this test: Smith saw Galaga keep the rack
  centered and expand/contract its vertical extent like breathing. Consider
  that motion separately; this comparison retains the existing lateral march.
