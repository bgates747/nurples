from pathlib import Path
import struct
from tempfile import TemporaryDirectory
import unittest

from nurples_tools.agnb import ImageRecord, build_container
from nurples_tools.agnb_verify import parse_images, verify_container
from nurples_tools.current_assets import (
    build_catalogs,
    check_current_assets,
    render_container_include,
    render_family_include,
)


PROJECT_ROOT = Path(__file__).resolve().parents[1]


class AgnbContainerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.catalogs = build_catalogs(PROJECT_ROOT)
        cls.ui_records = cls.catalogs["ui"]
        cls.game_records = (
            cls.catalogs["bg"]
            + cls.catalogs["sprites"]
            + cls.catalogs["tiles_dg"]
        )

    def test_catalog_contract_and_global_ids(self) -> None:
        self.assertEqual(len(self.ui_records), 46)
        self.assertEqual(len(self.catalogs["bg"]), 4)
        self.assertEqual(len(self.catalogs["sprites"]), 159)
        self.assertEqual(len(self.catalogs["tiles_dg"]), 187)
        self.assertEqual(len(self.game_records), 350)
        self.assertEqual(
            [record.family for record in self.game_records[:5]],
            ["bg", "bg", "bg", "bg", "sprites"],
        )
        ids = [record.buffer_id for record in self.ui_records + self.game_records]
        self.assertEqual(len(ids), len(set(ids)))
        self.assertNotIn(0xFFFF, ids)
        self.assertEqual([record.buffer_id for record in self.ui_records], list(range(46)))
        self.assertEqual(
            [record.buffer_id for record in self.catalogs["sprites"]],
            list(range(256, 415)),
        )
        self.assertEqual(
            [record.buffer_id for record in self.catalogs["bg"]],
            list(range(1024, 1028)),
        )

    def test_checked_in_containers_match_every_catalog_payload(self) -> None:
        ui = (PROJECT_ROOT / "tgt/ui.agnb").read_bytes()
        game = (PROJECT_ROOT / "tgt/game.agnb").read_bytes()
        self.assertEqual(len(ui), 246_844)
        self.assertEqual(len(game), 367_168)
        verify_container(ui, self.ui_records)
        verify_container(game, self.game_records)

    def test_generated_outputs_are_deterministic(self) -> None:
        self.assertEqual(check_current_assets(PROJECT_ROOT), [])

    def test_assembly_metadata_has_no_runtime_paths_or_load_tables(self) -> None:
        for family, records in self.catalogs.items():
            rendered = render_family_include(family, records)
            self.assertNotIn(".rgba2", rendered)
            self.assertNotIn("_image_list", rendered)
            self.assertNotIn("fn_", rendered)
        container_counts = render_container_include(self.ui_records, self.game_records)
        self.assertIn("ui_agnb_num_images: equ 46", container_counts)
        self.assertIn("game_num_images: equ 350", container_counts)

    def test_target_contains_only_runtime_files(self) -> None:
        files = {
            path.relative_to(PROJECT_ROOT / "tgt").as_posix()
            for path in (PROJECT_ROOT / "tgt").rglob("*")
            if path.is_file()
        }
        self.assertEqual(
            files,
            {
                "fonts/Lat38-VGA8_8x8.font",
                "game.agnb",
                "nurples-map-tests.bin",
                "nurples-map-tests.symbols",
                "nurples.bin",
                "nurples.symbols",
                "ui.agnb",
            },
        )
        self.assertFalse(list((PROJECT_ROOT / "tgt").rglob("*.rgba2")))

    def test_writer_rejects_duplicate_and_reserved_ids(self) -> None:
        with TemporaryDirectory() as temp_dir:
            payload = Path(temp_dir) / "one.rgba2"
            payload.write_bytes(b"\xff")
            first = ImageRecord("test", "first", 1, 1, 1, payload)
            duplicate = ImageRecord("test", "second", 1, 1, 1, payload)
            reserved = ImageRecord("test", "reserved", 0xFFFF, 1, 1, payload)
            with self.assertRaisesRegex(ValueError, "duplicate VDP buffer ID"):
                build_container([first, duplicate])
            with self.assertRaisesRegex(ValueError, "invalid VDP buffer ID"):
                build_container([reserved])

    def test_independent_parser_rejects_structural_corruption(self) -> None:
        valid = bytearray((PROJECT_ROOT / "tgt/ui.agnb").read_bytes())

        bad_size = bytearray(valid)
        struct.pack_into("<I", bad_size, 4, len(bad_size))
        with self.assertRaisesRegex(ValueError, "RIFF size"):
            parse_images(bytes(bad_size))

        bad_version = bytearray(valid)
        bad_version[20:22] = b"\x00\x02"
        with self.assertRaisesRegex(ValueError, "version 0.1"):
            parse_images(bytes(bad_version))

        with self.assertRaises(ValueError):
            parse_images(bytes(valid[:-1]))

        # VERS has a two-byte payload followed by two bytes of RIFF padding.
        bad_padding = bytearray(valid)
        bad_padding[22] = 1
        with self.assertRaisesRegex(ValueError, "nonzero alignment padding"):
            parse_images(bytes(bad_padding))


if __name__ == "__main__":
    unittest.main()
