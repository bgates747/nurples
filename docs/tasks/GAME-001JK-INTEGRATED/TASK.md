# GAME-001JK-INTEGRATED — Game Over visual test in the full game

Smith approved implementation. Three full-game review bundles are built locally. They use the regular game background and assets so Smith can judge overlay flicker in context. Target execution remains open.

## Proposed scope

The standalone Olimex test rendered the title and prompt over moving test bands. Smith reported flicker during their visible periods. That observation leaves the full-game rendering behavior unverified. The four buffered-condition restore checks are covered separately by the [standalone fixture task](../GAME-001JK-OLIMEX/TASK.md).

Use the same splash, loading, start choices, level initialization, ship entrance, renderer, and assets as the current game. The hook belongs at the gameplay coordinator's first handoff to player input. The current code starts input in `player_grace`, after the spawn animation. In the test build, bypass that first call to player input, put the player actor in the terminal state expected by Game Over, and use the existing coordinator transition. Game Over owns player and projectile cleanup.

Smith selected a frozen Game Over backdrop. Hold the starfield and terrain at their entry positions, stop new seeker launches, and let seekers already on screen enter harmless retreat immediately. Reuse the player-death retreat flight and docking logic. Choose the nearest active, visible launchpad. If there is no usable pad, or the selected pad becomes invalid, head toward the nearer horizontal playfield edge and release the actor before its sprite would cross it, per GAME-001B-EDGE below. Keep the orbit routine available for other states. The hardware overlays and non-firing turret search continue to animate.

Keep a scrolling comparison run at the ordinary one-scanline pace. Both arms use the same entry point, level, assets, overlays, launch suppression and retreat policy. Only scrolling differs, so the comparison can isolate whether scrolling contributes to visible-period flicker.

Build the hook only into a separately named review binary and bundle. Keep the ordinary build output, startup files, and shared firmware unchanged. The forced transition may repeat after a new game so Smith can compare two cycles from one launch. The existing continue and exit controls remain available.

## Review and acceptance

- JK-INT-001 [x] Smith approved the existing first level and the two trigger policies for implementation. Use the first input frame for the initial run and the first published visible seeker in live play for both comparison arms.

- JK-INT-002 [x] Assemble the isolated binaries and verify the ordinary binary is byte-identical to a build from the same source with review settings disabled. Record executable and asset hashes. Source and host checks passed. Runtime state order still needs the target check in JK-INT-004.

- JK-INT-003 [x] Commission and stage the immediate-frozen arm on production Olimex VDPlet v0.1.3-alpha. Both later comparison arms remain local until Smith finishes the first run.

- JK-INT-004 [ ] Smith observes ordinary startup and spawn, automatic Game Over, and the animated title and blinking prompt over the real backdrop. Verify that the intended arm freezes scenery and stops new launches, and that existing seekers dock or disappear before crossing the playfield boundary. Run the scrolling comparison from the same scene if needed to separate rendering problems from motion. Verify continuation and clean exit to MOS.

- JK-INT-005 [ ] Fix unchecked display mode negotiation exposed by the immediate-frozen run. Avoid a redundant playfield MODE request when the required surface is already active. Verify mode, dimensions, and color count from a fresh VDP response before drawing. Reject unexpected mode/depth with a readable report and normal MOS exit. Rebuild the immediate arm, commission a fresh packet, and request a new Olimex slot. The actual fallback mode and cause of the original failure remain open until target evidence confirms them.

The immediate trigger may freeze a starfield before terrain and pads are visible. For the later arms, start from a fresh launch, select the same configuration, and leave controls neutral so the deterministic launch sequence reaches the same scene. The trigger waits for a live player and a published seeker that overlaps the playfield.

## Build evidence

The [review builder](../../../build/scripts/build_game_over_review.py) changes review settings only in a private source copy. It builds and compares the ordinary binary with both settings disabled. The checkout's built `tgt/nurples.bin` and assets were left in place.

Bundles in `.build/game-over-integrated-review/` each contain `nurples.bin`, the unchanged game and UI containers, the required font, and a verified SHA256SUMS file.

