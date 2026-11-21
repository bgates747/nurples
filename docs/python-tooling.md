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

`--assets` first prepares the cel UI artwork, four starfield
tiles, and the five passive original-Z80 tiles, then builds the one runtime
font plus `ui.agnb` and `game.agnb` from the RGBA2222 payloads under
`assets/rgba2`. It also compiles the ordered production campaign manifest into
`src/asm/levels_tileset_0.inc`. `--check` checks the source-to-payload
conversion, decoded PNG previews, generated metadata, every packed payload and
the campaign include without writing. The orchestrator does not deploy files
or launch an emulator.

### Production soundtrack

`--assemble` and `--all` require private copies of `terminator.pt3`,
`bugs2_2.pt3` and `01_loadtune.pt3`. Set `NURPLES_MUSIC_DIR` to their
directory, or put them in the ignored `.build/mus001/audition/`. The build
checks lengths and SHA-256 against [the tune manifest](tasks/MUS-001/samples.json)
before replacing any production output. It does not download music or fall
back to a silent release. Asset-only operations do not need the songs.

The normal output is still `tgt/nurples.bin`, with matching local symbols.
Neither output is tracked; do not redistribute an executable, assembly listing
or card image containing the songs. Standalone diagnostic assembly uses the
song-free configuration. [Campaign audio](../src/asm/campaign_audio.inc)
uses Nick J. Date's unmodified agonAMP player sources (commit
`91ef401bcb7f636e7593ad1ba176508344deebb9`, [license](../src/asm/audio/agonAMP/LICENSE)).
Laser and explosion patches are from the AgonArcade soundboard; the review
builder verifies the command bytes against the pinned generator.

### Cel UI artwork

`build/ui_assets.json` explicitly fixes all 47 UI buffer IDs and runtime
dimensions. Additional PNG variants in the original-art directory cannot
enter the runtime catalog or shift IDs. Its `prepare` entries select
`bezel_l_cel.png`, `bezel_r_cel_v2.png`, the hand-edited
`splash_sprite_faithful_v3_512x384.png`, and the two generated title phases.

Prepare these images independently with:

```bash
.venv/bin/python build/scripts/build_ui_art.py
.venv/bin/python build/scripts/build_ui_art.py --check
```

This stage uses Pillow Lanczos scaling to prepare the 128x304 bezels. The
512x384 splash background arrives at its final size and
palette, so the hand-edited export is preserved pixel for pixel. Canonical
`agonutils` performs nearest-RGB Agon64 conversion without dithering and
RGBA2222 encoding through the local adapters. Decoded previews replace their
PNGs in `assets/img/proc/ui`.
Both current asset-build entry points invoke this stage before packing AGNB.
The standalone preparation command does not repack the containers.

The 240x89 `splash_title_0` and `splash_title_1` put the native-size tagline
`WELCOME TO ATTACK OF THE` above `PURPLE` and `NURPLES`. The large lettering
uses the same Lode Runner glyph rows, block shadows, and single-pixel
highlights as GAME OVER, at 4x. The tagline uses the opposite colour phase.
The generator writes editable SVGs to `assets/vector/ui` and produces
identical binary-alpha footprints. The loader plots them at the reviewed
Mode 20 anchor `(28, 50)` and changes colors only at completed-record
boundaries. The background is painted once; the existing fixed-width status
text is overwritten in place on every record. After the last record, a
keyboard/joystick question replaces that line before the pre-start screen.
The old checkerboard logo is archival artwork outside the runtime catalog.

The AGNB files upload only phase 0 of the splash and Game Over titles.
The asset builder keeps both authored phases for comparison and emits
one 256-byte lookup each. It rejects partial or mismatched alpha masks,
changes to transparent bytes, and ambiguous source-colour mappings. At
startup the VDP expands each master into the existing phase-1 ID with
buffered command 72, then registers the result as RGBA2222. The scratch
lookup buffer is cleared after each use. Both generated bitmaps remain
resident, so this reduces upload traffic, not steady-state VDP memory.

### Starfield tiles

The four hand-retouched 256×256 PNGs in `tiles/bg/orig` are the current sources. The
asset stage checks their dimensions, encodes them without scaling or
border edits, and packs them at the same IDs (1024–1027). The sparse atlas
remains archived artwork and is not a build input. See the
[rollback record](tasks/ART-001/original-starfield-rollback.md) for the
historical assets and byte-for-byte verification.
The untouched originals remain available as a
[separate control](tasks/ART-001/starfield-join-review/original).

```bash
.venv/bin/python build/scripts/build_starfields.py
.venv/bin/python build/scripts/build_starfields.py --check
```

Canonical Agon64/RGB conversion without dithering writes RGBA2222 files in
`assets/rgba2/bg`; canonical decoding writes previews to `tiles/bg/proc`.
Both asset build entry points prepare these before packing `game.agnb`.
The standalone command prepares or checks the tiles but does not repack.

### Original Z80 level and passive artwork

The production campaign is declared by `maps/campaign.toml` and can be built
or checked independently:

```bash
.venv/bin/python build/scripts/build_campaign_maps.py
.venv/bin/python build/scripts/build_campaign_maps.py --check
```

Level zero remains the modern 256-row Agon map. Level one is the recovered
original Z80 game's 255-row main level. `tools/import_legacy_z80_level.py` is a
strict, provenance-only importer that decodes the old row mask and packed tile
nibbles into the checked-in `tiles/dg/nurples_z80_main.tmx`; the normal build
does not depend on the recovered checkout or import executable code.

The unchanged old passive sources live under `tiles/dg/legacy_z80`. The asset
build converts them through Agon64 and canonical `agonutils`, publishes them as
runtime tile IDs 17 through 21, and updates their Tiled atlas cells. Old tile
ID 7 maps to modern active ID 1 and old ID 8 maps to modern active ID 5. This
deliberately preserves the modern launch-pad and laser-turret animations and
their in-place transformation to modern crater ID 16.

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

The tracked `LodeRunner-Hybrid-8x8.font` is generated project-locally from the
exact provenance-pinned A-Z rows in `nurples_tools.lode_font` plus the current
Lat38 fallback cells. Both supported current-asset orchestrators generate and
verify it before the runtime-font copy, even though it is not yet selected for
deployment. See `docs/lode-runner-font.md` for source hashes and the no-resample
contract.

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
