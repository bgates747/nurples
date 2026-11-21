# GAME-001I-LEAD — Turret overlead diagnostic

Resolved on `dev`: Smith accepted R04 with the ordinary turret clamp and
reload active. Its fireball movement correction is in the ordinary build;
normal damage is enabled and the diagnostic trace is not included.

Smith saw excess lead when his ship was level with or above a turret, and
better aim below it. The game currently calculates a lead heading every live
frame but fires along the physical, rate-limited barrel. It does not alternate
between programmed lead and straight-at-player shots. The temporary build
fires the targeting computer's vector directly and makes hits harmless to
the ship, so Smith can separate the two aiming paths during a longer test.

Use `dev` at `bf1970a` as the ordinary control. Keep tracking, barrel artwork,
reload cadence, projectile speed, seeker behavior, and the targeting maths
unchanged. In an isolated source copy, refresh the direct-bearings input and
run `targeting_computer` immediately before every actual turret launch.
Use its `Heading_p`, `Vp_x`, and `Vp_y` for the shot and its artwork. The
barrel may visually lag the shot in the diagnostic; that is intentional.
Skip only the shield deduction in `update_shields`. Keep hit detection and
normal impact responses, including fireball/seeker destruction; lightning
remains persistent by its normal rules. No change to the published build.

- GAME-001I-LEAD-001 [x] Assemble both source copies; verify the control
  matches published `dev` and the diagnostic source touches only the two
  selected boundaries. Keep the identical assets and `nurples.bin` name.
- GAME-001I-LEAD-002 [x] Superseded by the traced, rate-limited R04 passes
  below (no separate R01-only visual result). Original comparison: on FAE,
  compare steady lateral passes below, level
  with and above an active turret, in both directions. A shot or seeker hit
  should still register and destroy the actor without moving the shield bar.
  Record whether the excess lead persists with the physical barrel out of
  the firing path. Do not change the production turret or maths on this result
  without a separate decision.

## Build and FAE staging

The isolated ordinary build matched `bf1970a` byte for byte. The diagnostic
source differs only in `tile_turret_fireball.inc` and `player_shields.inc`;
the emitted code was checked for direct targeting before spawn and for an
unchanged-shields return. No tracked assembly or deployable target files were
changed. The normal test suite passed 225 tests with one headless-input skip.

Diagnostic `nurples.bin` SHA-256:
`2f5d4de897463d15a160c5eeab89be6ff6a7056b7f3941733744dc2b7d34f977`.
FAE is stopped on Oryx at
`/home/smith/Agon/mystuff/nurples/.emulator/turret-lead-review-r01` with
stock VDP and pinned Eirene RC3. Readback matched the executable, symbols,
both ordinary containers, and font. Checkout 557 (`d1419ce0b456`) is held
for Smith's launch. The guest autoexec loads the diagnostic `nurples.bin`
under the normal name; type `RUN`. No hardware card was changed.

### R02 — Snap the visible barrel during live aiming

Smith also asked the diagnostic barrel to snap immediately to the targeting
computer's current angle. Bypass the slew only at the live aiming update;
still wrap the display heading to one turn. Leave the non-firing search,
direct-lead shot, and collision invulnerability as in R01. Replace the
diagnostic `nurples.bin` under the same name in the held FAE profile;
retain R01 as an off-path control and leave production untouched.

R02 assembled and passed emitted-code checks for the live slew bypass,
unchanged non-firing search, direct lead and unchanged shields. Its sole
source delta from R01 is `tile_turret_fireball.inc`. Verified FAE readback
of `nurples.bin` (`7aed37a0…75973b`) and unchanged assets and autoexec.
R01's card remains byte-identical at `lead-review-r02/control-card.img`.
Oryx checkout 557 remains active; start the same profile fresh and type
`RUN` at MOS. No game or hardware run by Nurples.

