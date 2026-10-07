# Python tooling specification

## Supported environment

Purple Nurples supports Python 3.14 through the repository-local `.venv` only.
`requirements.txt` pins runtime tools; `requirements-dev.txt` adds audit tools.
The build backend is declared in `pyproject.toml`.

The canonical native extension is installed editable from:

```text
/home/smith/Agon/mystuff/agon-utils
```

It must resolve from the repository root to that checkout's compiled extension.
An `agonutils/` directory in this repository is forbidden because it masks the
editable extension as a namespace package.

## Image-processing boundary

Canonical `agonutils` owns:

1. palette conversion;
2. PNG-to-RGBA2222 encoding;
3. RGBA2222/RGBA8888 decoding to PNG; and
4. shared AGM, RLE, and SIMZ behavior.

`nurples_tools.images` owns only consumer adapters:

1. temporary PNG bridging for callers holding Pillow image objects;
2. returning decoded canonical output as a Pillow image; and
3. raw RGBA8888 serialization, which `agonutils` 1.1 does not export.

RGBA2222 adapters always use the checked-in `build/palettes/Agon16.gpl` or
`Agon64.gpl`. Non-dithered legacy packing maps through Agon64 with method `RGB`
before canonical encoding; this preserves nearest-level thresholds. Golden
tests cover RGB values 42/43 and 212/213 plus alpha values 0/128/255.

## Invocation contract

Invoke scripts from the repository root with `.venv/bin/python`. The editable
`nurples-tools` install makes `nurples_tools` imports work for scripts located
below `build/` and `tiles/` without `sys.path` manipulation.

All reusable script code must be safe to import. Generators, GUI display,
downloads, recursive cleanup, deployment, and emulator launch belong behind an
explicit CLI entry point.

The current game build has four explicit actions:

```bash
.venv/bin/python build/scripts/build_00_all_the_things.py --assets
.venv/bin/python build/scripts/build_00_all_the_things.py --check
.venv/bin/python build/scripts/build_00_all_the_things.py --assemble
.venv/bin/python build/scripts/build_00_all_the_things.py --all
```

`--assets` first prepares the four accepted cel UI images and four starfield
tiles, then builds the one
runtime font plus `ui.agnb` and `game.agnb` from the RGBA2222 payloads under
`assets/rgba2`. `--check` checks the source-to-payload conversion, decoded PNG
previews, generated metadata and every packed payload without writing. The
orchestrator does not deploy files or launch an emulator.

### Cel UI artwork

`build/ui_assets.json` explicitly fixes all 46 UI buffer IDs and runtime
dimensions. Additional PNG variants in the original-art directory cannot
enter the runtime catalog or shift IDs. Its four `prepare` entries select
`bezel_l_cel.png`, `bezel_r_cel_v2.png`,
`splash_bg_single_engine_cel.png`, and `splash_logo_cel_draft.png`.

Prepare these images independently with:

```bash
.venv/bin/python build/scripts/build_ui_art.py
.venv/bin/python build/scripts/build_ui_art.py --check
```

This stage uses Pillow Lanczos scaling to the existing 128x304 bezel,
320x240 splash and 160x106 logo dimensions. Canonical `agonutils` performs
nearest-RGB Agon64 conversion without dithering and RGBA2222 encoding through
the local adapters. The payload names stay `bezel_l`, `bezel_r`, `splash_bg`
and `splash_logo`; decoded previews replace their PNGs in `assets/img/proc/ui`.
Both current asset-build entry points invoke this stage before packing AGNB.
The standalone preparation command does not repack the containers.

The logo draft has a painted gray checkerboard. Its source-specific alpha mask
keeps pixels where both red and blue exceed green by more than 20, preserving
purple ink and pink highlights while removing neutral gray. It is applied
before premultiplied-alpha scaling; alpha is then thresholded at 128 for a
clean opaque/transparent overlay. This mask is specific to this draft, not a
general background-removal or palette-conversion algorithm. A future logo
with neutral ink must supply its own alpha and disable this manifest flag.

### Sparse scrolling backgrounds

`tiles/bg/starfield_cel_master.png` is the single source atlas. The background
stage scales it once to 512x512, then slices a 2x2 grid in reading order into
the existing `bg_stars_0`, `bg_stars_3`, `bg_stars_4`, `bg_stars_6` names.
These remain 256x256 images at IDs 1024 through 1027. The game starts with
0 above 4, but later selects tiles randomly; each tile therefore has a uniform
four-pixel opaque dark-blue border on all sides to support every adjacency and
self-repetition. Objects are composed away from the atlas's dividing lines.
The background and borders use RGB(0,0,85), sampled from the original tiles
on `main`. The current design increases small stars and larger sparkles while
retaining the same three planets.

```bash
.venv/bin/python build/scripts/build_starfields.py
.venv/bin/python build/scripts/build_starfields.py --check
```

The Author authorized overwriting `tiles/bg/orig/bg_stars_*.png` with these
slices. Canonical Agon64/RGB conversion without dithering writes the same
RGBA2222 names under `assets/rgba2/bg`; canonical decoding writes previews to
`tiles/bg/proc`. Both current build entry points prepare these before packing
`game.agnb`, and their checks include atlas-to-tile reproducibility.

The standalone gameplay-map suite has separate, non-destructive stages:

```bash
.venv/bin/python build/scripts/build_maps.py
.venv/bin/python build/scripts/build_maps.py --check
.venv/bin/python build/scripts/build_map_tests.py
.venv/bin/python build/scripts/build_map_tests.py --start-map 5
```

The map compiler reads only the ordered manifest and its TMX/TSX/catalog
inputs. It never invokes the dormant coupled `tiles/dg/tile.py` entry point or
regenerates bitmap assets. The assembly stage copies `src/asm` into a temporary
directory and overlays the generated suite text there as the temporary
`levels_tileset_0.inc` expected by `nurples.asm`, leaving the repository's
campaign include byte-for-byte untouched. It writes the separate runtime
artifact `tgt/nurples-map-tests.bin`; `--start-map` selects a PN-MAP ID to run
first while retaining the cyclic nine-map suite.

## Vendored fonts

`assets/fonts` preserves the vendored font collection and conversion metadata.
The runtime selection in `nurples_tools/fonts.py` contains only the
`Lat38-VGA8_8x8.font` face used by the game. The build must not read fonts from
an emulator SD tree, another checkout, or an Author-specific absolute path.

Build and verify the deployable `tgt/fonts` binary and
`src/asm/fonts_list.inc` manifest with:

```bash
.venv/bin/python build/scripts/build_fonts.py
.venv/bin/python build/scripts/build_fonts.py --check
```

Runtime font filenames are relative to `tgt` and therefore begin with
`fonts/`. Historical `original_font_path` values in the vendored XML are
provenance only. XML metadata is a build input retained under `assets/fonts`;
it is not copied into the deployable target.

## Audit status

The 2026-08-02 audit parsed all 49 Python files that were tracked when the work
began. The stale embedded extension and four `agonImages.py` clones were
removed; five unique inactive font scripts were preserved under
`tools/legacy_fonts/`. All retained scripts compile on Python 3.14.6, and the
migrated image consumers pass import smoke tests.

Application-level defects found in dormant scripts are recorded with evidence
in `devlog/2026-08-02-python-environment.md` and promoted, where intentional,
to the single checklist in `TODO.md`.
