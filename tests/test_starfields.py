from hashlib import sha256
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

from PIL import Image

from nurples_tools.starfields import check_starfields, generate_starfields, write_starfields


ROOT = Path(__file__).resolve().parents[1]
# Original RGBA2222 payloads at 0c741d5e, immediately before the atlas replacement.
ORIGINALS = {
    0: "24a6d1cc312b1be9c6260e3ca823d1791951a80db5ec8d6146d8a20ff88f5c82",
    3: "bc7d2e6861100af839e3182c05b4d40a3f1fc03d2c82dce6a62051126865634a",
    4: "8f8ef26cee87f3a04785fba0146512ad2d4c4f865f7723f80b524bee6e26c34e",
    6: "3de8843a3f3d516697094be02272a6bcac78b72c195109cfbdd4fc608945e915",
}
# Smith-approved retouch of the improved artwork, kept as native RGBA pixels.
RETOUCHED = {
    0: "a69c6e87e7e04a17e205b05e26a84acb758651bb77dc06ecad9bba369a988fca",
    3: "5ccb3f73b9f912dcfaa25c15572656f182a23066247878838b05a14f9fe3a572",
    4: "7994a4d198d5b053c9afc444616572ca30430ece30db0a68fe99a573e981c194",
    6: "66d726b57b7e8dd656efb2461e6f009205ffdd47b27b09b00eba6d57c41d3fa5",
}
CONTROL = ROOT / "docs/tasks/ART-001/starfield-join-review/original/source"


class StarfieldTests(unittest.TestCase):
    def check_clean_generation(self, authored, expected_hashes):
        with TemporaryDirectory() as temporary:
            root = Path(temporary)
            sources = {}
            for number in expected_hashes:
                relative = Path(f"tiles/bg/orig/bg_stars_{number}.png")
                sources[relative] = (authored / relative.name).read_bytes()
                (root / relative).parent.mkdir(parents=True, exist_ok=True)
                (root / relative).write_bytes(sources[relative])
            self.assertEqual(len(write_starfields(root)), 8)
            self.assertEqual(check_starfields(root), [])
            for number, expected in expected_hashes.items():
                with self.subTest(tile=number):
                    raw = (root / f"assets/rgba2/bg/bg_stars_{number}.rgba2").read_bytes()
                    self.assertEqual(len(raw), 256 * 256)
                    self.assertEqual(sha256(raw).hexdigest(), expected)
                    preview = root / f"tiles/bg/proc/bg_stars_{number}.png"
                    with Image.open(preview) as image:
                        self.assertEqual(image.size, (256, 256))
            for relative, before in sources.items():
                self.assertEqual((root / relative).read_bytes(), before)

    def test_clean_generation_preserves_approved_retouch_without_atlas(self):
        self.check_clean_generation(ROOT / "tiles/bg/orig", RETOUCHED)

    def test_original_control_still_encodes_to_historical_payloads(self):
        self.check_clean_generation(CONTROL, ORIGINALS)

    def test_wrong_dimensions_are_rejected_instead_of_resized(self):
        with TemporaryDirectory() as temporary:
            root = Path(temporary)
            path = root / "tiles/bg/orig/bg_stars_0.png"
            path.parent.mkdir(parents=True)
            Image.new("RGBA", (256, 255)).save(path)
            with self.assertRaisesRegex(ValueError, "bg_stars_0.png must be 256x256"):
                generate_starfields(root)


if __name__ == "__main__":
    unittest.main()
