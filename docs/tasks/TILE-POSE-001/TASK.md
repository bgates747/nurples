# TILE-POSE-001 — Unit-defined entry facing

Rollback: `dbf9dcb` on dev. Freeze this contract before implementation.

The fireball turret should scroll into view pointing toward the approaching
player (south), not spend its entrance turning around. North remains angle
zero for artwork and aiming. Configure initial facing by unit type, not a
turret exception in rendering or movement.

1. Declare the fireball turret's entry heading in the active-tile registry.
   Use it to orient its existing authored static tile at build time and seed
   both physical heading and first directional animation frame at creation.
   Both tile constructors must honor it, including on slot reuse. The existing
   tracking initializer should pick up that heading. Other types keep their
   existing defaults, including non-directional animation frame defaults.
2. Preserve bitmap IDs, one-byte maps, map compiler output, source artwork,
   actor layout and the gameplay draw loop. Do not change tracking speed,
   reload, firing policy or game states. No extra bitmap uploads.
3. Check north/south convention, generated tables, both constructor paths,
   packaged pixels and unchanged unrelated art. Run the project tests and a
   full ordinary build. A visual review should watch a turret scroll in from
   its first scanlines through activation, then turn and fire normally.

The frozen plan and pre-change rollback remain independently recoverable.

## Implementation and verification — 2026-11-23

Plan frozen at `ca590d9`. Registry entry `turret_fireball = "south"` drives
both the packed scroll-in tile and the generic lifetime-entry initializer.
Fresh allocations reset stale motion first; in-place transformations only
override facing if the destination type opts in. No new per-actor bytes.

The ordinary full build and deterministic asset check passed. The linked
turret defaults specify 32 orientations: heading 128 gives frame 16, buffer
399. Against the rollback build, all 380 game image IDs and their order match;
only 517 changed (the same authored tile rotated exactly 180°, 92 pixels
different). No change to bundle size, Tiled atlas bytes, or compiled maps.
The legacy atlas build now keeps the original encoding when pixels match.

243 tests ran: 241 passed; the headless joystick test and the Tk display test
skipped for lack of their optional runtimes. Local ordinary executable SHA-256:
`242610a0fbce616f1ffefb44f7019e671820b1b1a5cb4dbebca82ce9c07939d7`.
Smith accepted the Oryx FAE playtest: the barrel scrolled in facing south
and did not snap north when tracking began. The staged executable and matching
game artwork were verified by card readback before the run.
