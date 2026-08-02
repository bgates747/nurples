# 2026-08-02 — Python environment and script audit

## Scope

Bring Purple Nurples into the shared Agon Python-environment contract, install
canonical `agonutils` for image processing, and audit every tracked Python
script for environment or runtime breakage and duplicated image tooling.

## Environment result

1. Created `.venv` with Python 3.14.6.
2. Installed pinned Pillow, NumPy, pandas, Matplotlib, FontTools, yt-dlp, and
   Ruff dependencies.
3. Installed `/home/smith/Agon/mystuff/agon-utils` 1.1.0 editable with
   `--no-build-isolation --no-deps` after verifying FFmpeg/libpng native
   prerequisites through `pkg-config`.
4. Added `pyproject.toml`, reproducible requirements files, and the editable
   `nurples-tools` package.

`pip check` passed. The canonical consumer test passed. From the repository
root, `agonutils.__file__` now resolves to:

```text
/home/smith/Agon/mystuff/agon-utils/agonutils.cpython-314-x86_64-linux-gnu.so
```

## Root causes and changes

1. The tracked top-level `agonutils/` directory had no `__init__.py`. Python's
   normal path finder claimed it as a namespace before the editable canonical
   finder, yielding `agonutils.__file__ == None` and no API attributes.
2. That embedded version-1.0 fork built only its old `agonutils.c`, linked only
   libpng, used `setup.py install`, and omitted the canonical AGM/RLE/SIMZ and
   FFmpeg sources. Its cwd-relative `build_and_install.py` could delete the
   application's own `build/` directory when launched from the project root.
3. Removed the obsolete package/setup/C/test material and the now-empty masking
   directories. Preserved its five unique dormant font tools under
   `tools/legacy_fonts/`; the identical canonical C sorting helper was not
   retained.
4. Removed four 348/349-line `agonImages.py` copies. Three were byte-identical;
   the fourth differed by a comment. They duplicated palette matching,
   RGBA2222 encoding, and both decoders, mishandled alpha-zero pixels, used a
   non-circular HSV hue distance, and rejected lowercase method names.
5. Added `nurples_tools.images`, a thin adapter that delegates palette and
   RGBA2222 behavior to canonical `agonutils` and retains only Pillow bridging
   plus the missing raw RGBA8888 writer.
6. Updated every image consumer. Fixed obsolete two-argument canonical calls in
   `make_images.py`, `make_images_navball.py`, and `build_91a_asm_font.py`; fixed
   the fatal nonexistent import in `build_91b_asm_ui.py`; and migrated build and
   tile scripts away from bare `agonImages` imports.
7. Made the ASCII-art literal raw so strict Python 3.14 compilation does not
   fail on invalid escape sequences.

## Full-script audit findings

All 49 original Python files parsed. Retained files have no strict
`SyntaxWarning`, syntax error, undefined-name error, or migrated-module import
failure. The following application defects are not Python-environment blockers
and require behavioral decisions or fixtures; they are promoted to `TODO.md`.

### PY-001 — stale orchestrator and stages

- `build_00_all_the_things.py` depends on main-only globals, missing
  `src/mapmaker`, a missing `do_assembly`, and `src/asm/wolf3d.asm`; it passes
  wrong argument counts/order into SFX stages and enables every stage.
- `build_02_fetch_tiles.py`, `build_06_map_import_mapmaker.py`,
  `build_91_asm_img_load.py`, `build_91a_asm_font.py`, and both 91b generators
  reference absent Wolf-era tables or `src/assets` paths.
- `build_98_asm_sfx.py` still assembles `jukebox.asm` and swallows assembler
  failures.
- `build_06_map_import_mapmaker.py` consumes 20 header bytes despite its
  five-byte comment, dereferences a missing tile before checking it, and drops
  prior floors by recreating its table per import.

### PY-002 and PY-006 — unsafe implicit actions

- `build/scripts/tmp.py` walks and edits all of `~/Agon` at import, including
  read-only upstream trees.
