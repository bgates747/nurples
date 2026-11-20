# ART-001 — Full-frame Mode 20 vector splash review

The current 512×384 splash uses Bayer dithering, which Smith did not like in
review. Make a full-image cel-style alternative using editable vector shapes
and solid Agon colors. Present it next to the existing versions in an offline
browser review. This tranche stops at visual review; it does not change the
application or publish a new splash as production art.

## Execution contract — 2026-11-19

Owner: Purple Nurples. Smith asked to document the Jukebox consultation,
commit and push the contract, then proceed. He rejected a crop-only pilot;
the first candidate covers the whole 4:3 frame. This extends the existing
ART-001 cel UI task ([earlier asset work](../../../devlog/2026-09-10-cel-ui-assets.md)).

- ART-001-001 [x] Consult Jukebox and record the reusable method and its limits.

- ART-001-002 [x] Commit and push the full-frame review contract before any
  candidate generation. Announce the verified freeze commit.

- ART-001-003 [x] Generate editable whole-frame SVG and a native 512×384
  undithered Agon64 PNG; keep an untraced same-source control for comparison.

- ART-001-004 [x] Verify full opacity, exact palette membership, expected
  dimensions, byte-identical repeat output, and an exact round trip through
  canonical RGBA2222 packing and decoding. Inspect full composition and
  compare edges or lost detail with the raster control.

- ART-001-005 [x] Deliver an offline review page with native size and integer
  pixel zoom, editable SVG/downloads, and the existing artwork for comparison.
  Record the observations and leave production selection to Smith.

- ART-001-006 [x] Smith supplied his hand-edited export and requested packing
  it in the current game. The conversion and verification are recorded below.

## Research and method

Jukebox's [consultation reply](/Agon/mystuff/agon-dev-env/cross-agent/agentcoms/messages/2026-11-19_12-14-08_jukebox_nurples-splash-vectorization-methods-reply.md)
and [reproducible full-frame experiment](/Agon/mystuff/AgonJukebox/docs/tasks/SKIN-013/flat-color-full-02/README.md)
used an aligned RGB master and a hand-supplied binary mask. The mask made each
fill independently traceable; it could not recover boundaries between any
touching colors already merged in that mask. Potrace 1.16 used `alphamax=0.55`,
`opttolerance=0.2` source pixels, `turdsize=0`, and the BLACK turn policy.
Paths retained their holes. Color samples used a two-pixel inset and the
median RGB of the most populated 16-level RGB bin, matched to Agon64.
That inset only affected sampling. One per-path Inkscape Simplify at `0.0003`
reduced the pilot outlines; repeated rapid Simplify calls over-smoothed an
earlier candidate. Cairo rendered the simplified paths directly at 512×384
with `CAIRO_ANTIALIAS_NONE`; an exact palette lookup rejected fringe colors.
The [original gallery](/Agon/mystuff/AgonJukebox/docs/tasks/SKIN-013/flat-color-full-02/index.html)
used shared 1×/2×/4× zoom with `image-rendering: pixelated` and linked its
native PNG, SVG, fill table, and diagnostics.

Nurples has no aligned hand-made mask. Build full-resolution masks from the
cel master using non-dithered color labels through canonical `agonutils`;
use color masks rather than a global black/white threshold. An opaque base and
ordered coverage masks can avoid uncovered cracks at independently traced
boundaries. Preserve holes, dark outlines, and small lettering/detail where
the target grid allows them. Use the Jukebox tracing settings as the starting
point; skip optional Inkscape simplification unless it is both available and
shown to improve this art. Retain an untraced flat-color control to show what
the vector pass itself changes. The 320×240 GIMP finish is the artistic
reference, not aligned tooling input; the precise hand-edit technique is
unknown. Use registered 512×384 images for pixel differences. The Bayer
version is a separate source treatment, useful as a visual comparison only.

## Inputs and bounds

