from pathlib import Path
import unittest

from nurples_tools.active_tiles import REGISTRY, parse, render
from nurples_tools.agnb_verify import parse_images


ROOT = Path(__file__).resolve().parents[1]


class EntryFacingTests(unittest.TestCase):
    def test_opt_in_by_actor_kind_not_marker_name(self):
        spec = ('[slots]\n0=["blank","none"]\n1=["a","a"]\n'
                '2=["alias","a"]\n3=["b","b"]\n')
        for direction, value in (("north", 0), ("east", 64), ("south", 128), ("west", 192)):
            slots = parse(spec + f'[entry_facing]\na="{direction}"')
            self.assertEqual([s.entry_heading for s in slots], [None, value, value, None])
            self.assertTrue(render(slots).endswith(f"    db 255,{value},{value},255\n"))
        for invalid in ('alias="east"', 'unknown="north"', 'a="clockwise"'):
            with self.assertRaises(ValueError):
                parse(spec + "[entry_facing]\n" + invalid)

    def test_only_turret_changes_and_uses_exact_south_art(self):
        slots = parse((ROOT / REGISTRY).read_text())
        self.assertEqual([(s.name, s.entry_heading) for s in slots if s.entry_heading is not None],
                         [("turret_fireball", 128)])
        packed = {im.buffer_id: im for im in parse_images((ROOT / "tgt/game.agnb").read_bytes())}
        for art in (ROOT / "assets/rgba2/tiles/dg/0").glob("*.rgba2"):
            idx = int(art.stem)
            pixels = art.read_bytes()
            self.assertEqual(packed[512 + idx].payload, pixels[::-1] if idx == 5 else pixels)
        self.assertNotEqual(packed[517].payload, packed[517].payload[::-1])

    def test_pose_is_applied_at_lifetime_entry_not_during_scroll(self):
        table = (ROOT / "src/asm/tile_table.inc").read_text()
        for name in ("spawn_active_tile", "spawn_active_tile_from_sprite"):
            body = table.split(f"\n{name}:\n")[1].split(f"; end {name}")[0]
            self.assertLess(body.index("ldir"), body.index("call tile_apply_entry_facing"))
            self.assertLess(body.index("call tile_apply_entry_facing"), body.index("set actor_type_"))
        self.assertNotIn("tile_apply_entry_facing", (ROOT / "src/asm/tiles.inc").read_text())
