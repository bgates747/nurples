# Two-vblank game-loop review candidate — 2026-09-16

Author requests30fps at60Hz by waiting two vblanks between frames, intentionally
slowing frame-based gameplay. Cross-project owner: agon-extender BENCH-002.
Added one adjacent call to existing vdu_vblank at the main-loop pacing point;
kept bounded timeout behavior and timestamp/wall-clock timer semantics.
No movement compensation or asset regeneration. Frame-based motion is slower;
not every real-time timer necessarily runs at half speed.

Assembled using `.venv/bin/python build/scripts/build_00_all_the_things.py --assemble`.
Matched binary/symbols generated. Existing dirty cel UI/starfield work preserved,
not included in this commit. No emulator/profile changes or emulator launch.
Hardware deployment and human smoothness assessment are recorded by BENCH-002;
this build alone does not prove realized frame rate or visual smoothness.