| Input | SHA-256 |
| --- | --- |
| `assets/img/orig/ui/splash_bg_single_engine_cel.png` (1448×1086 cel master) | `4ed644cab714b76adae1f2b063f271bbf8049aa70edc261dae4a596d7369effa` |
| `assets/img/orig/ui/splash_bg_single_engine_cel_320x240.xcf` (Author finish) | `562c7db659c24658f442af0ea141b56238623e04a892e2e0d63864a0dee8b414` |
| `assets/img/orig/ui/splash_sprite_faithful_v3_512x384.png` (Bayer review input; currently outside Git) | `a7166277186cc05e1806da6ac7a5b8ffe85597a43f09ac05511dacf7f20d627a` |
| `build/palettes/Agon64.gpl` | `c61dd1476415352e74add70f1ed0c81b1d141361a12b26587a6829ee66ec7f94` |

Jukebox bridge source: `AgonJukebox` commit
`d470d329bb4304a69d2f86fd8152233439b4a12e`,
`docs/tasks/SKIN-013/flat-color-full-02/`. Use the existing Potrace and
Cairo bindings for this offline experiment; do not introduce an application
build dependency. Save a hash-verified copy of the untracked Bayer comparison
with the review artifacts without editing its original. The other source
inputs are recoverable from the freeze commit.

The acceptance target is an inspectable full-frame result, not a claim of
artistic approval. Show the 320×240 reference, Bayer trial, flat raster
control, and vector candidate in the browser at native or integer zoom. No
production image replacement, asset manifest/container edits, runtime or
loading-screen changes, emulator deployment, or hardware work in this
tranche. Leave existing unrelated edits intact. Stop for artwork review.

## Result — 2026-11-19

Contract frozen and verified on `origin/dev` at
`973595e124269284c6a4bffd982cb23e0efdd2a6`. The chosen result is R02:
[offline gallery](../../../.build/art-001/vector-splash-r02/index.html),
[portable gallery ZIP](../../../.build/art-001/nurples-splash-review-r02.zip),
[verification and hashes](../../../.build/art-001/vector-splash-r02/verification.json).
The original Bayer comparison is copied byte-for-byte into the review folder.

The composite uses a 3×3 median filter on the source and canonical Agon64 RGB
labels without dithering. Broad fills are painted first on an opaque base. For
R02 each traced color extends only two source pixels into later masks; holes
are kept in the compound paths. The local overlap reduced the SVG from 29 MiB
(R01 nested masks) to 4 MiB and brought the rendered image slightly closer to
the untraced control. Potrace 1.16 and Cairo 1.18.4 rendered without Inkscape
simplification. The large contours and sharp silhouettes survived; the weakest
details to judge are tiny stars and the thin bands around the ships and planet.
Tracing mostly tidied edges; it did not substitute for hand-selecting colors.

R02 uses 50 exact Agon64 colors. It is opaque, 512×384, and decodes from the
196,608-byte RGBA2222 payload to the identical image. It differs from the
registered untraced control at 3,426 pixels (1.74%). Two independent output
directories compared byte-for-byte, including the SVG, PNG, payload and
verification JSON. Ruff and the Chromium offline gallery smoke check passed.

From the project root, reproduce into a new directory:

```bash
.venv/bin/python docs/tasks/ART-001/vector_splash.py \
  --out .build/art-001/vector-splash-next
```

The gallery tranche did not change any production assets, game code, profiles,
or deployment contents, and did not use an emulator or bench.

## Author export — 2026-11-19

Smith replaced the 512×384 source PNG in place and asked to pack it into the
current game. The source is `assets/img/orig/ui/splash_sprite_faithful_v3_512x384.png`
with SHA-256 `68daa62e731a69d2094c388ecfad0ee54499ac3df988eb98a9f035ce2dfd7558`.
Canonical conversion preserved all 48 colors and every pixel. `tgt/ui.agnb`
changed only buffer 44, the 196,608-byte RGBA2222 splash. Its manifest slot and
dimensions are unchanged. This update did not assemble or deploy the game.

The historical gallery still contains the original Bayer PNG; the old review
generator's Bayer hash intentionally does not match the new game source.

## Mode 20 loading title — 2026-11-19

Smith requested that the old moving “Welcome to the Purple Nurples” logo be
replaced by a fixed **PURPLE NURPLES** title. Its letters use the same exact
Lode Runner glyphs and pixel-art treatment as GAME OVER, rendered at 4×. The
title alternates colors during loading updates, but its position and silhouette
never move. Use the old logo's final upper-left anchor (loader coordinates
`x=0, y=8`) and keep the existing `Loading images: n/total` text. The existing
512×384 hand-edited splash background is not part of the redesign.

