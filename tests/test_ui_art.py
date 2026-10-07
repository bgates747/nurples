from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

from PIL import Image

from nurples_tools.ui_art import prepare_image, remove_logo_checkerboard, ui_manifest
from nurples_tools.images import rgba2_to_img


ROOT = Path(__file__).resolve().parents[1]


class UiArtTests(unittest.TestCase):
    def test_gray_background_removed_but_purple_and_highlights_survive(self):
        image = Image.new("RGBA", (6, 1))
        image.putdata([
            (140, 140, 140, 255), (220, 220, 220, 255),
            (255, 0, 255, 255), (55, 0, 55, 255),
            (255, 170, 255, 255), (255, 0, 255, 0),
        ])
        result = remove_logo_checkerboard(image)
        self.assertEqual(list(result.getchannel("A").get_flattened_data()),
                         [0, 0, 255, 255, 255, 0])

    def test_resampling_has_binary_alpha_and_no_gray_fringe(self):
        image = Image.new("RGB", (32, 32), (180, 180, 180))
        image.paste((255, 0, 255), (8, 8, 24, 24))
        with TemporaryDirectory() as temporary:
            path = Path(temporary) / "logo.png"
            image.save(path)
            result = prepare_image(path, (7, 7), logo=True)
        self.assertEqual(set(result.getchannel("A").get_flattened_data()), {0, 255})
        for r, g, b, a in result.get_flattened_data():
            if a:
                self.assertEqual(g, 0)
                self.assertGreater(r, 240)
                self.assertGreater(b, 240)

    def test_replacements_preserve_runtime_slots_and_dimensions(self):
        prepared = [entry for entry in ui_manifest(ROOT) if entry.get("prepare")]
        self.assertEqual(
            [(e["name"], e["buffer_id"], e["width"], e["height"]) for e in prepared],
            [("bezel_l", 6, 128, 304), ("bezel_r", 7, 128, 304),
             ("splash_bg", 44, 320, 240), ("splash_logo", 45, 160, 106)],
        )
        for entry in prepared:
            path = ROOT / f"assets/rgba2/ui/{entry['name']}.rgba2"
            w, h = entry["width"], entry["height"]
            self.assertEqual(path.stat().st_size, w * h)
            decoded = rgba2_to_img(path, w, h)
            colors = set(decoded.get_flattened_data())
            for r, g, b, a in colors:
                self.assertTrue({r, g, b} <= {0, 85, 170, 255})
                self.assertIn(a, (0, 255))
            if entry["name"] == "splash_logo":
                self.assertEqual(decoded.getpixel((0, 0))[3], 0)
                self.assertIn(255, set(decoded.getchannel("A").get_flattened_data()))
            else:
                self.assertEqual(decoded.getchannel("A").getextrema(), (255, 255))


if __name__ == "__main__":
    unittest.main()
