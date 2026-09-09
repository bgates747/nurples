"""Execute the game's self-modifying joystick gates and keyboard movement."""

import os
from pathlib import Path
import shutil
import subprocess
from tempfile import TemporaryDirectory
import unittest


ROOT = Path(__file__).resolve().parents[1]


class PlayerInputTests(unittest.TestCase):
    def test_joystick_choice_and_keyboard_movement(self) -> None:
        runner = Path(os.environ.get(
            "AGON_HEADLESS_TEST",
            Path.home() / "Agon/mystuff/agon-headless-test",
        )) / "target/debug/agon-headless-test"
        if not runner.is_file():
            self.skipTest("build agon-headless-test or set AGON_HEADLESS_TEST")
        with TemporaryDirectory(prefix="nurples-input-") as temporary:
            staged = Path(temporary)
            (staged / "tests/asm").mkdir(parents=True)
            (staged / "src/asm").mkdir(parents=True)
            for name in ("tests/asm/player_input_fixture.asm", "src/asm/player_input.inc"):
                shutil.copy2(ROOT / name, staged / name)
            binary = staged / "input.bin"
            assembled = subprocess.run(
                [shutil.which("ez80asm") or str(Path.home() / ".local/bin/ez80asm"),
                 "-s", "player_input_fixture.asm", str(binary)],
                cwd=staged / "tests/asm", capture_output=True, text=True,
            )
            self.assertEqual(assembled.returncode, 0, assembled.stdout + assembled.stderr)
            symbols = dict(line.split() for line in
                           (staged / "tests/asm/player_input_fixture.symbols").read_text().splitlines())
            image = binary.read_bytes()
            for label in ("player_joystick_direction_gate", "player_joystick_fire_gate"):
                offset = int(symbols[label].removeprefix("$"), 16) - 0x040000
                self.assertEqual(image[offset:offset + 2], b"\0\0", label)
            executed = subprocess.run(
                [str(runner), "run-direct", "--binary", str(binary),
                 "--load-address", "0x040000", "--entry-address", "0x040000",
                 "--adl", "true", "--madl", "false", "--mbase", "0",
                 "--max-instructions", "25000", "--max-cycles", "200000",
                 "--wall-timeout-ms", "2000", "--expect-guest-status", "0x2a"],
                capture_output=True, text=True, timeout=10,
            )
            self.assertEqual(executed.returncode, 0, executed.stdout + executed.stderr)
            self.assertIn("PASS direct guest status 0x2a matched", executed.stdout)


if __name__ == "__main__":
    unittest.main()