- ART-001-007 [x] Generate both native Agon64 title frames from the shared
  glyph geometry and verify that they have the same nontransparent footprint.

- ART-001-008 [x] Draw the splash background once, update the title in place
  only when the color phase changes, and retain every existing image-count
  text update. Keep the current loading and start transitions. No scrolling
  logo, full-screen clears per loaded asset, or new progress-bar behavior.

- ART-001-009 [x] Verify the generated assets and build, and stage a stopped
  Alexe fixture for Smith's Oryx visual review. In the review, the fixed
  title should visibly change colors, the text count should remain legible,
  and the background should not flicker or blank between updates. Keep the
  review changes local pending that result and explicit commit approval.

R01 builds the title as two 464×40 bitmaps at UI IDs 45 and 46. The loader
prints `Loading images: n/359` in the same row and alternates pale-yellow
and magenta title faces after about 32 centiseconds, when the next record
completes. The background and UI records 0–44 are byte-identical to the
pre-title build; so is `game.agnb`. Build, deterministic assets, Ruff and the
full unit run passed (208 passed, one unrelated headless-input test skipped).

Oryx checkout 508 (`e342ca6cb2c3`) held a stopped
`/home/smith/Agon/mystuff/nurples-repair/.emulator/mode20-title-r01` profile
with the pinned Alexe VDPlet v0.1.2-alpha async replacement and Eirene MOS
RC3. The runtime pins and all four guest payloads were verified from the SD
image; the [local staging receipt](../../../.build/art-001/mode20-title-r01/staging-receipt.txt)
records the hashes. Smith reported “nice” and requested the refinements below.
R01 was [released](../../../.build/art-001/mode20-title-r01/release.json) after
a fresh empty-host check, and the next FIFO owner was notified.

## Stacked title and controller-first startup — 2026-11-19

Smith liked R01 and requested two refinements: stack `PURPLE` above `NURPLES`,
and prompt for the controller before entering the pre-start screen. Keep the
4× Lode lettering, pale-yellow/magenta flash, existing upper-left loading
anchor, fixed background, and per-record status text. Reuse the current
keyboard/joystick choice rather than adding a second prompt.

- ART-001-010 [x] Generate the two-line title with matching alpha footprints
  and verify both words remain on screen and clear of the progress text.

- ART-001-011 [x] Route the successful loader into controller configuration
  before pre-start. A valid selection enters pre-start; only a fresh start
  action can begin play. Preserve controller choice across game-over
  continuation and keep the existing escape behavior.

- ART-001-012 [ ] Verify startup order, input-release gating and current assets;
  stage a stopped Alexe review through ordinary Oryx scheduling if available.
  Record R01 handback, R02 results, review instructions and any open gate here.

R02 centers each word within a 240×76 two-line bitmap at the existing
upper-left anchor. The two colors have matching alpha masks and two separated
word rows. Only UI records 45 and 46 changed from R01; the splash background
and gameplay container are byte-identical. After game assets finish loading,
`JOYSTICK (Y) OR KEYBOARD (N)?` replaces the progress line. The existing
release-gated selector enables or disables joystick polling, then enters the
pre-start screen, whose separate gate requires Y/N to be released before a
fresh press. Game Over continuation reuses the selection until the next MOS
`RUN`. Build, deterministic assets, and Ruff passed. The full unit run passed
209 tests with one skipped: the machine has no direct-execution input harness.
The amended native startup fixture assembled; its live key sequence remains
part of the visual review.

R02 is staged and stopped on Oryx under checkout 512 (`8b77f283ed3d`).
The [staging receipt](../../../.build/art-001/mode20-title-r02/staging-receipt.txt)
verifies the pinned runtime and all four guest files read back from the card.
From `/home/smith/Agon/mystuff/nurples-repair/.emulator/mode20-title-r02`,
Smith launches `./flexe-agon-emulator` and types `RUN`. Review the two-line
title, the controller question on the finished splash, and the separate
pre-start press after releasing Y/N. Test both choices across two runs if
convenient. Hold the checkout until Smith reports and closes; no commit or
push before explicit approval.

