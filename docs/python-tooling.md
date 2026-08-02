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

## Vendored fonts

`assets/fonts` is the authoritative source for the 29 runtime font binaries and
their conversion metadata. The build must not read fonts from an emulator SD
tree, another checkout, or an Author-specific absolute path.

Build and verify the deployable `tgt/fonts` copies and
`src/asm/fonts_list.inc` manifest with:

```bash
.venv/bin/python build/scripts/build_fonts.py
.venv/bin/python build/scripts/build_fonts.py --check
```

Runtime font filenames are relative to `tgt` and therefore begin with
`fonts/`. Historical `original_font_path` values in the vendored XML are
provenance only and are not build inputs.

## Audit status

The 2026-08-02 audit parsed all 49 Python files that were tracked when the work
began. The stale embedded extension and four `agonImages.py` clones were
removed; five unique inactive font scripts were preserved under
`tools/legacy_fonts/`. All retained scripts compile on Python 3.14.6, and the
migrated image consumers pass import smoke tests.

Application-level defects found in dormant scripts are recorded with evidence
in `devlog/2026-08-02-python-environment.md` and promoted, where intentional,
to the single checklist in `TODO.md`.
