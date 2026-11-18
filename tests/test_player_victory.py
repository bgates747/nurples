"""Host contract tests for the actor-owned victory flight."""

from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "asm" / "player_victory.inc"
FIXTURE = ROOT / "tests" / "asm" / "player_victory_fixture.asm"
PRODUCTION = ROOT / "src" / "asm" / "nurples.asm"


class PlayerVictoryTests(unittest.TestCase):
    def test_loop_is_one_complete_bounded_heading_cycle(self) -> None:
        heading = 192 * 256
        observed = []
        for frame in range(64):
            observed.append((heading, 0 if frame < 32 else 2))
            heading = (heading + 4 * 256) & 0xFFFF
        self.assertEqual(heading, 192 * 256)
        self.assertEqual(sum(frame == 0 for _, frame in observed), 32)
        self.assertEqual(sum(frame == 2 for _, frame in observed), 32)

    def test_source_owns_named_actor_fields_and_collision_free_path(self) -> None:
        source = SOURCE.read_text()
        self.assertIn("player_victory_phase: equ player_custom_end", source)
        self.assertIn("player_victory_timer: equ player_victory_phase+1", source)
        self.assertIn("player_victory_loop_frames: equ 64", source)
        self.assertIn("res sprite_alive,(iy+sprite_collisions)", source)
        self.assertIn("call bearing_to_coord", source)
        self.assertIn("call sprite_compute_velocities", source)
        self.assertIn("call move_sprite", source)
        self.assertNotIn("check_enemy_collision", source)
        self.assertNotIn("player_input", source)

    def test_source_centres_before_reporting_hold(self) -> None:
        source = SOURCE.read_text()
        hold = source.index("@begin_hold:")
        zero_x = source.index("ld (iy+sprite_xvel),hl", hold)
        zero_y = source.index("ld (iy+sprite_yvel),hl", hold)
        report = source.index("@hold:\n    scf", hold)
        self.assertLess(zero_x, report)
        self.assertLess(zero_y, report)

    def test_victory_flight_is_integrated_as_a_player_state(self) -> None:
        production = PRODUCTION.read_text()
        state = (ROOT / "src" / "asm" / "state.inc").read_text()
        player = (ROOT / "src" / "asm" / "player_state.inc").read_text()
        self.assertIn('include "player_victory.inc"', production)
        self.assertIn("call player_victory_init", state)
        self.assertIn("ld hl,player_victory", state)
        self.assertIn("ld de,player_victory", player)

    def test_fixture_assembles_when_toolchain_is_available(self) -> None:
        assembler = shutil.which("ez80asm")
        if assembler is None:
            self.skipTest("ez80asm is not installed on this host")
        with tempfile.TemporaryDirectory(prefix="nurples-player-victory-asm-") as temp:
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


if __name__ == "__main__":
    unittest.main()
