"""Contracts for the bounded start-flow joystick choice controller."""

from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "asm" / "start_choice_controller.inc"
FIXTURE = ROOT / "tests" / "asm" / "start_choice_controller_fixture.asm"
PRODUCTION = ROOT / "src" / "asm" / "nurples.asm"


class StartChoiceControllerTests(unittest.TestCase):
    def test_source_is_bounded_device_independent_and_reuses_input_gate(self) -> None:
        source = SOURCE.read_text()
        self.assertIn("call input_gate_update", source)
        self.assertIn("jp input_gate_reset", source)
        self.assertIn("START_CHOICE_YES:  equ 1", source)
        self.assertIn("START_CHOICE_NO:   equ 2", source)
        code = "\n".join(line.split(";", 1)[0] for line in source.splitlines())
        self.assertNotIn("waitKeypress", code)
        self.assertNotIn("mos_", code)
        self.assertNotIn("vdu_", code)

    def test_reference_trace_requires_release_and_valid_fresh_choice(self) -> None:
        wait_release = 0
        armed = 1
        gate = wait_release
        result = 0

        def update(any_down: int, choice: int) -> bool:
            nonlocal gate, result
            if not any_down:
                gate = armed
                return False
            if gate != armed:
                return False
            gate = wait_release
            if choice not in (1, 2):
                return False
            result = choice
            return True

        self.assertFalse(update(1, 1))  # held attract key
        self.assertFalse(update(0, 0))
        self.assertFalse(update(1, 0))  # unrelated fresh key
        self.assertFalse(update(1, 2))  # continued hold
        self.assertFalse(update(0, 0))
        self.assertTrue(update(1, 2))
        self.assertEqual(result, 2)
        self.assertFalse(update(1, 2))

    def test_qualified_controller_is_in_production(self) -> None:
        self.assertIn(
            'include "start_choice_controller.inc"', PRODUCTION.read_text()
        )

    def test_fixture_assembles_when_toolchain_is_available(self) -> None:
        assembler = shutil.which("ez80asm")
        if assembler is None:
            self.skipTest("ez80asm is not installed on this host")
        with tempfile.TemporaryDirectory(prefix="nurples-start-choice-") as temp:
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
