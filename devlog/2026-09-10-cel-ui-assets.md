# Cel splash, logo and bezel runtime preparation

ART-001 implements the Author's requested runtime replacement of four accepted
art designs. `build/ui_assets.json` selects the left cel bezel, corrected
right bezel v2, single-engine cel splash and cel logo draft. It explicitly
records the established 46 UI IDs and dimensions, preventing additional
original-art PNG variants from changing runtime ordering.

`build/scripts/build_ui_art.py` provides build/check entry points over
`nurples_tools/ui_art.py`. Both existing current-asset build entry points call
the preparation stage through `write_current_assets`; their checks also
compare regenerated payloads and previews against the selected sources.
Pillow scales the images; canonical agonutils performs nearest RGB Agon64
conversion without dithering, RGBA2222 encoding and preview decoding.
The logo's painted neutral checkerboard is removed with a source-specific
magenta chroma mask before scaling and binary-alpha thresholding. Details
and commands are in `docs/python-tooling.md`.

Validation:

1. `build_00_all_the_things.py --all` and the standalone current-assets check
   passed. All 79 unit tests passed, including alpha cleanup, resampling,
   retained dimensions/IDs and decoded palette constraints. Pip check, the
   canonical agonutils smoke test, Ruff and whitespace checks passed.
2. Independent AGNB parsing compared the old/new container: only IDs 6, 7, 44
   and 45 changed payloads. Every record's ID, dimensions and format remain
   identical. UI container length remains 246,844 bytes.
3. `nurples.bin`, its symbols, `game.agnb` and assembly metadata remain
   byte-identical to HEAD. Decoded previews of all four replacements were
   inspected; the logo has actual binary transparency.

The existing `.emulator/nurples` profile is now the deployment manifest's
default, replacing its stale external profile path. The canonical deployer
installed the four-file `nurples-runtime` bundle there, preserving autoexec.
The profile-local generated `./fab-agon-emulator` wrapper launched successfully
with Wayland and the correct SD root; its window was left open for review.
The profile's existing autoexec loads and runs the game. Launch output is in
the ignored `.emulator/nurples/cel-art-review.log`.

Runtime hashes:

1. `ui.agnb`: `655b3476bea1304b4ca8ea486d25e7e1e53cb8bdd9846a4139555d4b9947717b`
2. `nurples.bin`: `63c89d2676f6645631b2240f23002b8b68d45069979a4804ca7d14f34e720088`
3. `game.agnb`: `b5e27f3f36ee9f25fe69fb35b3294c1268af1610f53327386fac6ae0055af823`

Author visual acceptance of the converted runtime art remains pending under
ART-001. No commit, push or hardware deployment was performed.
