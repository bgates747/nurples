import runpy
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

from nurples_tools.maps import (
    MapCompileError,
    check_map_include,
    compile_map_manifest,
    write_map_include,
)

PROJECT_ROOT = Path(__file__).resolve().parents[1]
SUITE_MANIFEST = PROJECT_ROOT / "tests/maps/dg/suite.toml"
ACTIVE_GLYPHS = {1: "S", 5: "T", 9: "E", 10: "=", 11: "e", 16: "C"}
BUILD_MAP_TESTS = runpy.run_path(
    str(PROJECT_ROOT / "build/scripts/build_map_tests.py")
)


class MapCompilerTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.maps_dir = self.root / "maps"
        self.maps_dir.mkdir()
        self.tileset = self.root / "dg0.tsx"
        self.catalog = self.root / "dg_0_files.xml"
        self.manifest = self.root / "manifest.toml"
        self.output = self.root / "generated/levels.inc"
        self._write_tileset()
        self._write_catalog({0, 1, 5, 9, 10, 11, 16, 191})

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def _write_tileset(self) -> None:
        self.tileset.write_text(
            """<?xml version="1.0" encoding="UTF-8"?>
<tileset name="dg0" tilewidth="16" tileheight="16" tilecount="256" columns="16">
 <image source="dg0.png" width="256" height="256"/>
</tileset>
""",
            encoding="utf-8",
        )

    def _write_catalog(self, indices: set[int]) -> None:
        records = "\n".join(
            f'    <Tile index="{index}">tiles/dg/0/{index:03d}.png</Tile>'
            for index in sorted(indices)
        )
        self.catalog.write_text(
            f"""<?xml version="1.0"?>
<TileSet>
  <TileWidth>16</TileWidth>
  <TileHeight>16</TileHeight>
  <BaseBufferId>512</BaseBufferId>
  <Tiles>
{records}
  </Tiles>
</TileSet>
""",
            encoding="utf-8",
        )

    @staticmethod
    def _row(first: int, fill: int = 0, *, trailing_comma: bool = False) -> str:
        row = ",".join(str(value) for value in (first, *([fill] * 15)))
        return row + ("," if trailing_comma else "")

    def _write_tmx(
        self,
        filename: str,
        rows: list[str],
        *,
        orientation: str = "orthogonal",
        width: int = 16,
        height: int | None = None,
        firstgid: int = 1,
        tileset_source: str = "../dg0.tsx",
        encoding: str = "csv",
        extra_content: str = "",
        layer_width: int | None = None,
        layer_height: int | None = None,
    ) -> Path:
        height = len(rows) if height is None else height
        layer_width = width if layer_width is None else layer_width
        layer_height = height if layer_height is None else layer_height
        path = self.maps_dir / filename
        path.write_text(
            f"""<?xml version="1.0" encoding="UTF-8"?>
<map orientation="{orientation}" width="{width}" height="{height}" tilewidth="16" tileheight="16" infinite="0">
 <tileset firstgid="{firstgid}" source="{tileset_source}"/>
 <layer width="{layer_width}" height="{layer_height}">
  <data encoding="{encoding}">
{chr(10).join(rows)}
  </data>
 </layer>
 {extra_content}
</map>
""",
            encoding="utf-8",
        )
        return path

    def _write_manifest(self, levels: list[tuple[str, str]]) -> None:
        entries = "\n".join(
            f'''[[levels]]
name = "{name}"
source = "maps/{source}"
'''
            for name, source in levels
        )
        self.manifest.write_text(
            f'''format_version = 1
symbol_prefix = "tileset_00"
tileset = "dg0.tsx"
tile_catalog = "dg_0_files.xml"
output = "generated/levels.inc"

{entries}''',
            encoding="utf-8",
        )

    def _compile_single(self, rows: list[str]) -> None:
        self._write_tmx("one.tmx", rows)
        self._write_manifest([("Test One", "one.tmx")])

    def test_manifest_order_and_runtime_row_reversal(self) -> None:
        self._write_tmx(
            "first.tmx",
            [self._row(1, trailing_comma=True), self._row(5)],
        )
        self._write_tmx("second.tmx", [self._row(9)])
        self._write_manifest(
            [("First Level", "first.tmx"), ("Second Level", "second.tmx")]
        )

        compiled = compile_map_manifest(self.manifest)

        self.assertEqual([level.name for level in compiled.levels], ["First Level", "Second Level"])
        self.assertEqual(compiled.levels[0].runtime_rows[0][0], 5)
        self.assertEqual(compiled.levels[0].runtime_rows[1][0], 1)
        self.assertIn("tileset_00_num_levels: db 2", compiled.text)
        self.assertIn(
            "tileset_00_levels:\n"
            "\tdl tileset_00_level_00\n"
            "\tdl tileset_00_level_01\n"
            "\tdl 0 ; list terminator",
            compiled.text,
        )
        self.assertIn("\tdl 512        ; base bufferId", compiled.text)
        first_runtime_row = "\tdb 005," + ",".join(["000"] * 15)
        first_tiled_row = "\tdb 001," + ",".join(["000"] * 15)
        self.assertLess(compiled.text.index(first_runtime_row), compiled.text.index(first_tiled_row))

    def test_write_and_check_are_deterministic_and_check_does_not_write(self) -> None:
        self._compile_single([self._row(1)])
        first_render = compile_map_manifest(self.manifest).text
        second_render = compile_map_manifest(self.manifest).text
        self.assertEqual(first_render, second_render)

        self.assertEqual(write_map_include(self.manifest), self.output.resolve())
        first_bytes = self.output.read_bytes()
        self.assertEqual(check_map_include(self.manifest), [])
        write_map_include(self.manifest)
        self.assertEqual(self.output.read_bytes(), first_bytes)

        self.output.write_bytes(b"deliberate drift\n")
        drift = self.output.read_bytes()
        self.assertEqual(len(check_map_include(self.manifest)), 1)
        self.assertEqual(self.output.read_bytes(), drift)

    def test_missing_output_is_reported_without_creation(self) -> None:
        self._compile_single([self._row(1)])
        differences = check_map_include(self.manifest)
        self.assertRegex(differences[0], "missing generated map include")
        self.assertFalse(self.output.exists())

    def test_rejects_nonorthogonal_or_invalid_dimensions(self) -> None:
        cases = (
            ({"orientation": "isometric"}, "orientation"),
            ({"width": 15}, "width"),
            ({"height": 0}, "outside 1..256"),
            ({"height": 257}, "outside 1..256"),
            ({"layer_width": 15}, "layer dimensions"),
        )
        for arguments, message in cases:
            with self.subTest(arguments=arguments):
                self._write_tmx("one.tmx", [self._row(1)], **arguments)
                self._write_manifest([("Test One", "one.tmx")])
                with self.assertRaisesRegex(MapCompileError, message):
                    compile_map_manifest(self.manifest)

    def test_rejects_invalid_map_tile_dimensions(self) -> None:
        self._compile_single([self._row(1)])
        source = self.maps_dir / "one.tmx"
        source.write_text(
            source.read_text(encoding="utf-8").replace(
                'tilewidth="16" tileheight="16"',
                'tilewidth="8" tileheight="16"',
            ),
            encoding="utf-8",
        )
        with self.assertRaisesRegex(MapCompileError, "tile dimensions"):
            compile_map_manifest(self.manifest)

    def test_rejects_wrong_layer_count_or_encoding(self) -> None:
        self._write_tmx(
            "one.tmx",
            [self._row(1)],
            extra_content='<objectgroup name="forbidden"/>',
        )
        self._write_manifest([("Test One", "one.tmx")])
        with self.assertRaisesRegex(MapCompileError, "exactly one tile layer"):
            compile_map_manifest(self.manifest)

        self._write_tmx("one.tmx", [self._row(1)], encoding="base64")
        with self.assertRaisesRegex(MapCompileError, "CSV encoding"):
            compile_map_manifest(self.manifest)

    def test_rejects_wrong_tileset_or_firstgid(self) -> None:
        self._write_tmx("one.tmx", [self._row(1)], firstgid=2)
        self._write_manifest([("Test One", "one.tmx")])
        with self.assertRaisesRegex(MapCompileError, "firstgid must be 1"):
            compile_map_manifest(self.manifest)

        self._write_tmx("one.tmx", [self._row(1)], tileset_source="other.tsx")
        with self.assertRaisesRegex(MapCompileError, "external dg0.tsx"):
            compile_map_manifest(self.manifest)

    def test_rejects_inexact_csv_shape(self) -> None:
        self._write_tmx("one.tmx", [self._row(1)], height=2)
        self._write_manifest([("Test One", "one.tmx")])
        with self.assertRaisesRegex(MapCompileError, "CSV has 1 rows; expected 2"):
            compile_map_manifest(self.manifest)

        short_row = ",".join(["1"] * 15)
        self._write_tmx("one.tmx", [short_row])
        with self.assertRaisesRegex(MapCompileError, "has 15 cells; expected 16"):
            compile_map_manifest(self.manifest)

        doubled_trailer = self._row(1) + ",,"
        self._write_tmx("one.tmx", [doubled_trailer])
        with self.assertRaisesRegex(MapCompileError, "has 18 cells; expected 16"):
            compile_map_manifest(self.manifest)

    def test_rejects_tiled_transform_bits_and_gid_overflow(self) -> None:
        self._compile_single([self._row(0x8000_0001)])
        with self.assertRaisesRegex(MapCompileError, "transform bits"):
            compile_map_manifest(self.manifest)

        self._compile_single([self._row(256)])
        with self.assertRaisesRegex(MapCompileError, "exceeds 255"):
            compile_map_manifest(self.manifest)

    def test_rejects_nonzero_gid_absent_from_catalog(self) -> None:
        self._compile_single([self._row(7)])
        with self.assertRaisesRegex(MapCompileError, "absent from dg_0_files.xml"):
            compile_map_manifest(self.manifest)

    def test_rejects_catalog_without_runtime_base_tile(self) -> None:
        self._write_catalog({1})
        self._compile_single([self._row(0)])
        with self.assertRaisesRegex(MapCompileError, "tile index 0"):
            compile_map_manifest(self.manifest)

    def test_rejects_catalog_with_wrong_runtime_buffer_base(self) -> None:
        self._compile_single([self._row(0)])
        source = self.catalog.read_text(encoding="utf-8")
        self.catalog.write_text(
            source.replace(
                "<BaseBufferId>512</BaseBufferId>",
                "<BaseBufferId>256</BaseBufferId>",
            ),
            encoding="utf-8",
        )
        with self.assertRaisesRegex(MapCompileError, "expected 512"):
            compile_map_manifest(self.manifest)

    def test_manifest_rejects_unknown_keys_absolute_paths_and_duplicates(self) -> None:
        self._compile_single([self._row(1)])
        original = self.manifest.read_text(encoding="utf-8")
        self.manifest.write_text(original + "unknown = true\n", encoding="utf-8")
        with self.assertRaisesRegex(MapCompileError, "unknown"):
            compile_map_manifest(self.manifest)

        self._write_manifest([("Test One", "one.tmx")])
        portable = self.manifest.read_text(encoding="utf-8")
        self.manifest.write_text(
            portable.replace('tileset = "dg0.tsx"', f'tileset = "{self.tileset}"'),
            encoding="utf-8",
        )
        with self.assertRaisesRegex(MapCompileError, "relative for portability"):
            compile_map_manifest(self.manifest)

        self._write_manifest([("Test One", "one.tmx"), ("Test One", "one.tmx")])
        with self.assertRaisesRegex(MapCompileError, "duplicate.*name"):
            compile_map_manifest(self.manifest)