**R02 visual PASS.** Smith reported that the apparent overlead was gone
with the snapped barrel and direct-lead shots. He suggested the ordinary
limiter might carry a full step past the aim point. The source instead
clamps the final step in both directions, including at wrap, and returns
carry when the desired angle is reached. The ordinary firing decision uses
only the reload timer; it can shoot along the physical barrel before that
barrel reaches the current lead solution. This is a candidate explanation,
not a measured fault trace. R01 had no separate reported result, and R02
changes both barrel motion and launch heading relative to production.
No production code or timing changed on the strength of this result.

### R03 — Trace the ordinary firing path

Smith asked to restore the normal rate limiter and reload-based firing in
the private diagnostic, keep the ship collision-invulnerable, and capture
the ideal, pre-slew, post-slew, and actual shot headings. Take one sample per
successfully allocated fireball. Include the direct bearing, ship and turret
positions, ship velocity, and actual fireball velocity for sign and geometry
checks. Keep logging in RAM during play and save a bounded binary trace on
normal Escape to MOS; copy it from the FAE card after closing the emulator.
No change to the production firing policy or math. Keep the normal game
filename on the same held FAE profile.

R03 assembled with the normal barrel and reload calls verified in the
binary. It records the first 512 actual fireball launches and counts any
further launches without writing beyond the trace RAM. The decoder passed
synthetic wrapped-angle, signed-velocity and incomplete-file checks. Escape
prints `Turret trace saved: TURRLOG.BIN` on a full write; the file is the
latest completed run in the guest game directory. No guest execution was
done by Nurples, so the actual save/readback is pending Smith's run.

R03 `nurples.bin` (`af7f8f67…f7a03d`) and unchanged assets verified on the
existing stopped FAE profile and checkout 557. R02's exact card is archived
as `lead-review-r03/control-card.img` within that profile. Start fresh and
type `RUN`, make steady crossing passes both above and below some turrets,
then Escape to MOS and close FAE for retrieval. No changes to tracked game
sources, normal release assets, or the Console8 card.

### R03 result — aligned shots can still overlead

Smith completed several passes and returned cleanly to MOS. The raw
[trace](../../../.build/turret-lead-review-r03/author-TURRLOG.BIN) and
[decoded shots](../../../.build/turret-lead-review-r03/author-shots.csv)
contain 182 successful launches, zero dropped; the on-card executable
matched the staged R03 SHA-256. Of 182 shots, 139 reached the requested
heading and 43 fired while turning. In all cases the limiter obeyed the
2-angle256-unit step or snapped exactly to an aim within one step; no step
passed the current aim. Every shot heading matched the post-limit barrel.

Frame 3139 is a resolved counterexample to barrel overshoot. The ship was
90 pixels left of the turret, three pixels above it, moving right at
3 px/frame with no vertical input. The barrel stepped from 203.410 to
204.707 angle256 units, exactly the desired and fired angle. The installed
shot velocity was (-3.8125, -1.15625) px/frame. An exact constant-velocity
intercept at the game's 4 px/frame projectile speed yields 194.376 if the
ship keeps its observed screen-space velocity, and 204.702 if it is treated
as also moving up by 1 px/frame. The latter matches the logged desired
angle. At the observed velocities' horizontal crossing (ignoring discrete
frame and launch-offset details), the shot would be roughly 12 pixels
above the ship's recorded Y. This is a model comparison, not an observed
post-launch collision or proof the player held the input after the shot.

`targeting_computer` subtracts the terrain's 1 px/frame vertical scroll
from the target's Y velocity. `enemy_fireball` starts from the turret's
screen position and then advances by its own screen-space velocity; it does
not inherit ground scroll. Using a turret-relative target velocity with a
screen-space projectile explains the bias in the aligned example. Other
out-of-alignment shots remain a distinct consequence of the reload policy.
Only 15 launches sampled lateral input, so this is evidence for a specific
failure mode, not a full map of the quadrant or proof about the earlier
R02 visual run. Next proposed comparison: remove only that scroll offset in
another private build; preserve the limiter, reload, speed and logging.

Oryx checkout 557 released after the trace was retrieved and a fresh
whole-host inspection found no emulator. TRS-80 request `5cd2c56de9c6`
was notified. No production code changed.

