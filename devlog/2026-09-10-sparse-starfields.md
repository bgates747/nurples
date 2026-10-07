# Sparse scrolling starfield atlas

ART-002 follows the Author's request to replace the dense backgrounds, retain
a few planets, compose one atlas and slice it into square tiles. Original PNG
replacement was explicitly authorized; prior versions remain in Git.

The built-in image generator produced `tiles/bg/starfield_cel_master.png` and
its adjacent prompt records the design. `nurples_tools/starfields.py` scales
the square master to 512x512 and slices it in reading order into the existing
256x256 names: `bg_stars_0`, `bg_stars_3`, `bg_stars_4`, `bg_stars_6`.
The result has three small planets and sparse small stars, with fewer than
2% nonblack pixels in each palette-converted tile.

Inspection of `bg_plot` in `src/asm/tiles.inc` established that runtime selects
the next bitmap randomly. `init_bg` starts with 0 above 4. Consequently,
continuous atlas slicing alone cannot guarantee later adjacencies. The master
places objects away from tile divisions, and the preparation stage enforces
a four-pixel opaque-black border on every side of every tile. All tile pairs,
including self-repetition, can meet without color seams or clipped objects.

The new standalone `build/scripts/build_starfields.py` supports build/check.
Both current asset entry points invoke the stage before packing `game.agnb`.
Canonical agonutils owns Agon64/RGB conversion without dithering, RGBA2222
encoding and preview decoding. Original slices and processed previews retain
their paths under `tiles/bg`; runtime payload names and IDs remain unchanged.

Validation:

1. Full asset build, deterministic checks, assembly, Ruff and whitespace
   checks passed. All 82 unit tests passed. New tests cover quadrant order,
   square input, density, opacity, palette channels and all 16 seam pairs.
2. Decoded 256x256 previews were visually inspected. Independent AGNB parsing
   confirms that only background IDs 1024 through 1027 changed in `game.agnb`;
   every record retains its dimensions, format and ID. Sprites and tiles are
   byte-identical to HEAD. The executable remains byte-identical to HEAD.
3. The canonical deployer updated `nurples-runtime` in `.emulator/nurples`.
   All four deployed files were compared with `tgt` and matched. Existing
   autoexec was preserved. The profile-local generated launcher started the
   emulator successfully with Wayland and the intended SD root; it remains
   open for the Author's review. Log: `.emulator/nurples/starfield-review.log`.

New `game.agnb` SHA256:
`fa9c7e88967f9d8e9df8ccc9c7e2fd9e3c2caef3ce3e4acf327870a79405e3f7`.
The cel UI assets from ART-001 are preserved. Visual runtime acceptance remains
pending; no commit, push or hardware deployment was performed.

## Density revision

The Author requested approximately three times the star density with the same
planet density, then accepted the displayed revised design. The built-in
image transformer increased the stars and retained three planets; a refinement
reduced an initially excessive increase. The current master is the accepted
refined image; `tiles/bg/starfield_density_revision.prompt.txt` records both
prompts. The earlier container hash above belongs to the first sparse version.

Rebuilt the four slices and game container, passed all 82 tests and deterministic
build checks, redeployed the runtime bundle and relaunched the profile-local
emulator. All deployed files matched the build. ART-002 still tracks runtime
visual acceptance; the source-image approval does not authorize a commit.

## Dark-blue background and separate star-density adjustment

The Author next requested approximately double the small stars and triple
the larger sparkle stars, preserving the planets and restoring the original
dark blue. Sampling all four original PNGs on `main` found the dominant
background to be RGB(0,0,85). The built-in image transformer revised the master
accordingly, retaining three planets. The prompt is recorded in
`tiles/bg/starfield_blue_revision.prompt.txt`.

Tile borders now use that exact opaque dark blue instead of black. The seam
test checks the matching blue on all 16 ordered tile pairs; its occupancy
limit now allows up to 6% foreground for the requested increased density.
Decoded Agon64 previews were inspected. Full build, deterministic checks,
all 82 tests and Ruff passed. The runtime bundle was deployed, all deployed
bytes verified against `tgt`, and the profile-local emulator relaunched for
review. No commit or push was made.