class StandaloneMapSuiteTests(unittest.TestCase):
    def test_order_dimensions_output_and_determinism(self) -> None:
        first = compile_map_manifest(SUITE_MANIFEST)
        second = compile_map_manifest(SUITE_MANIFEST)

        self.assertEqual(first.text, second.text)
        self.assertEqual(
            [level.name for level in first.levels],
            [
                "PN-MAP-00 Flight Deck",
                "PN-MAP-01 Seeker Nursery",
                "PN-MAP-02 Fire-Control Range",
                "PN-MAP-03 Circuit Breaker",
                "PN-MAP-04 Demolition Yard",
                "PN-MAP-05 Damage Control",
                "PN-MAP-08 Orbital Shipyard",
                "PN-MAP-06 Dirty Handoff",
                "PN-MAP-07 Clean Room",
            ],
        )
        self.assertEqual(
            [level.height for level in first.levels],
            [32, 32, 32, 32, 32, 64, 48, 32, 32],
        )
        self.assertEqual(
            first.output_path,
            (PROJECT_ROOT / "src/asm/levels_test_maps.inc").resolve(),
        )
        self.assertNotEqual(
            first.output_path,
            (PROJECT_ROOT / "src/asm/levels_tileset_0.inc").resolve(),
        )

    def test_behavioral_rows_match_the_operator_cards(self) -> None:
        compiled = compile_map_manifest(SUITE_MANIFEST)
        expected = [
            {},
            {0: "..S..........S..", 4: ".......S........", 7: "....S......S...."},
            {0: "..T..........T..", 4: ".......T........", 7: "....T......T...."},
            {0: ".E============e.", 4: "....E======e....", 7: "......E==e......"},
            {0: "..C...C..C...C..", 4: "....S......T....", 7: ".C.C.C.C.C.C.C.C"},
            {
                0: ".......S........",
                4: "....T......T....",
                8: "....E======e....",
                12: "..E==========e..",
                14: "..E==========e..",
            },
            {
                0: "..S..........S..",
                4: ".......T........",
                7: "..E====e........",
                12: "...T........T...",
                16: ".C....C..C....C.",
                20: "....S....T......",
            },
            {
                0: "..S....T....S...",
                6: ".......T........",
                18: "....S......T....",
                26: ".E============e.",
                30: "..C..........C..",
            },
            {},
        ]

        for level, expected_rows in zip(compiled.levels, expected, strict=True):
            actual_rows = {}
            unexpected_low_ids = {
                gid
                for row in level.runtime_rows
                for gid in row
                if 0 < gid < 17 and gid not in ACTIVE_GLYPHS
            }
            for index, row in enumerate(level.runtime_rows):
                rendered = "".join(ACTIVE_GLYPHS.get(gid, ".") for gid in row)
                if rendered != "." * 16:
                    actual_rows[index] = rendered
            with self.subTest(level=level.name):
                self.assertEqual(unexpected_low_ids, set())
                self.assertEqual(actual_rows, expected_rows)

    def test_start_map_selects_by_pn_map_id_not_manifest_position(self) -> None:
        levels = compile_map_manifest(SUITE_MANIFEST).levels
        resolve = BUILD_MAP_TESTS["qualification_start_position"]
        rotate = BUILD_MAP_TESTS["rotate_pointer_table"]

        self.assertEqual(resolve(levels, 0), 0)
        self.assertEqual(resolve(levels, 8), 6)
        self.assertEqual(resolve(levels, 6), 7)
        self.assertEqual(resolve(levels, 7), 8)
        with self.assertRaisesRegex(MapCompileError, "0 through 8"):
            resolve(levels, 9)

        source = compile_map_manifest(SUITE_MANIFEST).text
        rotated = rotate(source, resolve(levels, 8), len(levels))
        table = rotated.split("tileset_00_levels:\n", 1)[1]
        self.assertTrue(table.startswith("\tdl tileset_00_level_06\n"))


if __name__ == "__main__":
    unittest.main()