| Arm | Trigger | SHA-256 of binary |
| --- | --- | --- |
| `immediate-frozen` | First input frame | `b149fca4c780dad21d05329862eca0231a0cc985508bc5059ef6ce3f485aef49` |
| `seekers-frozen` | First published seeker visible in live play | `ae38dfdf07ca613753e4b99cf6c70068ef5ebba17b09dc60c09dad9a54318d18` |
| `seekers-scrolling` | Same seeker gate, one scanline per Game Over update | `d36a5026b4a28dd81126c59c192acf55cc510f88c604ae113d12c80bd355ecf6` |

Pi compiler generation 989 assembled the three variants, both ordinary builds, and the extended seeker fixture. The unit suite ran 205 tests with one skip. Ruff and Git whitespace checks passed. The native seeker fixture was assembled only. No emulator or hardware session ran in this tranche. The compiler slot was released and its next operator notified.

Smith requested Olimex staging for the immediate arm alone. Extender commissioned the [immediate-frozen packet](/Agon/mystuff/agon-extender/agents/nurples-go-immediate/PACKET.md), and preflight passed. The scheduler replied “Added to the FIFO.” Await slot-open before staging. Both seeker comparison arms remain local for a later run.

The scheduler opened the slot. The [joined stage receipts](/Agon/mystuff/agon-extender/agents/nurples-go-immediate/stage) verify four successful PARLIO uploads. The listener returned control and the keyboard is ready. Smith received the launch commands in an immediate alert. Bench custody remains with Nurples pending the visual verdict.

## Immediate-frozen result

Visual FAIL on production Olimex. Smith reported an apparent mode change and a mangled display. The phone clip shows persistent yellow corruption across the backdrop and bezel art while the title alternates and the prompt blinks. The clip starts after Game Over appears, so it does not locate the first corrupted frame. Escape returned to usable MOS.

Source trace shows intentional loader mode 8 and single-buffered 512x384 playfield mode 20. Playfield setup reissues mode 20 after the start choice. There is no explicit mode command on the immediate trigger or Game Over entry/update path. The clip is compatible with mode 20 and supplies no evidence of a fallback. Compare the same build before and after Game Over entry with actual mode readback and overlay visibility isolated before interpreting the seeker comparisons. Those arms remain unrun.

Smith identified the yellow screen as the color-depth symptom of a fallback. The exact v0.1.3-alpha source confirms that a failed MODE request first retries the previous mode, then falls back to mode 1 if that also fails. Mode 1 is single-buffered and uses fewer colors than mode 20 in either legacy-mode setting. Nurples does not currently verify the negotiated display before drawing 64-color assets. The recording contains no mode packet, so the actual selected mode and first failing request remain unverified. For the next diagnostic, collect fresh reported mode, pixel dimensions and color count after each playfield MODE request and again on Game Over entry. Treat a lower-depth result as a display setup failure before judging overlay behavior.

Smith requested handback before changes. R01 handback joined with exit 0, working Legacy CLI, idle transfers, listener offline, and serial handles free. The normal scheduler check-in was delivered. [Handback receipt](/Agon/mystuff/agon-extender/agents/nurples-go-immediate/handback/handback.json).

## Display fix and R02

Nurples now requests 512x384, 64-color mode 20 before allocating resident assets and keeps the surface through loading, attract, play, and Game Over. The loader artwork is drawn in this same surface. Redraw resets viewports and colors in place. Validation reads a fresh MOS reply at loading, attract entry, game start, and Game Over entry. A distinct VDP reply fences queued viewport responses before each mode query. A mismatch or timeout returns to MOS and prints the failed checkpoint; reaching Game Over prints its observed mode on exit from a review build.

Pi build generation 1010 assembled the ordinary binary and all three R02 review arms. The suite ran 206 tests with one skip; Ruff and whitespace passed. Immediate R02 at `.build/game-over-display-r02/immediate-frozen/` is 46,754 bytes, SHA-256 `de537b5804c38dfa8d273faed24cef1d62eef6858269b312cbbb6b81175f4920`. All four payload hashes verify. Fresh visual commissioning requested through `2026-11-18_18-41-52_nurples_nurples-integrated-display-r02-commission`. Actual video and mode readback remain an Olimex gate.

