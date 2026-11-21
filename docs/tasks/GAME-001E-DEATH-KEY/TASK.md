# GAME-001E-DEATH-KEY — On-demand death and respawn review

Give Smith a full-game FAE fixture in which collisions still register but
cannot damage the player. A fresh D press kills the live ship through the
normal death path. This lets Smith choose the exact terrain and ship position
at death and review the existing safe-respawn behavior. Production stays unchanged.

- E-DKEY-01 [x] Freeze this plan and all pending work on dev, including the
  Console8 notes. Use the accepted turret fix as the ordinary binary control.
- E-DKEY-02 [x] Build a reproducible, isolated fixture using the ordinary
  assets, campaign, shields-return semantics from the turret diagnostic,
  and the real kill_player entry. No fake lives, teleports, timer changes,
  or targeting trace. D is edge-triggered in live normal play only. A held
  or ineligible press must not kill again after pause, death, grace or restart.
  P, Escape, and ordinary controller choice remain available.
- E-DKEY-03 [x] Verify fixture controls, build identity, unchanged production
  output, and readback of a new stopped R01 FAE profile on Oryx (RC3 MOS,
  stock VDP, normal guest name nurples.bin). Obtain the ordinary visual slot;
  Smith launches. Keep the implementation uncommitted pending review.
- E-DKEY-04 [ ] Human review: take a hit without shield loss, earn points,
  approach a playfield edge, and press D when a ground hazard obstructs the
  centre respawn lane. Look for one death/life loss, frozen terrain, inward
  bounce, retained score and map progress, a safe endpoint and blinking grace
  before normal play. Try holding D through respawn and releasing/repressing.
  A centred respawn on clear terrain is a control, not proof of the blocked
  case. Last-life D should use ordinary Game Over. Record observed results only.

## R01 ready for human review

Contract and pre-existing changes frozen on origin/dev at
`8778aaf1d14c59c4f3a31324743de5c16c58e62a` before implementation.
The builder `build/scripts/build_respawn_death_review.py` checks an ordinary
assembly against `tgt/nurples.bin`, then interposes the input and shield
entries only in a temporary source copy. The 227-test host suite completed
with one skip. Emitted-code checks verified MOS D (bit 2, byte 6), both state
guards, the fresh-edge gate, pause carry, ordinary death entry and the same
no-damage return as the turret fixture. No turret tracing or real SD writes.

Held Oryx generation 565, request `ea0eb683dd6e`, for Smith to launch
FAE in `.emulator/respawn-death-review-r01`. Card readback matched
`nurples.bin` SHA-256 `b89944de12e49c7639cd5ca53214f1ab6e2faddfee51b58d7d67eb6a07b537c5`
and the unchanged game/UI/font assets. No human gameplay verdict yet.

## R01 observation — explosion particles at the playfield edge

Smith saw particles appear to bounce and requested the seeker's despawn rule.
Use the full rendered bitmap footprint on first show and after movement;
never clamp or reflect a particle. Preserve the dying ship's intentional
inward bounce, particle velocities, animation cadence and other sprite rules.
Prepare the updated D-key fixture separately; leave the live R01 untouched.

R02 staged (not visually qualified). The old callback called the reflecting
mover and then tested its edge flag; it did not use the seeker's pre-publish
footprint rule. The explosion record also declared 7×7 while all five
uploaded frames are 16×16. The revised callback integrates signed velocity
without reflection and deactivates on the first step whose raster would
extend outside the field. The publication guard now covers explosions too,
including their first show. Ship movement and the in-bounds effect are unchanged.

Ordinary binary `b596aec8…c8ff9`; isolated fixture `9678dd67…c6fe2`.
Host suite: 226 passed, one skipped. Verified assembled movement paths in
both builds and read back the new stopped `.emulator/respawn-death-review-r02`
card on Oryx. R01 and its checkout were not changed. Follow ordinary FIFO
after R01 closes before launching R02.

Smith closed R01. After fresh idle inspection, checkout 565 was released
with no successor waiting. R02 now holds checkout 566, request
`895c683bfb9f`. The executable read directly from the profile card matched
the staged R02 hash; load-only startup, runtime and MOS manifests verified.
FAE is stopped for Smith's edge review.

## R02 edge review passed — production integration

Smith confirmed the explosion edge behavior on R02 and approved promotion.
The ordinary `tgt/nurples.bin` contains the same footprint check and
despawn path, with normal shield damage and no D-key hook. The fixture
builder is retained separately for repeatable death/respawn tests.
This pass covers the edge behavior; it does not close the off-centre
safe-respawn scenario in E-DKEY-04. Oryx checkout 566 was released after
FAE closed and the next owner was notified.