### R02 playtest reports — record only

- ART-001-013 [x] **Player launch starts too high.** Smith observed on Alexe
  that the ship appears near the top of the screen during the launch phase.
  Expected: the ship swoops in from the bottom and settles at the established
  start point. Reported during the R02 review; cause and affected builds are
  unverified. Investigation and fixes are on hold while Smith reports the
  remaining findings.

- ART-001-014 [x] **Level Complete triggers before the last terrain leaves.**
  Smith reports that the transition starts when the last tile row has fully
  scrolled into view. Expected: ordinary gameplay scrolling continues while
  any of that terrain is visible. Trigger Level Complete only after the last
  tiles have fully scrolled off the bottom edge of the playfield. Recorded
  during the R02 review; investigation and fixes are on hold.

- ART-001-015 [x] **Player laser freezes during the next-level warp.** Smith
  reports that an in-flight player laser stops moving relative to the screen
  during the warp. Expected: the laser continues moving normally on screen
  and runs its ordinary collision checks throughout the warp. Recorded
  during the R02 review; investigation and fixes are on hold.

- ART-001-016 [x] **Seekers do not enter despawn behavior on level transition.**
  Smith observed that active seekers do not enter their existing despawn
  behavior when transitioning to the next level. Expected: trigger that
  behavior on transition entry. Cause is unverified; investigation is on hold.

- ART-001-017 [x] **Keep enemy fireballs moving and collidable during warp.**
  In-flight enemy fireballs should continue their normal movement and
  collision checks throughout the next-level warp. Smith has not observed
  their current behavior, so this is a requirement to verify, not a confirmed
  bug. Investigation is on hold.

- ART-001-018 [x] **Keep the ship under player control during the warp.**
  Revised design: the player retains steering during the next-level warp so
  they can dodge any remaining enemy ships or projectiles. This supersedes
  automatic steering wherever it would take control away during the warp.
  Keep the existing starfield warp animation and speed profile. Implementation
  is on hold.

### R03 corrections — 2026-11-19

Smith ended the report-only hold and requested all recorded fixes plus a new
Alexe build. He confirmed R02 returned cleanly to MOS, then closed Alexe;
[checkout 512 was released](../../../.build/art-001/mode20-title-r02/release.json).

- ART-001-019 [x] Move the fixed two-line title to `(28, 47)` in Mode 20.
  The full screenshot's pasted title matches all 9,164 opaque pixels in
  the generated yellow frame. It is 105 screenshot pixels right and 147
  down from the old title, or 28 and approximately 39 guest pixels.
  Keep both existing bitmaps, the loading text, and controller prompt intact.

- ART-001-020 [ ] Run the staged R03 Alexe review. Verify title placement
  and launch from near the bottom, then play through a level boundary to
  check last-tile timing, ship control, departing seekers, and live projectile
  movement/collisions. Confirm a clean return to MOS and close Alexe before
  Oryx handback. Items 013–019 are implemented in R03, not yet visually
  qualified.

Source findings: the launch reconstructs Y using an 8-bit add (224 + 96
wraps to 64) and loads sprite defaults before a helper that clobbers HL.
The tile loader signals completion when its last source row is admitted;
move the 336-pixel drain ahead of the transition under normal gameplay.
Warp should retain its existing starfield-only speed schedule after that
drain. Use the normal live player/enemy pipeline so lasers/fireballs advance
and collisions apply; send seekers directly into existing retreat movement
with ordinary collisions during warp, without changing harmless retreat for
player death or Game Over. Preserve ordinary death/respawn and Game Over
priority rather than forcing the player alive at the next-level handoff.

R03 implements the fixes above. The canonical launch starts at field-local
Y=320, not Y=64. The last row arms a 336-scroll countdown; a paused world
does not spend it and no subsequent tile definition is read. The final
terrain scroll is still an ordinary combat frame. The warp retains the
existing 300 starfield frames including approach, acceleration, hold, and
deceleration. Death or terminal outcomes continue through the existing actor
lifecycle. The automated victory flight is retained for future use.

