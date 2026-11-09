from hashlib import sha256
from pathlib import Path
import unittest

from nurples_tools.lode_font import (
    BASE_FONT_NAME,
    BASE_FONT_SHA256,
    LODE_UPPERCASE_ROWS,
    OUTPUT_FONT_NAME,
    build_hybrid_font,
    generate_lode_font,
)


ROOT = Path(__file__).resolve().parents[1]


class LodeFontTests(unittest.TestCase):
    def setUp(self):
        self.base = (ROOT / "assets/fonts" / BASE_FONT_NAME).read_bytes()
        self.hybrid = build_hybrid_font(self.base)

    def test_pinned_lat38_base(self):
        self.assertEqual(sha256(self.base).hexdigest(), BASE_FONT_SHA256)

    def test_exact_uppercase_rows_are_eight_by_eight(self):
        self.assertEqual(set(LODE_UPPERCASE_ROWS), set("ABCDEFGHIJKLMNOPQRSTUVWXYZ"))
        for rows in LODE_UPPERCASE_ROWS.values():
            self.assertEqual(len(rows), 8)
            self.assertTrue(all(0 <= row <= 0xFF for row in rows))

    def test_uppercase_and_lowercase_slots_are_exact_aliases(self):
        for character, rows in LODE_UPPERCASE_ROWS.items():
            upper = self.hybrid[ord(character) * 8:ord(character) * 8 + 8]
            lower = self.hybrid[ord(character.lower()) * 8:ord(character.lower()) * 8 + 8]
            self.assertEqual(upper, bytes(rows))
            self.assertEqual(lower, bytes(rows))

    def test_every_other_glyph_remains_lat38(self):
        changed = {
            ord(character)
            for character in LODE_UPPERCASE_ROWS
            for character in (character, character.lower())
        }
        for code in range(256):
            if code not in changed:
                offset = code * 8
                self.assertEqual(
                    self.hybrid[offset:offset + 8],
                    self.base[offset:offset + 8],
                    code,
                )

    def test_generated_assets_are_current(self):
        outputs = generate_lode_font(ROOT)
        self.assertIn(Path("assets/fonts") / OUTPUT_FONT_NAME, outputs)
        for relative, expected in outputs.items():
            self.assertEqual((ROOT / relative).read_bytes(), expected, relative)

    def test_supported_asset_orchestrators_own_generation_and_check(self):
        for relative in (
            "build/scripts/build_00_all_the_things.py",
            "build/scripts/build_current_assets.py",
        ):
            source = (ROOT / relative).read_text()
            with self.subTest(relative=relative):
                self.assertIn("write_lode_font(PROJECT_ROOT)", source)
                self.assertIn("check_lode_font(PROJECT_ROOT)", source)


if __name__ == "__main__":
    unittest.main()
