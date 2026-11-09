from io import BytesIO
from pathlib import Path
import unittest

from PIL import Image

from nurples_tools.overlay_art import (
    generate_overlay_art,
    game_over_drawing,
    overlay_drawings,
    palette_colors,
)


ROOT = Path(__file__).resolve().parents[1]


class OverlayArtTests(unittest.TestCase):
    def test_generated_assets_are_current(self):
        for relative, expected in generate_overlay_art().items():
            self.assertEqual((ROOT / relative).read_bytes(), expected, relative)

    def test_all_source_colors_are_exact_agon64_with_binary_alpha(self):
        colors = palette_colors(overlay_drawings().values())
        self.assertTrue(colors)
        for red, green, blue, alpha in colors:
            self.assertIn(red, (0, 85, 170, 255))
            self.assertIn(green, (0, 85, 170, 255))
            self.assertIn(blue, (0, 85, 170, 255))
            self.assertEqual(alpha, 255)

    def test_pngs_are_transparent_and_need_no_palette_transform(self):
        pngs = {
            path: payload for path, payload in generate_overlay_art().items()
            if path.suffix == ".png"
        }
        self.assertEqual(len(pngs), 4)
        for path, payload in pngs.items():
            with Image.open(BytesIO(payload)) as image:
                self.assertEqual(image.mode, "RGBA")
                colors = set(image.get_flattened_data())
                self.assertIn((0, 0, 0, 0), colors, path)
                for red, green, blue, alpha in colors:
                    self.assertIn(red, (0, 85, 170, 255))
                    self.assertIn(green, (0, 85, 170, 255))
                    self.assertIn(blue, (0, 85, 170, 255))
                    self.assertIn(alpha, (0, 255))

    def test_selected_dimensions_fit_the_320_by_240_playfield(self):
        drawings = overlay_drawings()
        self.assertEqual((drawings["game_over_0"].width,
                          drawings["game_over_0"].height), (240, 64))
        self.assertEqual((drawings["game_over_1"].width,
                          drawings["game_over_1"].height), (240, 64))
        self.assertEqual((drawings["press_any_key_to_continue"].width,
                          drawings["press_any_key_to_continue"].height), (160, 16))
        self.assertEqual((drawings["press_any_key_to_start"].width,
                          drawings["press_any_key_to_start"].height), (160, 16))

    def test_game_over_frames_change_color_not_geometry(self):
        first = game_over_drawing(0)
        second = game_over_drawing(1)
        self.assertEqual(len(first.shapes), len(second.shapes))
        first_geometry = [shape.__dict__ | {"color": None} for shape in first.shapes]
        second_geometry = [shape.__dict__ | {"color": None} for shape in second.shapes]
        self.assertEqual(first_geometry, second_geometry)
        self.assertNotEqual(first.shapes, second.shapes)


if __name__ == "__main__":
    unittest.main()
