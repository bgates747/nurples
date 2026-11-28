# EARTH-001 — Earth City opening level

Open the campaign on Earth, defending the city before following the attackers to their worlds. This silo owns the Earth level, from its editable map to Smith's first in-motion review.

- EARTH-001-FRAME [ ] [Synthetic Earth streaming frame-timing fixture](NURPLES-FRAME-TIMING.md).
  Nurples owns the draft transferred from Sprite. Awaiting design discussion
  with Smith; implementation and hardware testing have not started.

**Olimex timing follow-up:** [Check whether FAT fragmentation explains slow Earth reads](vblank-dispatch-review.md#olimex-sd-card-fragmentation-check).
Keep both staged files in place through the firmware comparison and five repeat runs; inspect their allocation read-only afterward.

**Current render contract:** Earth background strips contain Xevious pixels only.
The ordinary dev build and both aligned fixtures draw the Kenney Structures
layer with the regular tile renderer. Spawn markers are independent even when
placed over a structure. The original baked-strip description below is historical.

## Earth formation pose sends — proposed synchronous comparison

### Authorized execution order

On November 26 Smith approved proceeding, in this order: **ordinary stock
batch, asynchronous batch, state-only stock buffered commands written in
Golem, then native VDPlets**. This amendment supersedes the planning-only
and stop-before-async clauses below, not their output-equivalence requirements.
Owner: Nurples. No production promotion solely on benchmark results. Do not
write new MOS or VDP firmware to implement these arms. The native arm may
load an application module on existing module-capable firmware. Queued music
is a separate feasibility question, not a change to the pose comparison.

- EARTH-001-POSE-07 [x] Build matched direct and batched stock-compatible
  full-game fixtures. The old traced fixture uses EMOS async even for terrain:
  keep a 256-byte synchronous terrain payload in *both* new fixtures. Retain
  identical map, unaligned Earth file, 1 KiB reads, pending VBlanks, deterministic
  input, audio and splash lifetime. Keep last hardware-qualified binaries intact.

- EARTH-001-POSE-08 [ ] Implement async pose transport using the existing
  UART ABI without changing terrain transport. Protect packet lifetime, order
  relative to other commands, source release, failures and stream drain.
  Qualify on the supported MOS; stock modes do not call the extension.

- EARTH-001-POSE-09 [ ] Compile actual Golem source for state-only stock
  buffered commands. Verify decoded output against direct poses for mixed
  coordinates, order, zero and sparse counts. Report setup and per-frame cost.

- EARTH-001-POSE-10 [ ] Explore native module poses using installed capabilities,
  compare module on/off on identical firmware, and document whether the
  existing music queue supports timed PT3 packets without firmware changes.
  Do not substitute another firmware's baseline or alter music in pose runs.

- EARTH-001-POSE-11 [ ] Run after Sprite checks in, one physical operator at
  a time. Reset before each measured run. Report skips / VBlanks, completed
  updates, 1 KiB refill distributions and actual wire counts separately from
  computed bounds. Keep the reservation until Smith directs otherwise. No
  Oryx work; Smith authorized Pi5 emulator runs during this task. These are
  correctness checks, not substitutes for hardware timings.

**Execution record:** [Pose comparison — builds, checks and hardware results](pose/RESULTS.md).
The release game and the earlier SD timing fixtures remain separate from these arms.
The R02 native arm stopped on repeated RUN rejections. [Investigation plan,
POSE-15–18](pose/native-run-investigation.md): establish correct invocation
before interpreting performance. This does not advance the deferred scheduling experiments.

**Approved November 27:** [Direct UART vs. locally submitted poses from a C VDPlet (POSE-19–22)](pose/pure-native.md).
Use the existing public interface and the corrected native fixture above;
no firmware extension, Golem or music changes. Compare against the current
direct control. This measures game performance, not interpreter cost alone.

### After the full hardware suite: terrain deadlines and overdue formation poses

Finish and review the current R02 comparisons first: direct, synchronous batch,
async batch, Golem buffered commands, and native VDPlet against its matched
direct control, plus the planned cost measurements. Keep sending every eligible
pose in those fixtures. The follow-ups below are proposals, not changes to the
fixtures or authorization to resume the bench.

- EARTH-001-POSE-12 [ ] Locate the missed deadline before deciding what to
  defer. [Direct-build deadline observer: execution contract and results](pose/direct-deadlines.md).
  Check for a pending VBlank after terrain submission, after the last
  actor update, and after the final pose batch. Today the actor traversal also
  sends poses, so the middle checkpoint is not pure simulation time. Report
  dense formations separately from the whole level; accompany aggregate
  sends and bytes with rates per update and per second. If the best full-pose
  path meets pace through the dense interval, keep it and skip the added logic.

- EARTH-001-POSE-13 [ ] If terrain is already late before actor updates,
  audition preparing the next 256-byte row ahead of time. Keep the current
  row's one-pixel scroll, exposed-row plot, and terrain overlay together;
  do not overwrite a single VDP row buffer before submitting its plot.
  Try moving the foreground music poll and SD refill outside that sequence,
  **not** masking interrupts: the 50 Hz ISR records a tick, it does not send
  music. Check scroll continuity and music timing as well as frame skips.

- EARTH-001-POSE-14 [ ] If routine poses push otherwise on-time frames over
  deadline, compare a separate overdue-first fixture with the unchanged sync
  batch. Process all movement, firing, collisions and timers in their usual
  order. Budget only already-visible formation poses. For the 32 slots,
  try a ready mask, an owed mask and a rotating cursor (nine bytes). Pack
  overdue slots first using their **current** frame and position, not a saved
  pose. Keep first pose/show, hide and death immediate; clear pending bits
  on retirement, reuse and reset. Leave the player and projectiles alone.
  First try 12 visible ships with a budget of six: all 12 must keep updating,
  and each surviving ship must be shown at least every other update. On
  hardware, report the longest gap between pose sends alongside dense-frame
  skips; a better frame counter alone does not establish smoother motion.

### Earlier design (execution order amended above)

Does Earth miss fewer VBlanks if the eZ80 makes fewer MOS calls, or if it
also sends fewer bytes? Compare three ways of publishing the **same poses in
the same update**. Planning only: leave the current timing binaries, runners,
SD files, firmware comparisons and repeat runs untouched. No async pose arm
in this comparison; review the synchronous results before designing one.

| Arm | How an already-visible formation ship gets its pose | Twelve consecutive poses |
| --- | --- | ---: |
| A — current sends | Keep the existing synchronous select-sprite, select-frame, move-absolute calls (4 + 4 + 7 bytes). | 36 sends, 180 bytes |
| B — one synchronous batch | Prefill the fixed bytes of 15-byte records once. Write ID, frame, X and Y directly into their final positions; send the populated prefix once. | 1 send, same 180 bytes |
| C — stock VDP buffered commands | Send one synchronous packet: update a resident state buffer with six bytes per ship (ID, frame, 16-bit X and Y), then CALL a preinstalled program that patches and executes the ordinary pose commands. Movement and collision logic stay on the eZ80. | 1 send, 89 bytes: 11-byte update header + 72 state + 6-byte CALL |

The byte counts model one uninterrupted run of twelve poses, not a whole
frame. Count any extra headers or count-selection bytes in the real result.
A versus B tests fewer MOS calls with identical wire bytes; B versus C trades
wire bytes for ESP32 interpreter work. [Rally's wire census](../../../../AgonArcade/docs/tasks/RALLY-28/standard-vdp/capacity/WIRE-CENSUS.md)
documents the encoding; [its hardware result](../../../../AgonArcade-rally19/docs/tasks/RALLY-19/DELIVERY.md)
is why fewer wire bytes alone do not establish a win.

- EARTH-001-POSE-01 [ ] Pin the comparison inputs after the current series:
  one Earth map and staged file, firmware, input/fire sequence, seed and
  4,544-update interval. Use the same 256-byte terrain uploads, pending-VBlank
  logic, music, drawing and splash lifetime in every arm. Reset the mainboard
  before each measured run; identify each artifact and keep its report separate.

- EARTH-001-POSE-02 [x] Capture A as the reference, including sprite ID,
  frame and position in publication order. Limit batching to uninterrupted
  runs of ordinary formation poses. Flush before any intervening non-pose
  output or presentation boundary. Leave spawn/show, hide/death, missiles,
  sound and HUD on their existing paths; never defer a pose to the next frame.

- EARTH-001-POSE-03 [x] Compare B against A: verify byte-for-byte equality
  of the ordered pose stream, including sparse ranks after kills. Submit
  only occupied records; an empty run sends nothing.

- EARTH-001-POSE-04 [ ] Compare C against that same reference: verify the
  *executed* selects, frames and moves, not equality of the deliberately
  different UART stream. Execute only the submitted records, never stale
  slots after a kill. Check first spawn, full ranks and last disappearance.
  Count resident buffer memory and one-time installation separately.

- EARTH-001-POSE-05 [ ] Report skipped / total VBlanks over equal completed
  updates as the primary outcome, with the dense formation interval broken
  out. Report recurring pose bytes and sends, packet-build time and blocking
  send time. A MOS return is not proof that VDP has rendered a frame. Compare
  differences with measured run-to-run variation; report no clear winner if
  appropriate. Stop for review before any async or production change.
  The isolated [sparse clock observer](pose/RESULTS.md#sparse-attribution-observer-separate-from-the-scored-games)
  is built and passed linked CPU checks; target evidence is still pending.

- EARTH-001-POSE-06 [ ] **Deferred option, not a fourth arm now:** after the
  synchronous comparison, consider a native VDPlet consuming the same compact
  pose records. Compare module on/off on the *same* module-capable firmware;
  production VDPlet already beat stock without running a module, and the
  fastest R30 direct-Set build has no module runtime. Keep async UART and
  [queued music](../MUS-001/TASK.md#proposed-vdp-timed-music) separate.
  No decision to require non-stock firmware for distribution.

## Design history

The Xevious source is 1024×2048. The first two columns were cut into sixteen 256×256 strips, bottom to top in each column, left column first. In Tiled, strip 00 is at the foot of the 16×256-cell level. The assembled background is a locked image layer. The editable Structures layer uses Kenney's CC0 Tiny City pack: 132 original 16×16 tiles repacked sixteen across, converted to Agon64 by nearest RGB without dithering. IDs 0–15 are transparent and reserved for spawn markers. The 135 existing structure placements were shifted with their art and the rendered map checked pixel-for-pixel. Clockwise 90/180/270 variants of IDs 81, 92 and 93 occupy IDs 148–156. Rotations are baked; runtime tiles have no transform flags. Separate Civ terrain studies and picker are documented in MAP-003; they are not another runtime layer.

## Earth terrain streaming and the facility — design only

The playable approach and the Game Over scene will show the **same facility**, not two copies of its backdrop. Smith will lay it out in Tiled across strips **14 and 15** (the top 32 tile rows), framing a 336-scanline view within their combined 512 scanlines. On a successful defense the player flies there normally; on a breach the scene can cut there immediately. The facility's name and destruction choreography remain [open story decisions](../../design/storyline/README.md#stage-1-breach-ending--undecided).

Reserve **three reusable VDP bitmap buffers** for the scrolling terrain. Preload strips 00–02 for the opening; as strips are consumed, reuse those slots for 03–13. Separately preload and **pin strips 14 and 15**: never recycle them in the ring. Normal play uses those pinned buffers when it reaches them; the Game Over scene can use them at any point without an SD read or a duplicate upload.

At 64 KiB per strip, the five buffers hold **320 KiB** of bitmap payload instead of 1 MiB for sixteen strips. That is not a measured peak RAM figure. When implementation is authorized, check refill-time RAM usage, music tempo during reads, and the cut to the pinned scene on hardware. The current game still preloads all sixteen strips; streaming is deferred.

**Current proposal:** [EARTH-001-008–012 — JIT background scanlines](jit-background.md).
Earth now uses ordinary level entry: warp finishes before the first Earth
scanline. The earlier 336-line warp reveal below is historical. This makes
one row per terrain step worth testing before building the three-strip ring.
The two pinned ending strips retain their dual use.

## Approved integration tranche (Nurples)

Framework clarification: Earth City is a campaign level, not a fork, separate executable, or hard-coded level-zero renderer. Preserve the existing Tiled → validated campaign manifest → generated runtime data → ordinary game path. Extend it so a level can select its tileset, ordered background stream, initial spawn and music; existing levels inherit their current defaults. Keep empty cells zero, one-byte runtime cells, no tile transforms, and explicit active-tile mappings. The [Oryx editing workspace](../MAP-001-ORYX/TASK.md), [safe tile moves and active registry](../MAP-002/TASK.md), [terrain curation](../MAP-003/TASK.md), and [enemy spawn work](../AIR-001/TASK.md) are prior stages of the same design workflow, not rival map formats. Retain their original task records.

- EARTH-001-001 [ ] Earth City first, then dg0_00, dg0_03 and the Z80 level with their existing music and appearance. Render Earth terrain 00 through 15 in flight order with the authored structures.

- EARTH-001-002 [ ] Earth initial ship at Tiled cell (6,253); use Smith's replacement ZXArt tune 79463 privately, record provenance; retain stock MOS/VDP compatibility.

- EARTH-001-003 [ ] Verify the campaign, build and stage ordinary nurples.bin for an Oryx FAE review; leave GUI launch to Smith. No production asset redistribution or game implementation commit pending review. Rollback: game/assets at 7bc87f5 and unchanged editable Tiled sources.

## Approved amendment — Earth opening and takeoff

Smith approved implementation, compilation and FAE staging. This is a special opening presentation, not a change to ordinary play, the later level-to-level warp, or mid-level respawns.

- EARTH-001-004 [ ] After the splash, hold the pre-start starfield dead still with the ship hidden. The start key accelerates the starfield from rest to warp. Roughly halfway through the sequence, start scrolling in the numbered Earth background and its corresponding Tiled structures together, bottom row first. Compute the reveal point from the remaining **distance**, not a guessed frame number: the reveal and deceleration must admit exactly 336 Earth scanlines and end at zero speed. If the boundary falls inside a multi-pixel step, split the step. The fast reveal belongs to the opening renderer, never to the gameplay terrain loop.

- EARTH-001-005 [ ] At rest, verify that Tiled rows 235–255 inclusive occupy the 336-pixel playfield, with row 255 fully visible and just about to scroll off the bottom. Only then show the ship, without a swoop, at cell (6,253): playfield-relative top-left (96,288). Use that stopped frame as a visual acceptance checkpoint.

- EARTH-001-006 [ ] From rest, accelerate through fractional scroll speeds to the normal one pixel per update. Keep the fractional distance accumulator (proposed: 8.8) in the takeoff state only. Advance terrain only when a whole scanline is due; do not throttle audio or other frame updates. Hand off at normal speed with the next incoming row (234 at the stop), scanline, and numbered-background phase aligned to actual travel. Verify no repeated or skipped line at handoff.

- EARTH-001-007 [ ] Preserve the regular gameplay hot loop unchanged: no opening-state branch, speed check, accumulator, or extra per-frame work. Select the special handling outside that loop. Verify this by code comparison, and regression-check ordinary scrolling, respawn and later warps.

## Source audit

**R03: shared tile-center anchor and wider Earth formation.** A tile launch
aligns the geometric center of the actual sprite rectangle with the center
of its 16×16 tile, in both axes, once at spawn. Render, motion and collisions
continue to use the resulting top-left. A partially entering air sprite is
hidden until its whole rectangle fits (stock VDP does not clip sprites).
This is shared across levels, unlike the choice of march movement.
Earth's revised front rank spans columns 4–15 on row 232; large ships are
on row 230. Wait 32 terrain ticks before the front rank marches; both ranks
move 64 pixels left and then alternate 64 right/left. This supersedes the
old audition timings below. No VDP firmware change.

Smith accepted R03 on Oryx FAE on 2026-11-23. The reviewed executable
SHA-256 was `3dd9c352f331eebc4c40535e9501a7e1ef860a9e1ec126236ff66ed7fd3be4d5`.

**Earth march audition.** Use the saved eight-actor wave. Earth small and
medium ships wait one terrain row (16 ticks) longer than the large pair
before taking their first horizontal step. Then every actor goes left 96
pixels, right 128, left 128, repeating at one pixel per tick. During the
initial countdown descend one pixel per terrain pixel, including its final
tick. On the first sideways step and thereafter descend at half that rate.
A zero-delay actor starts directly at half speed. With consecutive authored
rows this preserves 16 pixels between their top edges.
Use individual actor timers, not a formation controller. Leave the ordinary
side-to-side pads, eight-slot pool, HP, collision and artwork unchanged.

Staged on Oryx FAE using Smith's latest TMX (first rank row 232, large pair
231). Verified the compiled callback tables and read back the guest binary
(`d8af96cf1152da4146f68fb07dee5cf1eba54569ccb6c82f41125245bf5dcad7`).
A geometry model checked 649 ticks across all eight compiled placements:
first horizontal step on tick 17, reversals at ticks 112 and 240, no
footprint outside the field. This is not a visual pass. Full suite: 235
passed, two skipped, three outstanding checks (two terrain source-label
expectations and byte encoding of dg0.png). Human motion review pending.

**R03 — eight-ship Earth wave (2026-11-23), staged for FAE.** Smith's saved
map puts four small and two medium ships on Tiled row 231, then two large on
row 230. All eight fit without a kill. Verified all 4,096 compiled Earth
cells against the TMX; the only six byte differences from the R02 executable
are in those map cells. No pool, movement, or pad-art change. Read back
`nurples.bin` from the Oryx FAE card (SHA-256
`a9087093808b21fc30f34099a55103f12df129197c6fcc8af74c971ca2901534`).
Visual confirmation is pending.

**Earth air markers, R02 placement review (2026-11-23).** In R01 Smith
saw small ships using the existing side-to-side movement, but no mediums or
larges. The first incoming rank alone requested all eight shared air records.
A marker attempts allocation once, when its row is admitted; a full pool
skips that marker, not a later ship already in flight. This is a Nurples
capacity setting, not an eight-sprite VDP restriction.

R02 uses Smith's revised Oryx TMX: six small markers on row 231, two
medium on 230, two large on 229 (Tiled coordinates; larger Y enters first).
No other executable changes from R01: all 28 changed bytes are Earth map
cells. Verified the three constructor pointers and read back the staged
FAE binary (`9c61d0d4154b4381301a945881e5eebaa6ad7fce9ea2e95a30e43af466020440`).
The eight-slot shared pool and existing motion stay as-is for this review.
Smith will adjust spacing/counts in Tiled. Discuss level-specific movement
after this playtest; neither R02 motion nor visual results are newly qualified.

**FAE acceptance, 2026-11-23:** Smith accepted the staged four-level campaign
after playtest and authorized commit and push. This supersedes the R01 pending
review note below; no hardware test is claimed.

**R01 staged for human review.** Approved amendment frozen on dev at
`04f3189000ba289db3f7e089c64812ff6a0af63d`. Ordinary four-level game;
no forced transition or invulnerability. Static structures are composited
onto the sixteen ordered strips at build time. The timed reveal travels
12×16 + 6×12 + 5×8 + 5×4 + 4×2 + 4×1 = 336 pixels. Regular terrain
instructions remain unchanged: level setup binds the existing ADL CALL
operand, restoring starfield selection after strip 15. Original three map
records compared identical. Full build/check and 240 tests (one skipped);
FAE guest binary and AGNB files read back equal. Private binary SHA-256
`127e7aec53c418c6db5f3f7cf573b13f535eefa2809c19f55649044036fdb87c`.
Staged in held Oryx profile `.emulator/map002-electrode-game-r01`.
Next: Smith to verify the stopped Earth screen, takeoff, normal play,
and real end-of-Earth warp into dg0_00 with ACIDFLSH. No visual pass claimed.

Music decision resolved: Smith replaced 79469 with [BREAK2](https://zxart.ee/tune/79463). The native file identifies itself as a ProTracker 3.1 compilation. Downloaded privately and pinned in MUS-001/samples.json; no format conversion required. Not yet auditioned in Earth City.

Freeze: 14395230405ee8dd8f7c14d6d4069daae8fbe00c. The current compiler validates a single shared tileset, one tile layer, and one base buffer ID for all levels. The current music build stages PT3 only. ZXArt's music API identifies entry 79469 as Spawn's 1997 “'TO INTRO'”, **PT2**, 1:44.85; embedded credit: “TO INTRO” FOR PARADOX BY ALEX. Original download 1,420 bytes, SHA-256 82602748fffaee23da449e545fd4a8e19fbe4eb3e9ad470871e0d8d7e7531b08. No redistribution permission established. Do not feed the raw module to the PT3 player. Music strategy needs a decision before claiming a faithful playable build.