Deterministic assets, assembly, Ruff, and whitespace passed. The host suite
ran 211 tests (210 passed; one existing direct-input harness skip). The new
336-scroll terrain boundary case and updated speed/seeker CPU fixtures
assembled; the live behavior still needs ART-001-020. `ui.agnb` and `game.agnb`
match R02 byte for byte; the title reposition only changes its plot coordinates.

R03 was staged and stopped on Oryx under checkout 514 (`3d6cd486118c`), in
`/home/smith/Agon/mystuff/nurples-repair/.emulator/mode20-title-r03`. The
[staging receipt](../../../.build/art-001/mode20-title-r03/staging-receipt.txt)
records the pinned runtime and all four guest files read back from the card.
Smith launches `./flexe-agon-emulator` there and types `RUN` at MOS.
No code, assets, or emulator changes were committed or pushed.
Smith later requested handback. A fresh empty-host inspection preceded the
[release of checkout 514](../../../.build/art-001/mode20-title-r03/release.json);
TRS-80, next in the FIFO, was notified. The R03 profile remains intact.

### R03 playtest findings — launcher cadence and first seeker heading

- ART-001-021 [ ] **Launchers can fill their entire sprite pool in a burst.**
  Smith observed groups of three seekers and four turret fireballs. Fireballs
  also came out four at a time on later firing cycles; repeat seeker bursts
  were not confirmed. Both launchers call their spawn routine while
  `tile_move_timer` is zero. On success, each routine leaves IY pointing to
  the new sprite, and the subsequent reload writes to that sprite's byte
  36 (`sprite_move_timer`) instead of the parent tile's byte 36
  (`tile_move_timer`). The tile remains ready and can fire again on each
  successive update until its pool fills. Once allocation fails, IY remains
  on the tile, allowing a reload and explaining the recurring turret volley.
  Expected: one spawn per countdown expiry, with the pad's 64–95 or turret's
  24–55 tick cooldown written to the parent after either allocation outcome.
  Add a deterministic single-pad/single-turret regression for initial launch,
  repeat launch, and a full pool. Source diagnosis only; no changes deployed.

- ART-001-022 [ ] **A newly launched seeker climbs instead of zig-zagging.**
  The allocator clears heading to zero (straight up), and live seek entry
  arms its turn timer without setting a first heading. With the player below,
  the zig-zag path reuses that heading until the timer expires or the seeker
  hits a field edge. The pad's misdirected cooldown can overwrite the seeker's
  first turn timer too. Expected: a fresh live launch under those conditions
  seeds its first diagonal heading and matching zig-zag step before the
  countdown runs. Preserve later homing, orbit, and retreat behavior. Source
  diagnosis only; no changes deployed.

- ART-001-023 [ ] **Starfield tile gaps persist across game phases.** Smith
  reports gaps during ordinary one-scanline gameplay, acceleration, sustained
  fast scrolling, and deceleration in the R03 Alexe review. Foreground tiles
  make the gap harder to see at normal speed. At the end of level one, Smith
  clearly saw a gap before the last terrain had finished scrolling off and
  before the next-level warp or any fast scrolling began. That observation
  rules out a bug exclusive to the fast-scroll path as the sole explanation.
  The earlier one-scanline overdraw guard addressed the fast-scroll strip;
  it did not establish seam-free behavior in every phase. The gaps were also
  reported in the [integrated Game Over review](../GAME-001JK-INTEGRATED/TASK.md).
  The cause of the current gaps and whether every phase shares it are
  unverified. When investigation is authorized, capture the first discontinuity
  in ordinary one-scanline play and in fast warp, then compare the tile join
  and viewport boundaries through slowdown and phase handoff. Acceptance:
  no exposed line or gap at a tile join in any reported phase. Record only;
  no renderer, artwork, or emulator changes.

- ART-001-024 [ ] **Add a screenshot-friendly pause toggle on `P`.** A fresh
  press pauses or resumes during ordinary play, ship entrance, terrain drain,
  and next-level warp. While paused, leave the displayed frame intact with no
  overlay; freeze scroll, actors, projectiles, collisions, and gameplay timers.
  Keep polling for `P` and Escape. Holding `P` must not toggle repeatedly,
  and resuming must continue the same phase without catching up on elapsed
  real time. Verify pausing on a starfield join at normal speed and during
  warp. Recorded only; no implementation authorized in this tranche.

