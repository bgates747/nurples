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
