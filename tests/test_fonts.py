from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

from nurples_tools.fonts import (
    build_fonts,
    check_fonts,
    discover_fonts,
    render_fonts_manifest,
)


PROJECT_ROOT = Path(__file__).resolve().parents[1]
FONT_SOURCE = PROJECT_ROOT / "assets/fonts"


class FontBuildTests(unittest.TestCase):
    def test_runtime_font_set_contains_only_the_game_font(self) -> None:
        fonts = discover_fonts(FONT_SOURCE)
        self.assertEqual(len(fonts), 1)
        self.assertEqual(fonts[0].path.name, "Lat38-VGA8_8x8.font")

    def test_manifest_uses_runtime_font_directory(self) -> None:
        manifest = render_fonts_manifest(discover_fonts(FONT_SOURCE))
        self.assertIn(
            'fn_Lat38_VGA8_8x8: db "fonts/Lat38-VGA8_8x8.font",0',
            manifest,
        )
        self.assertIn("Lat38_VGA8_8x8: equ 64000", manifest)
        self.assertNotIn("amiga_forever", manifest)
        self.assertNotIn('db "fontsLat38', manifest)

    def test_build_and_check_are_deterministic(self) -> None:
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            target = root / "tgt/fonts"
            manifest = root / "src/asm/fonts_list.inc"
            build_fonts(FONT_SOURCE, target, manifest)

            first_manifest = manifest.read_bytes()
            build_fonts(FONT_SOURCE, target, manifest)

            self.assertEqual(manifest.read_bytes(), first_manifest)
            self.assertEqual(check_fonts(FONT_SOURCE, target, manifest), [])


if __name__ == "__main__":
    unittest.main()