- ART-001-025 [ ] **Compare starfield gaps in FAE with stock VDP.** Smith
  selected Fab Agon Emulator (FAE), exact Eirene MOS RC3, and stock VDP.
  Do not attempt to port VDPlet for this comparison. Use the same game bytes
  if stock VDP supports their startup, and check normal scrolling and warp;
  changing the VDP makes this a cross-backend comparison, not a test that
  isolates the emulator core. The revised existing-profile query is in
  `2026-11-19_15-10-21_nurples_nurples-fae-rc3-stock-vdp-comparison`.
  Smith paused, then authorized a stopped Oryx fixture with the exact R03
  binary on 2026-11-19. Reuse the matched official FAE 1.2.6 executable and
  native stock Platform VDP module; clone the hash-verified R03 raw card and
  pin the exact RC3 image and map in this profile only. Do not rebuild the
  game or change the Alexe profile or global MOS defaults. Author starts the
  GUI. Flexe's `2026-11-19_15-16-54_flexe_nurples-fae-rc3-stock-vdp-fx304-evidence-one`
  establishes RC3 startup under a related FAE frontend, not Nurples startup
  or Mode 20 behavior under the exact stock pair.

  Staged and stopped on Oryx under checkout 516 (`0da4f418a222`) at
  `/home/smith/Agon/mystuff/nurples-repair/.emulator/mode20-fae-r03`.
  The [official FAE 1.2.6 release](https://github.com/tomm/fab-agon-emulator/releases/tag/1.2.6)
  provided both stock x86-64 files. The exact app, bitmaps, font, and load-only
  autoexec were verified from the cloned raw card, which remains byte-identical
  to Alexe R03 at staging. The RC3 image/map, launcher syntax, runtime hashes,
  shared libraries, and stock VDP printer ABI export all passed. See the
  [staging receipt](../../../.build/art-001/mode20-fae-r03/staging-receipt.txt).
  Smith launches `./fab-agon-emulator` and types `RUN` at MOS; actual startup
  and gameplay observation remain pending. Keep the Alexe fixture untouched
  and release FAE custody after Smith reports and closes.

  Smith's short playtest reached gameplay and reproduced the same bugs he
  observed on Alexe. He reported that FAE ran at full speed while Flexe felt
  around half to one-quarter speed; this is a visual estimate, not a measured
  ratio. Reproduction with identical game, MOS, and card bytes across the two
  VDP implementations makes a Flexe-only cause unlikely for the reproduced
  bugs. The report does not itemize phases or confirm a full warp comparison.
  The FAE card still matched its staged hash after the run. Oryx checkout 516
  was [released](../../../.build/art-001/mode20-fae-r03/release.json) after a
  fresh empty-host inspection; no successor was queued. Both fixtures remain
  intact. No fixes or further runs were started.

### Launcher, seeker, and screenshot pause follow-up — 2026-11-19

Smith approved ART-001-021, ART-001-022, and ART-001-024 and requested a
new stopped FAE fixture for screenshot collection. Keep the stock FAE 1.2.6
VDP and pinned Eirene RC3 combination. Do not change the starfield renderer
or artwork in this tranche. Reuse the matched R03 asset containers unless
the changes need a new asset (none planned).

- ART-001-026 [ ] Build, stage, and visually verify the launcher, seeker and pause updates.
  Stage a new isolated FAE profile after ordinary Oryx checkout and verify
  the game bytes read back from SD. Smith launches and uses `P` to freeze the
  frame for gap captures. Preserve the previous FAE and Alexe controls.

R04 implementation and staging:

- **021 — implemented, playtest pending.** Save and restore the parent tile's
  IY around spawning before writing its usual reload, even with no free actor
  slot. A successful spawn no longer changes either the launcher's cooldown
  destination or the child's own movement timer. Source assertions and a
  countdown model cover first and repeat attempts and rearming at a full
  pool; these are not observations of running eZ80 actors.
- **022 — implemented, playtest pending.** Live launch seeds the first
  down-and-in diagonal, its matching zig-zag step and visible frame before
  arming the first turn. The existing homing, orbit and retreat routes stay
  in place. Verify a launch with the player below the pad on each half.
- **024 — implemented, playtest pending.** The held-key map triggers once
  per fresh `P` press. The pause skips gameplay dispatch, its logical clock
  and telemetry frame advance, but still paces and checks Escape. It draws
  nothing; rebase the raw MOS clock sample on resume to discard paused wall
  time. Verify in normal scroll and warp, hold `P` to check repeat suppression,
  and check Escape from a paused frame.
- **026 — staged, waiting for Smith.** Build preflight, native assembly,
  Ruff, and whitespace checks passed. The full [test log](../../../.build/art-001/mode20-fae-r04/tests.log)
  records 216 tests (one headless-runtime skip); the subsequently added
  countdown model passed in the focused
  [launch test log](../../../.build/art-001/mode20-fae-r04/cooldown-contract.log).
  Both AGNBs match R03; the renderer and starfield artwork were not changed.
  Oryx checkout 518 (request `d453e560a18f`) has the stopped
  `/home/smith/Agon/mystuff/nurples-repair/.emulator/mode20-fae-r04` profile
  with the same official stock FAE 1.2.6 VDP and exact Eirene RC3 as R03.
  [Readback](../../../.build/art-001/mode20-fae-r04/staging-receipt.txt)
  matched the R04 binary (`bce83089…15ca`), unchanged UI/game/font, and
  load-only autoexec. Smith launches `./fab-agon-emulator` from that profile
  and types `RUN` at MOS. No emulator launched during staging. Hold custody
  until Smith finishes and closes the emulator; no commit or push.

### R04 review — starfield join at the handoff to normal play

The broad empty strip below the ship in Smith's [paused FAE frame](/Agon/mystuff/agent-state/codex/attachments/85f85607-192f-4009-9829-dae5571a0530/codex-clipboard-541edee7-e182-4f46-8db2-855f89243ad8.png)
is in the artwork. It does not require a missed redraw to explain. Smith
confirmed pause and both launcher cooldowns; he did not report a separate
verdict on the first seeker turn. Oryx 518 was [released](../../../.build/art-001/mode20-fae-r04/release.json)
after a fresh empty-host check, and TRS-80 was notified.

- Sample the screenshot at native pixel centres: playfield origin (482,214)
  in the PNG, scale 3.75. Matching the stars above and below identifies
  tile 4 over tile 6, with tile 6 starting at playfield row 263. Tile 4
  ends with 17 solid-blue rows; tile 6 starts with five. Playfield rows
  246–267 match the decoded RGBA2222 art at every pixel (5,632/5,632).
  Rows 268–309 immediately below also match (10,752/10,752), so the stars
  pick up at the expected source row. The red striped row near the top is
  separate: level 00 starts with sixteen copies of terrain tile 248.
- The corresponding four PNGs at `a2fd5d7^` and at the first working
  starfield (`173e457`) have stars on both end rows. The replacement set at
  `a2fd5d7` already has today's margins. The slicer deliberately clears four
  pixels at each edge; sparse star placement makes the effective vertical
  gap longer. Over the 16 possible joins the current solid band ranges
  from 10 to 34 rows. The existing artwork test *requires* clear edges but
  does not constrain how many adjacent rows are empty.
- Both scrollers select from the same four contiguous buffers. The earlier
  warp overdraw guard remains present; it addresses exposed cockpit pixels,
  not blue space already in the tiles. This frame does not exclude a second
  rendering fault elsewhere. Recommended next step: keep the approved art,
  add small complete stars near top and bottom, and check the longest empty
  run for all 16 ordered joins. Review-only: no artwork or code changed.

Smith subsequently rejected adjusting the replacement tiles and chose to
restore the original artwork with no code changes. See the
[frozen rollback contract and result](original-starfield-rollback.md);
its visual verification is pending.

### ART-001-027 — Hand-retouched starfield comparison

Smith wants another look at the improved artwork before choosing a production
set. Leave the restored originals as the control and the active game assets.
Work on separate 256×256 copies of the improved pre-rollback tiles. Hand-place
a few small stars near sparse vertical edges, keeping each entire footprint
inside the tile and preserving the other pixels. Show all 16 ordered vertical
joins (including repeats) in a review sheet, with an identically framed
original-art control. Keep the unretouched improved tiles for reference.
Review only: no code, build inputs, deployable assets, or emulator changes.

Review packet ready:
[retouched all-joins sheet](starfield-join-review/retouched-joins.png),
[untouched original control](starfield-join-review/original-joins.png), and
[untouched improved control](starfield-join-review/improved-joins.png).
Each cell is 48 native rows from either tile, with ticks outside the image
at the join. Copies of the source and edited tiles are beside the sheets.
The improved control is the palette-converted artwork from `a2fd5d7`;
the original control is the currently restored, palette-converted artwork.

Fourteen hand-placed stars changed only 56 previously solid-blue pixels.
Every changed pixel is Agon64 opaque and strictly inside its tile. The
full-width solid-blue band at the screenshot's tile 4-over-6 join falls
from 22 native rows to 5. Across all 16 pairs the widest such band falls
from 34 rows to 7. These are static previews only; awaiting Smith's art
selection before touching the game assets.

### ART-001-028 — Retouched tiles in motion on FAE

Smith approved the hand-retouched set and requested a full rebuild and FAE
playtest fixture on Oryx. Preserve the original source tiles, review sheet,
and stopped original FAE profile as controls. Promote the four approved
256×256 previews without resizing or edge edits, update the asset test to
verify the intended pixels, and run the normal full build. Assert that only
the four starfield records changed, and preserve every assembly source file,
the executable, symbols, and UI container. Stage a distinct stopped FAE
profile with the same stock VDP and pinned RC3 as the control. Smith launches
the GUI after card readback; do not change scrolling code or publish the
unreviewed fixture.

Full rebuild passed; 216 tests passed and one input test was skipped for
an unavailable headless runner. Verification against the archived original
game container found precisely four changed payloads (IDs 1024–1027); all
355 other records and all assembly, binary, and UI hashes were unchanged.
The [original source control](starfield-join-review/original/source) still
re-encodes to the original payload hashes. The rebuilt game container is
`6ed01df9…5755`.

Original review checkout 520 was released after fresh host-idle verification;
its profiles were not modified. The retouched FAE profile is stopped at
`/home/smith/Agon/mystuff/nurples-repair/.emulator/retouched-starfield-fae-r01`
under checkout 521 (request `02e800a45529`). Its
[SD readback receipt](../../../.build/art-001/retouched-fae-r01/staging-receipt.txt)
matches the local full build. Same stock FAE 1.2.6/VDP and pinned RC3 as the
control; type `RUN` at MOS (the autoexec only loads). Use the corresponding
`original-starfield-fae-r01` profile for comparison, one emulator at a time.
Hold Oryx until Smith finishes and closes FAE.

Smith accepted the moving retouched starfield on FAE: a close inspection can
still pick out joins, but they no longer stand out during ordinary play.
Accepted for the current build. Checkout 521 was released after a fresh
whole-host inspection found no emulator process; no successor was queued.
Smith authorized committing and pushing the accepted build on `dev`.

### ART-001-029 — Reuse Mode 20 at startup

Smith asked to skip the startup mode switch when MOS is already in the
requested mode. The text and graphical loaders already use the same surface;
the unconditional mode request is before the text loader. Query the live VDP
mode and skip `VDU 22` if it reports Mode 20, while still checking the dimensions
and colour depth. A different mode must still get one acknowledged mode switch.
Reset the loading viewports without relying on a mode switch. Leave the
separate exit-mode diagnostic and restoration unchanged.

- ART-001-029 [x] Verify the matching, different-mode, and failed-query paths
  and assemble locally. No emulator or hardware deployment in this change.

The startup branch now reuses a fresh matching mode report, issues one
acknowledged mode request on a mismatch, and uses the existing display error
path on timeout. Both success paths validate the physical display. Assembly
and source-contract tests passed: 221 tests passed, one input fixture skipped
because its headless runner was unavailable. Both asset containers are
byte-identical to the prior ordinary build. The Pi compile slot was released;
there has been no emulator or hardware test of this change.
