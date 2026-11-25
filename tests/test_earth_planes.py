"""Compare the deployed artwork and compiled map planes with the Tiled inputs."""
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest
import xml.etree.ElementTree as ET

from PIL import Image

from nurples_tools.agnb_verify import parse_images
from nurples_tools.earth_city import EARTH_TILE_BASE
from nurples_tools.images import img_to_rgba2
from nurples_tools.maps import compile_map_manifest


ROOT = Path(__file__).resolve().parents[1]


class EarthPlaneTests(unittest.TestCase):
    def test_deployed_planes_match_tiled(self):
        earth = compile_map_manifest(ROOT / "maps/campaign.toml").levels[0]
        tmx = ET.parse(earth.source_path)
        base = earth.source_path.parent
        for layer, rows in (("Tiles", earth.runtime_rows), ("Spawns", earth.spawn_rows)):
            self.assertEqual([v for row in reversed(rows) for v in row],
                             list(map(int, tmx.findtext(f"layer[@name='{layer}']/data").split(","))))
        bank = {im.buffer_id: im.payload for im in parse_images((ROOT / "tgt/game.agnb").read_bytes())}
        bg = Image.open(base / tmx.find("imagelayer/image").get("source")).convert("RGBA")
        tsx = base / tmx.find("tileset").get("source")
        atlas = Image.open(tsx.parent / ET.parse(tsx).find("image").get("source")).convert("RGBA")
        with TemporaryDirectory() as tmp:
            encoded = Path(tmp) / "pixels"
            img_to_rgba2(bg, encoded)
            self.assertEqual(b"".join(bank[1280+i] for i in reversed(range(16))), encoded.read_bytes())
            self.assertEqual(bank[EARTH_TILE_BASE], bytes(256))
            for gid in {v for row in earth.runtime_rows for v in row if v}:
                x, y = (gid-1)%16*16, (gid-1)//16*16
                img_to_rgba2(atlas.crop((x, y, x+16, y+16)), encoded)
                self.assertEqual(bank[EARTH_TILE_BASE+gid], encoded.read_bytes(), gid)
