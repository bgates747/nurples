# Boss encounters

**Proposed, not implemented.** Damage should change a boss’s silhouette while the fight is still in progress. Use individually drawn sprite modules so one part can change its bitmap, extend, retract or break without replacing the whole boss.

A controller could own formation and attack phases. Each targetable module could use the existing actor framework for health, damage frames and movement relative to the hull. Its hitbox should follow its visible position. Ideally, shooting off a gun silences that gun: the missing piece tells the player where it is now safer to fly.

## Where bosses might appear

In the [proposed campaign](../../storyline/README.md), stage 3’s outer defenses could introduce a gatekeeper. Stage 5 could use a stationary planetary installation, and stage 8 a mobile boss on the final approach to Earth. These are candidates, not a requirement for every stage to end in a boss.

## First prototype

One hull, two independently destructible arms. Check whether losing either arm visibly changes the silhouette and attack, whether hits register while an arm moves, and whether overlapping sprites scan out cleanly on hardware. Sprite budget, victory conditions and final placements remain open.
