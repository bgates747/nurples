# AIR-001 — Side-to-side air enemies

Freeze the movement contract before implementation. Use the existing actor
record and top-left playfield coordinates. Constructor input BC: B = signed
4.4 horizontal velocity, C = signed 4.4 vertical velocity, pixels per movement
tick. Expand once to the ordinary signed 16.8 velocity fields. The generic
initializer honors both signs. The small, medium and large pad constructors
choose horizontal direction once, away from the nearer side using the
sprite center; choose left on an exact tie. Preserve vertical sign and speed.

Bounce on the inclusive playfield (not screen) sides using actual sprite
width: the rightmost legal top-left X is right + 1 − width. Clamp on contact
and point inward. Zero velocity stays zero. Never reverse vertical velocity.
Advance until the last raster row leaves the top or bottom; with unclipped
sprites suppress drawing on partially visible vertical frames so no pixels
bleed into the panel. No additional ground-scroll displacement.

Qualify signed fractional velocities, both sides, widths 16/32/28, starts
at edges, midpoint choice, zero speed and both vertical exits. Scaffold
initialization without enabling pad launches, choosing production speeds,
allocating sprite IDs or changing seekers. Discuss parameters before wiring
the three pad types to live spawn timers. No deployment requested here.

## Implementation evidence

Contract frozen at 96a1bfa. The post-allocation constructors and pure motion
step are in `src/asm/side_to_side.inc`. An isolated ez80asm 2.1 build on Oryx
ran against its eZ80 CPU core: 177,696 actor snapshots matched the independent
raster/velocity oracle in `tests/side_to_side_cpu.rs`, using
`tests/asm/side_to_side_fixture.asm`. This is CPU evidence, not a visual test.

For integration, consume the step's visible/hidden/exited result both on spawn
and after moving. Do not send partial footprints through the legacy enemy-air
publication path: it deactivates them instead of temporarily hiding them.
Use allocator-owned records and vetted move dispatch once the new pools and
behavior are connected. No test velocity is a production default.

## First playtest profile

Speeds are pixels per game tick relative to the screen. Player horizontal
speed is 3; ordinary terrain scroll is 1 downward. Do not add scroll twice.

| Air enemy | Horizontal magnitude | Downward speed | Laser hits to destroy |
| --- | ---: | ---: | ---: |
| Small | 3.5 (proposed interpretation of “slightly faster”) | 4 | 1 |
| Medium | 3 | 2 | 2 |
| Large | 1.5 | 0.5 | 3 |

Each pad placement launches once, when its whole tile has scrolled in. No
reload timer or repeat launch on death/respawn. The pad is inert terrain
after launch: neither ship nor weapon collides with it; it cannot be
destroyed. This does not change the existing seeker pad. The airborne enemies
are ramming hazards, not shooters. A laser hit counts once; only a killing
hit destroys the enemy. Keep hit points in the individual actor record.

## Live campaign candidate

The ordinary game now uses the Oryx design copy of dg0_00 (three small,
two medium and three large markers), then dg0_03, then nurples_z80_main.
The test level reuses Terminator; the two existing maps retain their music.
Activation comes from completed row admission, not a timer or destructible
pad record. Eight pooled air records share the six resident A1/A2 images.
air_hp and air_visibility use the first two actor-owned bytes; contact
damage to the ship remains in the existing shared header. Nonlethal laser
contact consumes the shot but clears the enemy's pending-death bit.

Full asset rebuild/check and production assembly passed; 237 tests passed,
one skipped. In-game launch appearance, hit counts and campaign handoff
await playtest.

R02: after the first FAE audition, keep horizontal speeds and HP unchanged.
Set screen-relative down speeds to small 1.5, medium 1.25, large 13/16
(0.8125, nearest signed 4.4 to 0.8). Refresh dg0_00 from Smith's saved
Oryx level-design checkout and regenerate the campaign before assembly.

R03 — rightmost electrode regression. Keep the one-hit-per-laser rule.
Bound the beam at launch using its own width; preserve muzzle X during
vertical flight. Laser collisions require raster spans to overlap on both
axes; ship contact keeps the old touching-edge rule. The Oryx eZ80 CPU
fixture exercised 162 combinations of final X, fractional Y, edge contact,
overlap, and competing targets. Human review: shoot the outer electrodes
in dg0_03 (campaign level two). This candidate also includes the ACIDFLSH
first-level music assignment.
