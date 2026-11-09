# Palette-native overlay art

The prospective game-over and attract overlays are drawn from integer vector
geometry in `nurples_tools/overlay_art.py`. The generator writes both the
tracked SVG masters under `assets/vector/overlays/` and exact-size transparent
PNG previews under `assets/img/proc/overlays/`:

- `game_over_0` and `game_over_1`: 240x64 alternating colour frames;
- `press_any_key_to_continue`: 160x16 game-over prompt;
- `press_any_key_to_start`: 160x16 attract-mode prompt.

Every visible channel value is one of 0, 85, 170 or 255 and alpha is binary.
The drawings use no external fonts, antialiasing, resampling, palette reduction
or image transformer. SVG and PNG therefore share the same source geometry and
the PNG files are already valid Agon64 artwork.

Regenerate or verify them from the project environment:

```bash
.venv/bin/python build/scripts/build_overlay_art.py
.venv/bin/python build/scripts/build_overlay_art.py --check
.venv/bin/python -m unittest tests.test_overlay_art -v
```

These are presentation masters, not runtime assets yet. They intentionally have
no buffer IDs, RGBA2222 payloads, sprite upload commands or firmware claim.
`GAME-001J` requires the exact hardware-sprite VDP API, ownership rules and
cleanup behavior to be frozen and qualified before those integration choices
are made. Keeping art preparation on this side of that gate avoids baking an
unproven ABI into the asset catalog.
