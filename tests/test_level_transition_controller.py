"""Host contract tests for the bounded level-transition speed schedule."""

from __future__ import annotations

from collections import Counter
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "asm" / "level_transition_controller.inc"
FIXTURE = ROOT / "tests" / "asm" / "level_transition_controller_fixture.asm"
PRODUCTION = ROOT / "src" / "asm" / "nurples.asm"

DRAIN = 336
CENTER = 90
ACCEL = (1, 2, 3, 4, 6, 8, 12, 16)
HOLD = 90
DECEL = (12, 8, 6, 4, 3, 2, 1)
SPEED_FRAMES = 8


def schedule() -> list[tuple[int, bool]]:
    result = [(1, True)] * DRAIN
    result.extend([(1, False)] * CENTER)
    for speed in ACCEL:
        result.extend([(speed, False)] * SPEED_FRAMES)
    result.extend([(16, False)] * HOLD)
    for speed in DECEL:
        result.extend([(speed, False)] * SPEED_FRAMES)
    return result


class LevelTransitionControllerTests(unittest.TestCase):
    def test_schedule_is_bounded_and_returns_to_normal_speed(self) -> None:
        frames = schedule()
        self.assertEqual(len(frames), 636)
        self.assertEqual(sum(terrain for _, terrain in frames), DRAIN)
        self.assertEqual(frames[0], (1, True))
        self.assertEqual(frames[DRAIN], (1, False))
        self.assertEqual(frames[-1], (1, False))
        self.assertTrue(all(1 <= speed <= 16 for speed, _ in frames))

    def test_speed_histogram_freezes_accel_hold_and_decel(self) -> None:
        counts = Counter(speed for speed, _ in schedule())
        self.assertEqual(counts[1], 442)
        for speed in (2, 3, 4, 6, 8, 12):
            self.assertEqual(counts[speed], 16)
        self.assertEqual(counts[16], 98)

    def test_source_separates_frame_policy_from_next_phase_mutation(self) -> None:
        source = SOURCE.read_text()
        self.assertIn("level_transition_frame_speed", source)
        self.assertIn("level_transition_frame_policy", source)
        self.assertIn("LEVEL_TRANSITION_POLICY_TERRAIN", source)
        self.assertIn("level_transition_total_frames-636", source)
        code = "\n".join(line.split(";", 1)[0] for line in source.splitlines())
        self.assertNotIn("tiles_plot", code)
        self.assertNotIn("starfield_warp_scroll", code)
        self.assertNotIn("vdu_", code)

    def test_controller_is_integrated_without_generalizing_terrain(self) -> None:
        production = PRODUCTION.read_text()
        state = (ROOT / "src" / "asm" / "state.inc").read_text()
        terrain = (ROOT / "src" / "asm" / "tiles.inc").read_text()
        self.assertIn('include "level_transition_controller.inc"', production)
        self.assertIn("call level_transition_step", state)
        self.assertIn("call starfield_warp_scroll", state)
        self.assertIn("call tiles_plot", state)
        self.assertNotIn("starfield_warp_scroll", terrain)

    def test_fixture_assembles_when_toolchain_is_available(self) -> None:
        assembler = shutil.which("ez80asm")
        if assembler is None:
            self.skipTest("ez80asm is not installed on this host")
        with tempfile.TemporaryDirectory(prefix="nurples-level-transition-asm-") as temp:
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
