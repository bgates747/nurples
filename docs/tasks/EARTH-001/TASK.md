# EARTH-001 — Earth City opening level

Open the campaign on Earth, defending the city before following the attackers to their worlds. This silo owns the Earth level, from its editable map to Smith's first in-motion review.

**Current render contract:** Earth background strips contain Xevious pixels only.
The ordinary dev build and both aligned fixtures draw the Kenney Structures
layer with the regular tile renderer. Spawn markers are independent even when
placed over a structure. The original baked-strip description below is historical.

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
