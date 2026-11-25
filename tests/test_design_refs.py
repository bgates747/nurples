from pathlib import Path
import unittest
import xml.etree.ElementTree as ET
from nurples_tools.design_refs import validate


class DesignReferences(unittest.TestCase):
    def test_checkout(self):
        validate(Path(__file__).resolve().parents[1] / "level-design")

    def test_every_level_has_the_authoring_layers(self):
        root = Path(__file__).resolve().parents[1]
        for directory in ("level-design", "tests/maps", "tiles"):
            for p in (root/directory).rglob("*.tmx"):
                layers = [e.get("name") for e in ET.parse(p).getroot()
                          if e.tag in ("layer", "imagelayer", "objectgroup", "group")]
                self.assertEqual(layers, ["Background", "Tiles", "Spawns"], p)