The [commissioned R02 packet](/Agon/mystuff/agon-extender/agents/nurples-go-r02/PACKET.md) passed host preflight. The scheduler replied “Added to the FIFO.” Wait for slot-open before staging.

The scheduler opened the slot. R02 is staged on the verified production stack with four successful PARLIO transfers and a joined stage worker. The listener exited and keyboard is ready. Smith received the launch commands and mode readback criteria. [Stage receipts](/Agon/mystuff/agon-extender/agents/nurples-go-r02/stage).

R02 returned `Playfield display failed at check 1: no VDP mode reply` before assets loaded. Smith saw no visible mode switch. The code reaches its error message after issuing MODE 20, but its probe first waits for a distinct color reply and can quit before it ever sends the mode query. The same message covers failure at either reply boundary. There is no target proof yet that MODE 20 was acted on or that the VDP fell back. The next diagnostic should distinguish the MODE command's own acknowledgement, the reply fence, and the explicit mode query, with a bounded interval that allows time for the mode change.

## R03 acknowledgement retest

Smith authorized testing under the current Olimex reservation. R03 replaces the color fence with the VDP cursor reply. At startup that reply drains the earlier font-selection notification. The next mode packet is the MODE 20 command's acknowledgement. Later surface checks use fresh explicit mode queries. A bounded CPU wait remains valid if VBlank stops during allocation. Each of the three reply timeouts now has a distinct MOS message.

The immediate-frozen R03 binary is `.build/game-over-display-r03/immediate-frozen/nurples.bin`, 46,936 bytes, SHA-256 `09349c26d30b56d13c2303ed1f76452726a24a71b70e3a7a200a082f2e14a6eb`. Its four payload checksums passed. Pi compiler generation 1014 assembled the normal and review builds and ran 207 tests with one skip. Ruff and whitespace checks passed. The compiler slot was released. Extender was asked to commission the same-slot retest with a fresh SD namespace and evidence. R02 evidence remains intact.

R03 reached full-color Game Over over the real frozen starfield. Smith approved the pixel-scaled title treatment and reported slight scanout errors plus starfield gaps in IMG_2781.MP4. Overall visual qualification remains open; the numerical mode exit line was not supplied. The existing fast-scroll clipping guard remains in the R03 source. The clip begins with the field already frozen, so it does not establish when the reported gaps first appeared. Retain the entrance and first frozen frame as the next seam comparison. The two later seeker arms remain unrun.

For the frozen arm, consider plotting the fixed-footprint artwork as ordinary bitmaps and restoring the prompt rectangle from the frozen background when it blinks off. Retain software sprites as another candidate. At Smith's request, asked Rally for exact matrix recoloring commands, limits and existing evidence in `2026-11-18_19-54-56_nurples_nurples-bitmap-colour-matrix-guidance`. No renderer change in this review step.

Rally's return, `2026-11-18_19-57-44_rally_nurples-bitmap-colour-rally-source-return`, identifies stock buffered command 72 (`&48`, Expand Bitmap) with options `&10` as the lookup operation. It maps each packed RGBA2222 byte through a 256-byte table into a separate output buffer; register that output as bitmap format 1. The current two 15,360-byte Game Over frames have identical binary alpha coverage and differ by exactly three substitutions from frame 0 to frame 1: `DF→F3`, `E2→D1`, `F3→CB`. The identity table with these three replacements reproduces frame 1 byte for byte. This saves 15,104 payload bytes if the full table is uploaded, while both registered variants still occupy their own VDP buffers. The start and continue prompts have different masks and remain separate artwork. Advice and byte comparison only; no renderer change yet.

Olimex handback first timed out with the game still active. After Smith returned to MOS and authorized a retry, the retry verified a working Legacy prompt, offline listener, idle transfers, and free serial handles. The normal check-in was delivered. [Operator evidence](/Agon/mystuff/agon-extender/agents/nurples-go-r03/OPERATOR-LOG.md).

