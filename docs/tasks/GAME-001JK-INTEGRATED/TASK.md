# GAME-001JK-INTEGRATED — Game Over visual test in the full game

Smith approved implementation. Three full-game review bundles are built locally. They use the regular game background and assets so Smith can judge overlay flicker in context. Target execution remains open.

## Proposed scope

The standalone Olimex test rendered the title and prompt over moving test bands. Smith reported flicker during their visible periods. That observation leaves the full-game rendering behavior unverified. The four buffered-condition restore checks are covered separately by the [standalone fixture task](../GAME-001JK-OLIMEX/TASK.md).

Use the same splash, loading, start choices, level initialization, ship entrance, renderer, and assets as the current game. The hook belongs at the gameplay coordinator's first handoff to player input. The current code starts input in `player_grace`, after the spawn animation. In the test build, bypass that first call to player input, put the player actor in the terminal state expected by Game Over, and use the existing coordinator transition. Game Over owns player and projectile cleanup.

Smith selected a frozen Game Over backdrop. Hold the starfield and terrain at their entry positions, stop new seeker launches, and let seekers already on screen enter harmless retreat immediately. Reuse the player-death retreat flight and docking logic. Choose the nearest active, visible launchpad. If there is no usable pad, or the selected pad becomes invalid, fly out through the nearer horizontal edge and release the actor only after its entire sprite leaves the visible screen. Sprite positions here use physical screen coordinates. Keep the orbit routine available for other states. The hardware overlays and non-firing turret search continue to animate.

Keep a scrolling comparison run at the ordinary one-scanline pace. Both arms use the same entry point, level, assets, overlays, launch suppression and retreat policy. Only scrolling differs, so the comparison can isolate whether scrolling contributes to visible-period flicker.

Build the hook only into a separately named review binary and bundle. Keep the ordinary build output, startup files, and shared firmware unchanged. The forced transition may repeat after a new game so Smith can compare two cycles from one launch. The existing continue and exit controls remain available.

## Review and acceptance

- JK-INT-001 [x] Smith approved the existing first level and the two trigger policies for implementation. Use the first input frame for the initial run and the first published visible seeker in live play for both comparison arms.

- JK-INT-002 [x] Assemble the isolated binaries and verify the ordinary binary is byte-identical to a build from the same source with review settings disabled. Record executable and asset hashes. Source and host checks passed. Runtime state order still needs the target check in JK-INT-004.

- JK-INT-003 [x] Commission and stage the immediate-frozen arm on production Olimex VDPlet v0.1.3-alpha. Both later comparison arms remain local until Smith finishes the first run.

- JK-INT-004 [ ] Smith observes ordinary startup and spawn, automatic Game Over, and the animated title and blinking prompt over the real backdrop. Verify that the intended arm freezes scenery, stops new launches, and lets existing seekers dock or exit completely. Run the scrolling comparison from the same scene and record separately whether either overlay flickers during its visible periods. Verify continuation and clean exit to MOS.

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