### R04 — Keep ground-relative aim; convert the shot to screen motion

Smith selected the paired conversion rather than removing the target's
scroll correction. Frame 4288 is the stationary control: player and turret
were level, the barrel was on aim, and the shot rose 41 px over its 164 px
crossing. With the same barrel vector plus ordinary ground travel it would
pass 1.3 px below the ship instead. Across 134 stationary-at-launch aligned
shots, median projected closest approach was 19.3 px versus 1.4 px with that
conversion. These are straight-line projections, not collision observations.

Use an isolated copy of R03. Add actual terrain displacement to the fireball
at the muzzle and each subsequent movement tick. No displacement on a
stationary terrain frame; never borrow the decorative warp speed. Keep the
computed aim, physical heading, tracking rate, reload, invulnerability,
collision handling and trace format. The trace must record the effective
screen velocity at launch. Stage a separate stopped FAE profile for the same
passes without overwriting R03 evidence or changing the production binary.

R04 assembled and staged; human result pending. Emitted-code checks passed
for signed 24-bit addition, muzzle integration after the velocity trace, and
per-tick recomputation from immutable ground velocity before motion/collision.
The only displacement writes reset the logical frame and publish the actual
terrain scroll. R03's normal firing, invulnerability and trace checks passed.
FAE card readback matched `nurples.bin` (`e12f220c…908218`) and the
unchanged containers and font; there was no pre-existing guest trace.
Oryx checkout 560, request `46dd61edd2f6`, is held for Smith's launch in
`.emulator/turret-lead-review-r04`. Escape saves `TURRLOG.BIN` for retrieval.

### Repeat-run attribution — R03 control, not an R04 result

After Smith reported the trace save and closed FAE, the R04 card still
matched its complete prelaunch hash and contained no trace. The earlier
profile (`turret-lead-review-r01`, which houses the R03 executable) held a
new 185-shot trace, zero drops. Its guest executable matched R03
(`af7f8f67…f7a03d`); this is a second control run. Saved the
[raw repeat](../../../.build/turret-lead-review-r04/repeat-r03-TURRLOG.BIN)
and [decoded shots](../../../.build/turret-lead-review-r04/repeat-r03-shots.csv)
separately; raw SHA-256 `02af556c5dbf8f9436dd27cf0eddafc2aad7ceff554ed6c81404135a9ee2f4ef`.

Restricting to zero player screen velocity at launch and a barrel already
aligned after slew gives 134 shots in the original and 117 in the repeat.
Median projected closest approach to the stationary player was 19.3 versus
27.8 px; median horizontal separation was 80 versus 110 px. Frame 2222
alone projects 40.7 px high at X crossing from 158 px away with the ship
three pixels above the turret. The model assumes continued stillness and
does not measure collisions. R04 remains staged, **untested**. For a rerun,
verify the R04 profile card and binary before launch and use the profile
path explicitly. Checkout 560 released after retrieval; TRS-80 notified.

### R04 accepted — integrate the fireball correction

Smith reported the problem solved on the verified R04 card and approved
ordinary-game integration provided the turret rate clamp was active. On
checkout 563 the guest executable matched R04; the saved trace contains 64
shots, none dropped. All fired along the post-limit barrel heading and
21 fired before alignment. No logged firing update turned more than two
angle256 units. FAE was closed, evidence retrieved, and Oryx released.

Bring over only the signed ground-to-screen fireball movement at spawn and
in flight. Build the ordinary binary with damage enabled, no trace or test
hooks, and the same targeting, limiter and reload.

Ordinary `tgt/nurples.bin` SHA-256:
`aee644014c23927beaa5f3f84b82dd7a5e4f61be3da929cc7fc209084449cf1d`.
Assembled with matching symbols. Checked the emitted R04 and ordinary
tracking and firing calls, both use the same 2/256-turn cap, plus verified
ordinary shield writes and absence of trace symbols. Host suite: 226 passed,
one skipped. Added regression coverage for muzzle/flight conversion and
the per-frame terrain displacement. No production deployment in this step.