Smith requested the authored 512x384 splash on Alexe first. The stopped Oryx review profile is `/home/smith/Agon/mystuff/nurples-repair/.emulator/mode20-splash-r01` under checkout 501, request `9bf5feb27b26`. The card readback matched the ordinary R03 game binary and updated UI container with the authored dither; SHA-256 of card `849f23aceb122be7987eb2ef1949b9a1b913213b40e9df72e037a861a891ad71`. The wrapper and runtime manifests verify against published VDPlet v0.1.3-alpha BIN/ELF and pinned Eirene RC3 MOS. Autoexec loads the game and leaves invocation to Smith. Review and release are pending.

Smith reviewed the splash and preferred the older flatter cel finish to the Bayer treatment. On November 19 the fresh whole-host inspection showed no emulator processes, Oryx checkout 501 was released, and TRS-80 request `74f81499306b` was notified.

## Next complete-game gate — last life to clean restart

The entrance swoop and grace period, fireball presentation, accepted
death effects, launcher cooldowns, pause, and retouched starfield now
have Author observations. The current build includes moving attract,
death/respawn, seeker retreat, turret tracking/search, live warp, and the
recovered Z80 campaign level. Their presence is not a full acceptance run.
Top/side-edge death, mid-level respawn with score and map progress kept,
combat during hyperwarp, and the recovered level's hazards and crater
transforms still need focused coverage. New enemy patterns, attract replay,
and the Xevious mode are later work.

- JK-INT-006 [ ] **FAE first:** rebuild the seeker-present / frozen review
  arm against current `dev` and the accepted assets; preserve the ordinary
  binary. Use pinned RC3 and stock VDP in a fresh Oryx profile under
  `nurples/.emulator`. Enter via the normal start and ship entrance; trigger
  Game Over only with a seeker already visible. Verify that terrain stops,
  no new seekers are launched, turrets search without firing, and seekers
  dock or disappear without crossing the playfield boundary. Report the
  existing boundary defect separately if reviewing the baseline before its
  fix. The title and prompt must be readable while visible. Held input must
  not dismiss the screen; a fresh press must start a clean second game
  without asset reload, stale actors, timers, or overlay resources. Repeat
  once in the same run, then exit cleanly to MOS. The forced trigger does
  not qualify the actual last-life transition; cover that once in the
  ordinary build as well.

- JK-INT-007 [ ] **Then production Olimex:** repeat the same seeker-present
  frozen scene and verify the display remains at the required mode and
  color depth. Check and record whether the slight scanout errors seen in
  R03 recur, distinguishing intentional blinking from corruption while
  visible. If needed, run the matched one-scanline scrolling arm as a
  discriminator. Establish that result before choosing a different renderer.

## Seeker boundary bug report

Smith reports that seekers overlap the bezel or lower cockpit while
leaving the playfield. Stock VDP does not provide playfield clipping for
these sprites. A graphics viewport cannot be relied on to hide their
overshoot. The current edge-retreat handler in `seeker_retreat.inc` checks
against the physical screen, not the playfield, and can keep publishing
the sprite after it has entered the bezel.

The visibility contract is now **blink out before any pixel crosses the
playfield boundary**, regardless of the seeker state. Judge the proposed
rendered footprint using the playfield bounds and actual sprite dimensions,
in one coordinate space. An edge-retreat seeker disappears and releases
its slot on the first step that would cross; it does not fly behind the
bezel or wait until its entire image is off the physical screen. Docking
at an on-screen pad is unchanged, and harmless versus live collision
policy remains as before within the field.

Acceptance: on stock VDP, a fully contained seeker can still draw flush
with a legal edge. A one-pixel or multi-pixel crossing must never publish an
out-of-bounds frame. Check left and right retreat, plus top/bottom and
corners wherever a seeker can approach them; look specifically at the
lower cockpit artwork. Exercise ordinary play, death/Game Over and warp,
and confirm a despawned slot can be reused without a ghost or bounce.
Documentation only: no fix, build, or reservation in this tranche.

