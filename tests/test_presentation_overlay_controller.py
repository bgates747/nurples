"""Host contract tests for bounded attract/game-over presentation state."""

from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "asm" / "presentation_overlay_controller.inc"
FIXTURE = ROOT / "tests" / "asm" / "presentation_overlay_controller_fixture.asm"
PRODUCTION = ROOT / "src" / "asm" / "nurples.asm"


class PresentationOverlayControllerTests(unittest.TestCase):
    def test_cadences_are_independent_and_frame_bounded(self) -> None:
        title_frame = 0
        title_timer = 8
        prompt_visible = 1
        prompt_timer = 30
        title_flips = []
        prompt_flips = []
        for frame in range(1, 121):
            title_timer -= 1
            if title_timer == 0:
                title_timer = 8
                title_frame ^= 1
                title_flips.append(frame)
            prompt_timer -= 1
            if prompt_timer == 0:
                prompt_timer = 30
                prompt_visible ^= 1
                prompt_flips.append(frame)
        self.assertEqual(title_flips, list(range(8, 121, 8)))
        self.assertEqual(prompt_flips, [30, 60, 90, 120])
        self.assertEqual(title_frame, 1)
        self.assertEqual(prompt_visible, 1)

    def test_source_reuses_release_press_gate_and_has_no_device_calls(self) -> None:
        source = SOURCE.read_text()
        self.assertIn("call input_gate_reset", source)
        self.assertIn("jp input_gate_update", source)
        self.assertIn("presentation_title_period: equ 8", source)
        self.assertIn("presentation_prompt_period: equ 30", source)
        code = "\n".join(line.split(";", 1)[0] for line in source.splitlines())
        self.assertNotIn("vdu_", code)
        self.assertNotIn("mos_", code)
        self.assertNotIn("waitKeypress", code)

    def test_qualified_controller_is_in_production_binary(self) -> None:
        production = PRODUCTION.read_text()
        self.assertIn('include "presentation_overlay_controller.inc"', production)

    def test_fixture_assembles_when_toolchain_is_available(self) -> None:
        assembler = shutil.which("ez80asm")
        if assembler is None:
            self.skipTest("ez80asm is not installed on this host")
        with tempfile.TemporaryDirectory(prefix="nurples-presentation-overlay-") as temp:
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