- `analyse_noise.py`, `make_fireball.py`, both Xevious slicers, and
  `tiles/scripts/grid.py` perform file or GUI work at import.
- `clean_macos_litter.py` traverses `.git`, deletes without dry-run support,
  and suppresses failures.
- The no-argument `build_99_asm_assemble.py` assembles, recursively replaces a
  mounted path, and launches missing `agon-emulator`. The supported emulator
  entry is `agon-dev-env/scripts/run_emulator.sh nurples`.

### PY-003 — MapMaker conversion

Both identical `tile_png_to_mapmaker.py` copies have an unbound `dir_path` for
zero PNGs, overwrite a full final bank when the count is divisible by ten,
parse hex CSV data as decimal, allow fixed-grid overflow/negative offsets, and
validate decremented header dimensions as if they were actual dimensions.

### PY-004 and PY-005 — tile data

- DG XML repeats `BaseName`, gaps transparent indexes while counting only saved
  files, reuses buffer IDs across ranges, mixes set-number naming schemes, and
  passes a single `TileSet` where its level generator expects child sets.
- CIV/shared tile scripts can emit more than 255 unique indexes through `db`;
  current CIV data records 1,963 unique tiles. The CIV path returns `None`
  because its downstream generation is commented out.
- The shared tile entry expects missing `tiles/dg/dg.png`; tracked input is
  `dg0.png`.
- `xref_map.py` collapses repeated `<Tile index=...>` elements into one key,
  points to a nonexistent `tiled/dg0.tmx`, and does not consistently apply its
  CSV/one-based mapping.

### PY-007 and PY-008 — utilities and duplication

- `sprite_collision.solve_quadratic` divides by zero for identical velocities.
- `analyse_noise.py` filters frequency rather than level.
- `make_fireball.py` overwrites its `img2` variable and ignores one source.
- `youtube.py` claimed to return a path but returned `None`; yt-dlp was also
  absent before this environment work. It now runs the installed module through
  `.venv/bin/python` and requires an explicit URL.
- `make_images_navball.py` duplicates nearly all of `make_images.py`; both
  always write `.rgba2` names in assembly metadata even for their RGBA8 branch.
- CIV/shared `tile.py`, `hex_compare.py`, and MapMaker scripts are exact or near
  duplicates with divergent entry-point behavior.
- `make_bg.py` is nondeterministic and CWD-dependent; both Xevious slicers and
  `mfd_make_weapons.py` reference absent design images.
- `trim_wav_files.py` references a missing source directory and its comment says
  ten seconds while its FFmpeg command trims thirty.
- `build_08_make_sfx.py` has disabled output cleanup, can retain stale products,
  and does not put temporary-file deletion in failure-safe `finally` blocks.
- `build_91b_asm_consolefont.py` assumes headerless metadata and treats only
  exact grayscale 255 as foreground; `build_91b_asm_ui.py` ignores its
  `img_dir` argument.
- `reformat.py` can preserve stray carriage returns while rewriting every
  assembly/include file; it needs fixtures before reuse.
- `report_lst.py` reports zero-byte/overlapping ranges for some adjacent
  includes and continues collecting includes after its main-program marker.
- The standalone `build/scripts/tile.py` has a missing configured source path.
- Several experimental entry points therefore compile but are not qualified
  builds.

### PY-009 — legacy fonts

The five preserved scripts have no current caller, reference obsolete
`src/assets` locations, and include a shell-based recursive deletion in
`process_ttf_font.py`. They remain quarantined and uninstalled.

## Verification evidence

1. Strict `py_compile` completed for all retained Python files under Python
   3.14.6.
2. Ruff critical checks (`F63,F7,F82,E9`) passed.
3. Import smoke tests passed for all migrated direct and adapter consumers.
4. Project unit tests cover canonical resolution, lowercase palette methods,
   RGBA8888 round-trip, and RGBA2222 RGB/alpha threshold golden bytes.
5. `pip check` and the canonical `agonutils` tests passed.