### Edge fix and frozen FAE candidate R01

Smith authorized implementation and staging, with human review later.
The seeker footprint now uses the same integer pixels the VDP receives.
The retreat path deactivates before collision handling or position
publication if the next frame would cross the playfield. The shared
seeker publication check also covers first show from a pad and other
movement states; an ordinary clamped/bounced pose is still legal if its
entire footprint fits. The review trigger now requires a published,
fully contained seeker. No starfield or terrain scrolling code changed.

Native assembly and asset preflight passed. The ordinary binary from the
review builder matched the ordinary build. The suite ran 218 tests, one
headless-input skip; the expanded eZ80 seeker fixture assembled but was not
executed. It covers legal flush and fractional positions, both retreat
directions, top/bottom/corners, and no out-of-field collision during warp.

Staged only `seekers-frozen` at
`/home/smith/Agon/mystuff/nurples/.emulator/game-over-seeker-edge-fae-r01`.
The [SD readback](../../../.build/game-over-seeker-edge-fae-r01/staging-receipt.txt)
matches binary `5c3b4418…fcab` and the current retouched assets under stock
FAE 1.2.6/VDP and pinned Eirene RC3. The old Oryx profiles were untouched.
Staging checkout 525 was released after confirming FAE was stopped; no
successor was queued. Obtain a fresh ordinary Oryx checkout when Smith is
ready. He starts the profile wrapper and types `RUN` at MOS, chooses his
controller, and can leave controls neutral for the first visible seeker.
Visual review and the separate ordinary-build last-life run remain open.

November 20: Smith confirmed Oryx availability and requested the run.
Human review checkout 526 (request `66a03a80ed0d`) is held after a fresh
idle inspection. [Readiness verification](../../../.build/game-over-seeker-edge-fae-r01/human-ready-verification.txt)
matched the staged card, guest executable, stock FAE/VDP and pinned RC3;
CPU boost is off. FAE is stopped for Smith to launch. Release after Smith
reports and closes the emulator.

### Forced edge-exit arm — JK-INT-008

Smith observed the seeker dock immediately after its launch in the frozen
R01 fixture. That exercises docking, not the playfield boundary. Add a
separately named seeker-present frozen arm for stock FAE. On Game Over entry,
send the visible seeker toward the nearer horizontal playfield edge even
though a valid pad exists. Use the production edge-retreat movement and
hide/release rule. Keep the original docking arm, scroll comparison, and
ordinary game behavior unchanged; verify the ordinary build byte for byte.
Success is a visible retreat followed by disappearance before any sprite
pixel crosses onto the bezel. Keep the clean-restart check in the same run.

Built and staged `seekers-frozen-edge` under stock FAE 1.2.6/VDP and pinned
RC3. The ordinary binary and all three earlier review arms are byte-identical
to their pre-change builds. The suite ran 219 tests (one headless-input skip).
Checkout 526 was released after Smith's docking observation and a fresh
empty-host check; no successor was then queued. Checkout 527
(`08cb247beca5`) now holds the stopped forced-edge review. The
[SD readback](../../../.build/game-over-forced-edge-fae-r01/staging-receipt.txt)
verified guest binary `180a722b…0700`, retained assets, and load-only startup.
Smith launches on Oryx from
`/home/smith/Agon/mystuff/nurples/.emulator/game-over-forced-edge-fae-r01`,
types `RUN`, chooses the controller, and leaves controls neutral.

- JK-INT-008 [x] **FAE forced-edge visual PASS — November 20.** Smith saw
  the seeker blink out before it touched the bezel. Continuing from Game
  Over started a second game without reloading assets. Smith confirmed
  FAE was closed. This passes the forced horizontal retreat and
  resident-assets restart checks; the ordinary last-life path and the
  Olimex scanout comparison remain open.

Oryx checkout 527 was [released](../../../.build/game-over-forced-edge-fae-r01/release.json)
after a fresh whole-host inspection found no emulator processes. No
successor was queued.

### Ordinary playthrough — JK-INT-009

