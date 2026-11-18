"""Source contracts for the bounded MOS presentation input snapshot."""

from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "asm" / "presentation_input.inc"
FIXTURE = ROOT / "tests" / "asm" / "presentation_input_fixture.asm"
PRODUCTION = ROOT / "src" / "asm" / "nurples.asm"


class PresentationInputTests(unittest.TestCase):
    def test_sampler_takes_one_sysvar_and_one_held_map_snapshot(self) -> None:
        source = SOURCE.read_text()
        code = "\n".join(line.split(";", 1)[0] for line in source.splitlines())
        self.assertEqual(code.count("rst.lil $08"), 1)
        self.assertEqual(code.count("MOSCALL mos_getkbmap"), 1)
        self.assertIn("ld b,16", code)
        self.assertIn("ld a,(ix+sysvar_keyascii)", code)
        self.assertNotIn("waitKeypress", code)
        self.assertNotIn("multiPurposeDelay", code)

    def test_sampler_normalizes_only_y_and_n_for_choice_controller(self) -> None:
        source = SOURCE.read_text()
        self.assertIn("or 020h", source)
        self.assertIn("cp 'y'", source)
        self.assertIn("cp 'n'", source)
        self.assertIn("ld c,START_CHOICE_YES", source)
        self.assertIn("ld c,START_CHOICE_NO", source)
        self.assertIn("ld c,START_CHOICE_NONE", source)

    def test_qualified_sampler_is_in_production(self) -> None:
        self.assertIn('include "presentation_input.inc"', PRODUCTION.read_text())

    def test_fixture_assembles_when_toolchain_is_available(self) -> None:
        assembler = shutil.which("ez80asm")
        if assembler is None:
            self.skipTest("ez80asm is not installed on this host")
        with tempfile.TemporaryDirectory(prefix="nurples-presentation-input-") as temp:
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
