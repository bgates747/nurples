# TURRET-CLEAR-001 — Loaded versus cleared to fire

Baseline: `7295819`. Reload continues while the live turret tracks. At
zero the turret stays loaded until the rate limiter reaches this tick's
computed heading, not just the matching artwork frame. Fire from the physical
barrel. Restart the usual 24–55 tick interval only on a successful launch;
a full projectile pool leaves the turret loaded. Search and seeker pads are
unchanged. Assemble, run regression tests and stage the ordinary game in the
existing Oryx FAE profile for visual review.

R01 assembled and staged, with executable and game artwork verified by FAT
readback. 242 tests passed, two optional tests skipped. Visual acceptance pending.
