from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

import agonutils
from PIL import Image

from nurples_tools.images import (
    convert_to_agon_palette,
    img_to_rgba2,
    img_to_rgba8,
    rgba2_to_img,
    rgba8_to_img,
)


class PythonEnvironmentTests(unittest.TestCase):
    def test_canonical_agonutils_is_not_shadowed(self) -> None:
        module_path = Path(agonutils.__file__).resolve()
        self.assertEqual(module_path.name.split(".")[0], "agonutils")
        self.assertIn("/home/smith/Agon/mystuff/agon-utils/", str(module_path))

    def test_rgba8_round_trip(self) -> None:
        image = Image.new("RGBA", (2, 1))
        image.putdata([(0, 85, 170, 255), (255, 170, 85, 0)])

        with TemporaryDirectory() as temp_dir:
            raw_path = Path(temp_dir) / "image.rgba8"
            img_to_rgba8(image, raw_path)
            decoded = rgba8_to_img(raw_path, 2, 1)

        self.assertEqual(
            list(decoded.get_flattened_data()),
            list(image.get_flattened_data()),
        )

    def test_rgba8_rejects_wrong_payload_size(self) -> None:
        with TemporaryDirectory() as temp_dir:
            raw_path = Path(temp_dir) / "short.rgba8"
            raw_path.write_bytes(b"\x00" * 7)
            with self.assertRaisesRegex(ValueError, "expected 8"):
                rgba8_to_img(raw_path, 2, 1)

    def test_rgba2_uses_canonical_encoder_and_decoder(self) -> None:
        image = Image.new("RGBA", (4, 1))
        image.putdata(
            [
                (42, 42, 42, 255),
                (43, 43, 43, 255),
                (212, 212, 212, 255),
                (213, 213, 213, 255),
            ]
        )

        with TemporaryDirectory() as temp_dir:
            raw_path = Path(temp_dir) / "image.rgba2"
            img_to_rgba2(image, raw_path)
            self.assertEqual(raw_path.read_bytes(), bytes((192, 213, 234, 255)))
            decoded = rgba2_to_img(raw_path, 4, 1)

        self.assertEqual(
            list(decoded.get_flattened_data()),
            [
                (0, 0, 0, 255),
                (85, 85, 85, 255),
                (170, 170, 170, 255),
                (255, 255, 255, 255),
            ],
        )

    def test_rgba2_alpha_quantization_is_explicit(self) -> None:
        image = Image.new("RGBA", (3, 1))
        image.putdata(
            [
                (255, 0, 0, 0),
                (0, 255, 0, 128),
                (0, 0, 255, 255),
            ]
        )

        with TemporaryDirectory() as temp_dir:
            raw_path = Path(temp_dir) / "alpha.rgba2"
            img_to_rgba2(image, raw_path)
            self.assertEqual(raw_path.read_bytes(), bytes((3, 140, 240)))
            decoded = rgba2_to_img(raw_path, 3, 1)

        self.assertEqual(
            list(decoded.get_flattened_data()),
            [
                (255, 0, 0, 0),
                (0, 255, 0, 170),
                (0, 0, 255, 255),
            ],
        )

    def test_palette_adapter_accepts_lowercase_method(self) -> None:
        image = Image.new("RGBA", (1, 1), (250, 10, 10, 255))
        converted = convert_to_agon_palette(image, 64, "rgb")
        self.assertEqual(converted.getpixel((0, 0)), (255, 0, 0, 255))


if __name__ == "__main__":
    unittest.main()
