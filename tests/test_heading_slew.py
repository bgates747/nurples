from pathlib import Path
import shutil
import subprocess
from tempfile import TemporaryDirectory
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SOURCE = PROJECT_ROOT / "src" / "asm" / "heading_slew.inc"
FIXTURE = PROJECT_ROOT / "tests" / "asm" / "heading_slew_fixture.asm"


def reference_slew(current: int, desired: int, step: int) -> tuple[int, bool]:
    current &= 0xFFFF
    desired &= 0xFFFF
    step &= 0xFFFF
    delta = (desired - current) & 0xFFFF
    if delta == 0:
        return desired, True
    increasing = delta <= 0x8000
    magnitude = delta if increasing else (-delta) & 0xFFFF
    if magnitude <= step:
        return desired, True
    if increasing:
        return (current + step) & 0xFFFF, False
    return (current - step) & 0xFFFF, False


class HeadingSlewTests(unittest.TestCase):
    def test_reference_contract_at_wrap_and_half_turn_boundaries(self) -> None:
        self.assertEqual(reference_slew(0xFE00, 0x0200, 0x0100), (0xFF00, False))
        self.assertEqual(reference_slew(0x0200, 0xFE00, 0x0100), (0x0100, False))
        self.assertEqual(reference_slew(0xFF80, 0x0080, 0x0100), (0x0080, True))
        self.assertEqual(reference_slew(0, 0x8000, 0x0100), (0x0100, False))
        self.assertEqual(reference_slew(0x8000, 0, 0x0100), (0x8100, False))

    def test_reference_contract_never_overshoots(self) -> None:
        headings = (0, 1, 0x00FF, 0x0100, 0x7FFF, 0x8000, 0xFF00, 0xFFFF)
        steps = (0, 1, 0x0040, 0x0100, 0x7FFF, 0x8000, 0xFFFF)
        for current in headings:
            for desired in headings:
                for step in steps:
                    with self.subTest(current=current, desired=desired, step=step):
                        result, reached = reference_slew(current, desired, step)
                        before = min(
                            (desired - current) & 0xFFFF,
                            (current - desired) & 0xFFFF,
                        )
                        after = min(
                            (desired - result) & 0xFFFF,
                            (result - desired) & 0xFFFF,
                        )
                        self.assertLessEqual(after, before)
                        self.assertEqual(reached, result == desired)

    def test_qualified_primitive_is_integrated_with_turret_tracking(self) -> None:
        production = (PROJECT_ROOT / "src" / "asm" / "nurples.asm").read_text()
        turret = (PROJECT_ROOT / "src" / "asm" / "turret_tracking.inc").read_text()
        self.assertIn('include "heading_slew.inc"', production)
        self.assertIn('include "turret_tracking.inc"', production)
        self.assertIn("call heading_slew_168", turret)

    def test_fixture_assembles_when_toolchain_is_available(self) -> None:
        assembler = shutil.which("ez80asm")
        if assembler is None:
            self.skipTest("ez80asm is not installed on this host")
        with TemporaryDirectory(prefix="nurples-heading-slew-") as temp_dir:
            output = Path(temp_dir) / "heading_slew_fixture.bin"
            result = subprocess.run(
                [assembler, FIXTURE.name, str(output), "-c"],
                cwd=FIXTURE.parent,
                capture_output=True,
                check=False,
                text=True,
            )
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertGreater(output.stat().st_size, 0)

    def test_source_freezes_half_turn_tie_and_upper_byte_normalization(self) -> None:
        source = SOURCE.read_text()
        self.assertIn("jp z,@increasing", source)
        self.assertGreaterEqual(source.count("inc.s"), 6)
        self.assertIn("heading_slew_within_step:", source)


if __name__ == "__main__":
    unittest.main()
