# GAME-001E-DEATH-KEY — On-demand death and respawn review

Give Smith a full-game FAE fixture in which collisions still register but
cannot damage the player. A fresh D press kills the live ship through the
normal death path. This lets Smith choose the exact terrain and ship position
at death and review the existing safe-respawn behavior. Production stays unchanged.

- E-DKEY-01 [ ] Freeze this plan and all pending work on dev, including the
  Console8 notes. Use the accepted turret fix as the ordinary binary control.
- E-DKEY-02 [ ] Build a reproducible, isolated fixture using the ordinary
  assets, campaign, shields-return semantics from the turret diagnostic,
  and the real kill_player entry. No fake lives, teleports, timer changes,
  or targeting trace. D is edge-triggered in live normal play only. A held
  or ineligible press must not kill again after pause, death, grace or restart.
  P, Escape, and ordinary controller choice remain available.
- E-DKEY-03 [ ] Verify fixture controls, build identity, unchanged production
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
