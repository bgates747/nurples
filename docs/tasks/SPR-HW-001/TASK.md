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
