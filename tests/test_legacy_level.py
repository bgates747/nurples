import unittest

from nurples_tools.legacy_level import (
    LEGACY_ROWS,
    LegacyLevelError,
    parse_main_level,
    render_tmx,
)


def legacy_source(first_row: str = "#F0,#01,#24,#57,#8B") -> str:
    rows = [f"    defb {first_row} ; row 1"]
    rows.extend(f"    defb #00 ; row {number}" for number in range(2, 256))
    return (
        "tiles_level_1: defb  255 ; number of rows\n"
        + "\n".join(rows)
        + "\n"
    )


class LegacyLevelImportTests(unittest.TestCase):
    def test_decodes_pair_mask_and_maps_only_data_ids(self) -> None:
        rows = parse_main_level(legacy_source())

        self.assertEqual(len(rows), LEGACY_ROWS)
        self.assertEqual(rows[0], (0, 17, 18, 19, 20, 1, 5, 21, *([0] * 8)))
        self.assertEqual(rows[-1], (0,) * 16)
        rendered = render_tmx(rows)
        self.assertIn('width="16" height="255"', rendered)
        self.assertIn('source="dg0.tsx"', rendered)

    def test_rejects_unmapped_legacy_tile(self) -> None:
        with self.assertRaisesRegex(LegacyLevelError, "unmapped tile IDs"):
            parse_main_level(legacy_source("#80,#30"))

    def test_rejects_incomplete_row_sequence(self) -> None:
        source = legacy_source().replace("; row 255", "; row 254")
        with self.assertRaisesRegex(LegacyLevelError, "expected 255"):
            parse_main_level(source)


if __name__ == "__main__":
    unittest.main()
