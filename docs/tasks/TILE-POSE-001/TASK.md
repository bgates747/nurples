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
