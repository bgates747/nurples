"""Host contract tests for actor-owned rate-limited turret tracking."""

from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "asm" / "turret_tracking.inc"
FIXTURE = ROOT / "tests" / "asm" / "turret_tracking_fixture.asm"
PRODUCTION = ROOT / "src" / "asm" / "nurples.asm"


def slew(current: int, desired: int, step: int = 2 * 256) -> int:
    current &= 0xFFFF
    desired &= 0xFFFF
    delta = (desired - current) & 0xFFFF
    if delta == 0:
        return desired
    if delta <= 0x8000:
        return desired if delta <= step else (current + step) & 0xFFFF
    magnitude = (-delta) & 0xFFFF
    return desired if magnitude <= step else (current - step) & 0xFFFF


class TurretTrackingTests(unittest.TestCase):
    def test_live_tracking_is_bounded_across_wrap(self) -> None:
        self.assertEqual(slew(0xFE00, 0x0200), 0x0000)
        self.assertEqual(slew(0x0200, 0xFE00), 0x0000)
        self.assertEqual(slew(0x1000, 0x1000), 0x1000)
        self.assertEqual(slew(0x0000, 0x8000), 0x0200)

    def test_search_latches_direction_for_its_own_duration(self) -> None:
        heading = 0
        timer = 2
        direction = 0
        observed = []
        for _ in range(2):
            desired = (heading + 64 * 256) & 0xFFFF
            heading = slew(heading, desired)
            timer -= 1
            observed.append((heading, timer, direction))
        direction ^= 1
        timer = 24
        desired = (heading - 64 * 256) & 0xFFFF
        heading = slew(heading, desired)
        timer -= 1
        observed.append((heading, timer, direction))
        self.assertEqual(observed, [(0x0200, 1, 0), (0x0400, 0, 0), (0x0200, 23, 1)])

    def test_source_uses_named_actor_fields_and_physical_shot_heading(self) -> None:
        source = SOURCE.read_text()
        self.assertIn("turret_desired_heading: equ turret_tracking_custom_begin", source)
        self.assertIn("turret_search_direction: equ turret_desired_heading+3", source)
        self.assertIn("turret_search_timer: equ turret_search_direction+1", source)
        self.assertIn("call heading_slew_168", source)
        self.assertIn("ld hl,(iy+tile_heading)", source)
        self.assertIn("ld (Heading_p),hl", source)
        self.assertIn("call polar_to_cartesian", source)
        self.assertNotIn("tile_move_timer", source)

    def test_tracking_is_integrated_at_turret_lifecycle_boundaries(self) -> None:
        production = PRODUCTION.read_text()
        turret = (ROOT / "src" / "asm" / "tile_turret_fireball.inc").read_text()
        self.assertIn('include "turret_tracking.inc"', production)
        self.assertIn('include "heading_slew.inc"', production)
        self.assertIn("call turret_tracking_init", turret)
        self.assertIn("call turret_track_desired_heading", turret)
        self.assertIn("call turret_prepare_physical_shot", turret)
        self.assertIn("call turret_search_update", turret)
        self.assertLess(
            turret.index("call turret_prepare_physical_shot"),
            turret.index("call enemy_spawn_fireball"),
        )

    def test_fixture_assembles_when_toolchain_is_available(self) -> None:
        assembler = shutil.which("ez80asm")
        if assembler is None:
            self.skipTest("ez80asm is not installed on this host")
        with tempfile.TemporaryDirectory(prefix="nurples-turret-tracking-asm-") as temp:
            output = Path(temp) / "fixture.bin"
            result = subprocess.run(
                [assembler, FIXTURE.name, str(output), "-c"],
                cwd=FIXTURE.parent,
                capture_output=True,
                check=False,
                text=True,
            )
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertGreater(output.stat().st_size, 0)

    def test_fireball_uses_this_frames_terrain_displacement_from_the_muzzle(self) -> None:
        def instructions(file: str) -> str:
            lines = (SOURCE.parent / file).read_text().splitlines()
            return "\n".join(code for line in lines if (code := line.split(";")[0].strip()))

        game = instructions("nurples.asm")
        terrain = instructions("tiles.inc")
        fireball = instructions("enemy_fireball.inc")
        self.assertIn("xor a\nld (terrain_step_pixels),a\ncall do_game", game)
        self.assertIn("ld h,1\nld a,h\nld (terrain_step_pixels),a\nld a,2\ncall vdu_scroll_down", terrain)
        self.assertIn("ld hl,(Vp_y)\nld (ix+fireball_ground_vy),hl\ncall fireball_to_screen_vy\n"
                      "ld (ix+sprite_yvel),hl", fireball)
        self.assertIn("ld hl,(ix+sprite_y)\nld de,(ix+sprite_yvel)\nadd hl,de\nld (ix+sprite_y),hl",
                      fireball)
        self.assertIn("ld hl,(iy+fireball_ground_vy)\ncall fireball_to_screen_vy\n"
                      "ld (iy+sprite_yvel),hl\ncall move_sprite", fireball)
        self.assertIn("fireball_to_screen_vy:\nld de,0\nld a,(terrain_step_pixels)\n"
                      "ld d,a\nadd hl,de\nret", fireball)


if __name__ == "__main__":
    unittest.main()
