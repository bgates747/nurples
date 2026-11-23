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