Smith approved integrating the tested edge boundary into the ordinary
app, retaining normal pad docking, and requested a full playthrough on
Oryx. Build the real executable with all three review settings zero.
Normal Game Over and warp prefer the nearest usable pad; the death retreat
retains launch-pad identity. Fall back to the nearer horizontal playfield
edge only without a usable destination. Do not force a transition or
edge retreat. Stage the ordinary binary and accepted assets in a fresh
stock FAE/RC3 profile. Keep both prior diagnostic profiles intact.

Full build and deterministic asset check passed; 218 tests passed and one
headless-input test was skipped. The deployable executable (`33551113…0c2b`)
matches the isolated ordinary control, not the forced-edge fixture.
Oryx checkout 528 (`09d291a72cab`) holds the stopped production-app
profile at
`/home/smith/Agon/mystuff/nurples/.emulator/game-over-ordinary-fae-r01`.
The [SD readback](../../../.build/game-over-ordinary-fae-r01/staging-receipt.txt)
matched the ordinary executable, accepted assets, and load-only startup.
The previous profiles were not touched. Smith runs from MOS and plays
normally: cover mid-level death/respawn, live warp if reached, and the
ordinary last-life → Game Over → fresh-key restart without asset reload.
No commit/push or Olimex run. Hold review custody until FAE closes.

- JK-INT-009 [x] **Ordinary FAE playthrough PASS — November 20.** Smith
  reported that Game Over triggered normally and a fresh keypress started
  a new game without reloading assets. This was the ordinary build with
  all review overrides disabled. This closes the normal Game Over
  trigger and resident-assets restart gate on FAE; the production Olimex
  scanout comparison remains open.

Smith confirmed FAE was closed. Oryx checkout 528 was
[released](../../../.build/game-over-ordinary-fae-r01/release.json) after
a fresh whole-host check found no emulator processes. No successor was
queued.

### GAME-001B — Immediate seeker retreat on player death

Smith clarified that retreat starts when the player begins dying, not after
the old two-orbit hold. Take the existing launch-pad-or-nearer-edge route
immediately. Preserve an already-running retreat, including during warp.
Pause new pad launches until the player is fully alive again; an existing
death retreat must not turn back into pursuit when grace ends. No new orbit
behavior, visual effect, or scrolling changes. The orbit controller remains
available for other phases. Game Over keeps its nearest-usable-pad policy.

Implemented on `dev`. Death publishes the player phase before dispatching
the actor-owned retreat; grace resumes pursuit only for orbiters, not
retreaters. A death during warp preserves the current exit route but pause
of pursuit includes collision checks while the player is not fully alive.
The complete ordinary build, isolated review builds and asset check passed.
The expanded eZ80 fixture assembled; the eZ80 cases have not yet been
executed. The suite ran 221 tests with one headless-input skip. Pi compiler
checkout 1067 was released. Ordinary binary `c457b77b…9d59`; no emulator
staging, visual sign-off, commit, or push in this tranche.

Smith later requested an ordinary FAE review. Checkout 529
(`8cff65fb8814`) holds the stopped `seeker-death-retreat-fae-r01` profile
under the canonical Oryx `nurples/.emulator` root. The
[SD readback](../../../.build/seeker-death-retreat-r01/staging-receipt.txt)
matched the ordinary binary, retouched assets, and load-only startup.
All review overrides are disabled. In live play, lose a ship with a
seeker visible and check that it immediately heads for its pad or an edge
before respawn, with no holding orbit or new launches during death.
Await Smith's review and emulator shutdown before releasing.

- **FAE visual PASS — November 20.** In the ordinary build, Smith saw
  seekers head for their launch pads as soon as the player died, with no
  holding orbit. This confirms immediate pad retreat in live gameplay.
  The forced-edge fixture has separate bezel clearance evidence; this
  playtest did not separately establish the missing-pad fallback or
  launch cooldown through respawn.

Smith confirmed FAE was closed. Checkout 529 was
[released](../../../.build/seeker-death-retreat-r01/release.json) after a
fresh whole-host idle check. No successor was queued.
