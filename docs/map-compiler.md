# Safe map-only compiler

`build/scripts/build_maps.py` compiles an explicitly ordered set of Tiled TMX
maps into one assembly include. It never chops tiles, converts images, deletes
directories, deploys files, assembles the game, or launches the emulator.

The default invocation is:

```bash
.venv/bin/python build/scripts/build_maps.py
.venv/bin/python build/scripts/build_maps.py --check
.venv/bin/python build/scripts/build_map_tests.py
.venv/bin/python build/scripts/build_map_tests.py --start-map 5
```

An alternate manifest can be supplied as the positional argument. `--check`
performs the complete parse and validation but never writes; it fails if the
declared include is missing or differs byte-for-byte from deterministic output.

## Manifest

Paths are relative to the manifest. Absolute paths are rejected so a fixture
suite remains portable. TOML `[[levels]]` order is runtime level order.

```toml
format_version = 1
symbol_prefix = "tileset_00"
tileset = "../../../tiles/dg/dg0.tsx"
tile_catalog = "../../../tiles/dg/dg_0_files.xml"
output = "../../../src/asm/levels_test_maps.inc"

[[levels]]
name = "PN-MAP-00 Flight Deck"
source = "pn_map_00_flight_deck.tmx"

[[levels]]
name = "PN-MAP-01 Seeker Nursery"
source = "pn_map_01_seeker_nursery.tmx"
```

The compiler verifies the DG runtime base buffer ID is 512 and that bitmap 0
is loaded, then emits that base from `dg_0_files.xml`. Level labels are assigned
by manifest position (`tileset_00_level_00`, `_01`, and so on), and the level
pointer table ends with the runtime's required null pointer.

## Accepted TMX subset

Every input is required to have exactly:

1. A finite, orthogonal map 16 cells wide and 1 through 256 rows high.
2. One external `dg0.tsx` reference with `firstgid="1"`.
3. One same-sized tile layer, with no object, image, or group layers.
4. Plain, uncompressed CSV containing exactly 16 cells on every source row and
   exactly the declared number of rows. Tiled's conventional trailing comma is
   accepted.
5. GIDs from 0 through 255 with no Tiled flip or rotation bits. Zero is the
   transparent/null cell but remains a real bitmap selected by the runtime, so
   tile index 0 must be loaded. Every nonzero GID must also have a matching
   `<Tile index="...">` in `dg_0_files.xml`.

Tiled rows are written top-to-bottom. Purple Nurples consumes a top-scrolling
map from the bottom upward, so the compiler deliberately reverses complete rows
while preserving left-to-right cell order.

The Python API is intentionally small:

- `compile_map_manifest(path)` validates and renders without writing.
- `write_map_include(path)` writes only the manifest's declared output.
- `check_map_include(path)` reports missing or byte-different output without
  changing it.

The CLI additionally permits only the dedicated generated destination
`src/asm/levels_test_maps.inc` and explicitly refuses the campaign include
`src/asm/levels_tileset_0.inc` as a target.

## Standalone assembly

`build_map_tests.py` compiles the manifest in memory, copies `src/asm` to a
temporary directory, overlays the suite data there under the campaign include
name expected by `nurples.asm`, and assembles a short-named binary there. Only
after assembly succeeds and campaign bytes are rechecked is the result
atomically published as `tgt/nurples-map-tests.bin`. Neither the campaign TMX
nor `levels_tileset_0.inc` is written. `--start-map` resolves a PN-MAP ID and
rotates only the nine-entry level pointer table, allowing one fixture to run
first without changing any map definition or breaking cyclic suite order.
