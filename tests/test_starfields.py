from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

from PIL import Image

from nurples_tools.images import rgba2_to_img
from nurples_tools.starfields import EDGE_MARGIN, TILE_NAMES, slice_starfield


ROOT = Path(__file__).resolve().parents[1]


class StarfieldTests(unittest.TestCase):
    def test_slice_order_preserves_initial_vertical_pair(self):
        atlas = Image.new("RGB", (512, 512))
        colors = [(255, 0, 0), (0, 255, 0), (0, 0, 255), (255, 255, 0)]
        for index, color in enumerate(colors):
            x, y = index % 2 * 256, index // 2 * 256
            atlas.paste(color, (x, y, x + 256, y + 256))
        with TemporaryDirectory() as temporary:
            path = Path(temporary) / "atlas.png"
            atlas.save(path)
            tiles = slice_starfield(path)
        for name, color in zip(TILE_NAMES, colors, strict=True):
            self.assertEqual(tiles[name].getpixel((128, 128)), (*color, 255))
        self.assertEqual(tiles["bg_stars_0"].getpixel((128, 128)), (255, 0, 0, 255))
        self.assertEqual(tiles["bg_stars_4"].getpixel((128, 128)), (0, 0, 255, 255))

    def test_runtime_tiles_are_sparse_opaque_and_all_seams_match(self):
        images = []
        background = (0, 0, 85, 255)
        for name in TILE_NAMES:
            path = ROOT / f"assets/rgba2/bg/{name}.rgba2"
            self.assertEqual(path.stat().st_size, 65536)
            image = rgba2_to_img(path, 256, 256)
            pixels = list(image.get_flattened_data())
            # Increased star density still leaves at least 94% quiet blue space.
            self.assertLess(sum(pixel != background for pixel in pixels) / len(pixels), .06)
            for r, g, b, a in set(pixels):
                self.assertTrue({r, g, b} <= {0, 85, 170, 255})
                self.assertEqual(a, 255)
            for box in [(0, 0, 256, EDGE_MARGIN), (0, 256-EDGE_MARGIN, 256, 256),
                        (0, 0, EDGE_MARGIN, 256), (256-EDGE_MARGIN, 0, 256, 256)]:
                self.assertEqual(set(image.crop(box).get_flattened_data()), {background})
            images.append(image)
        # Random selection includes repeated tiles and all 16 ordered pairs.
        for above in images:
            for below in images:
                self.assertEqual(above.crop((0, 255, 256, 256)).tobytes(),
                                 below.crop((0, 0, 256, 1)).tobytes())
                self.assertEqual(above.crop((255, 0, 256, 256)).tobytes(),
                                 below.crop((0, 0, 1, 256)).tobytes())

    def test_nonsquare_master_rejected(self):
        with TemporaryDirectory() as temporary:
            path = Path(temporary) / "bad.png"
            Image.new("RGB", (64, 32)).save(path)
            with self.assertRaisesRegex(ValueError, "must be square"):
                slice_starfield(path)


if __name__ == "__main__":
    unittest.main()
