from pathlib import Path
import struct
import tomllib
from tempfile import TemporaryDirectory
import unittest

from nurples_tools.agnb import ImageRecord, build_container
from nurples_tools.announcer import audio_container
from nurples_tools.active_tiles import REGISTRY, parse
from nurples_tools.agnb_verify import parse_images, verify_container
from nurples_tools.current_assets import (
    build_catalogs,
    check_current_assets,
    container_records,
    earth_records,
    orient_entry_tiles,
    render_container_include,
    render_family_include,
    title_phase_lut,
)


PROJECT_ROOT = Path(__file__).resolve().parents[1]


class AgnbContainerTests(unittest.TestCase):
    def test_production_bundle_carries_the_reviewed_three_cue_bank(self):
        samples = PROJECT_ROOT / "assets/announcer"
        self.assertEqual({p.stem for p in samples.glob("*.wav")},
                         {"game_over", "shields_depleted", "shields_critical"})
        self.assertEqual(audio_container(samples), (PROJECT_ROOT / "tgt/voice.agnb").read_bytes())
        bundles = tomllib.loads((PROJECT_ROOT / "deploy.toml").read_text())["bundles"]
        for name in ("nurples", "nurples-runtime"):
            self.assertIn("voice.agnb", bundles[name]["files"])
            self.assertIn("voice.agnb", bundles[name]["require"])
        self.assertNotIn("voice.agnb", bundles["map-tests"]["files"])

    @classmethod
    def setUpClass(cls) -> None:
        cls.catalogs = build_catalogs(PROJECT_ROOT)
        cls.ui_records = cls.catalogs["ui"]
        cls.game_records = (
            cls.catalogs["bg"]
            + cls.catalogs["sprites"]
            + cls.catalogs["tiles_dg"]
            + cls.catalogs["overlays"]
        )

    def test_catalog_contract_and_global_ids(self) -> None:
        self.assertEqual(len(self.ui_records), 47)
        self.assertEqual(len(self.catalogs["bg"]), 4)
        self.assertEqual(len(self.catalogs["sprites"]), 169)
        self.assertEqual(len(self.catalogs["tiles_dg"]), 192)
        self.assertEqual(len(self.catalogs["overlays"]), 4)
        self.assertEqual(len(self.game_records), 369)
        self.assertEqual(
            [record.family for record in self.game_records[:5]],
            ["bg", "bg", "bg", "bg", "sprites"],
        )
        ids = [record.buffer_id for record in self.ui_records + self.game_records]
        self.assertEqual(len(ids), len(set(ids)))
        self.assertNotIn(0xFFFF, ids)
        self.assertEqual([record.buffer_id for record in self.ui_records], list(range(47)))
        self.assertEqual(
            [record.buffer_id for record in self.catalogs["sprites"]],
            list(range(256, 425)),
        )
        self.assertEqual(
            [record.buffer_id for record in self.catalogs["bg"]],
            list(range(1024, 1028)),
        )
        self.assertEqual(
            [record.buffer_id for record in self.catalogs["overlays"]],
            list(range(63_990, 63_994)),
        )

    def test_checked_in_containers_match_every_catalog_payload(self) -> None:
        ui = (PROJECT_ROOT / "tgt/ui.agnb").read_bytes()
        game = (PROJECT_ROOT / "tgt/game.agnb").read_bytes()
        self.assertEqual(len(ui), 371_052)
        self.assertEqual(len(game), 1_444_128)
        ui_records, game_records = container_records(self.catalogs)
        verify_container(ui, ui_records)
        with TemporaryDirectory() as tmp:
            slots = parse((PROJECT_ROOT / REGISTRY).read_text())
            game_records = orient_entry_tiles(game_records, slots, Path(tmp))
            verify_container(game, game_records + earth_records(PROJECT_ROOT, Path(tmp)))
        self.assertEqual(set(record.buffer_id for record in self.ui_records)
                         - set(image.buffer_id for image in parse_images(ui)), {46})
        self.assertEqual(set(record.buffer_id for record in self.game_records)
                         - set(image.buffer_id for image in parse_images(game)), {63_991})

    def test_title_phase_lookup_reproduces_both_alternates(self) -> None:
        for records, name in ((self.ui_records, "splash_title"),
                              (self.game_records, "game_over")):
            with self.subTest(title=name):
                frames = {rec.name: rec.payload_path.read_bytes() for rec in records
                          if rec.name.startswith(name)}
                master, alternate = frames[f"{name}_0"], frames[f"{name}_1"]
                lookup = title_phase_lut(master, alternate)
                self.assertEqual(len(lookup), 256)
                self.assertEqual(lookup[:0xC0], bytes(range(0xC0)))
                self.assertNotEqual(master, alternate)
                self.assertEqual(master.translate(lookup), alternate)

    def test_title_lookup_rejects_ambiguous_colors_or_changed_alpha(self) -> None:
        with self.assertRaisesRegex(ValueError, "conflicting mappings"):
            title_phase_lut(b"\xff\xff", b"\xfe\xff")
        with self.assertRaisesRegex(ValueError, "binary opacity mask"):
            title_phase_lut(b"\xff", b"\x00")
        with self.assertRaisesRegex(ValueError, "transparent bytes unchanged"):
            title_phase_lut(b"\x00", b"\x01")

    def test_generated_outputs_are_deterministic(self) -> None:
        self.assertEqual(check_current_assets(PROJECT_ROOT), [])

    def test_assembly_metadata_has_no_runtime_paths_or_load_tables(self) -> None:
        for family, records in self.catalogs.items():
            rendered = render_family_include(family, records)
            self.assertNotIn(".rgba2", rendered)
            self.assertNotIn("_image_list", rendered)
            self.assertNotIn("fn_", rendered)
        ui, game = container_records(self.catalogs)
        with TemporaryDirectory() as tmp:
            container_counts = render_container_include(ui, game + earth_records(PROJECT_ROOT, Path(tmp)))
        self.assertIn("ui_agnb_num_images: equ 46", container_counts)
        self.assertIn("game_num_images: equ 384", container_counts)

    def test_target_contains_only_supported_artifacts(self) -> None:
        files = {
            path.relative_to(PROJECT_ROOT / "tgt").as_posix()
            for path in (PROJECT_ROOT / "tgt").rglob("*")
            if path.is_file()
        }
        # A fresh checkout has no private music build; after assembly both
        # output files must be present, never an orphaned binary or symbols.
        private_pair = {"nurples.bin", "nurples.symbols"}
        self.assertIn(files & private_pair, (set(), private_pair))
        self.assertEqual(
            files - private_pair,
            {
                "fonts/Lat38-VGA8_8x8.font",
                "game.agnb",
                "nurples-map-tests.bin",
                "nurples-map-tests.symbols",
                "nurples-uart001.bin",
                "nurples-uart001.symbols",
                "ui.agnb",
                "voice.agnb",
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
