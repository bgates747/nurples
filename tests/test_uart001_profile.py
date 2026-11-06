from __future__ import annotations

import importlib.util
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "tools/stage_uart001_oryx.py"


def load_stage_module():
    spec = importlib.util.spec_from_file_location("stage_uart001_oryx", SCRIPT)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Could not load {SCRIPT}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class Uart001ProfileTests(unittest.TestCase):
    def test_profile_is_stopped_and_loads_without_running(self) -> None:
        module = load_stage_module()
        self.assertIn(b"SET KEYBOARD 1\r\n", module.AUTOEXEC)
        self.assertIn(b"load nurples-uart001.bin\r\n", module.AUTOEXEC)
        self.assertNotIn(b"run", module.AUTOEXEC.lower())

    def test_rc3_identity_and_required_payload_are_pinned(self) -> None:
        module = load_stage_module()
        self.assertEqual(
            module.RC3_RUNTIME_SHA256["mos_platform.bin"],
            "d7a51147f3381cd68d56f3fbe4605434a897dfae1b049269afeba09ec2b7eb50",
        )
        self.assertEqual(
            module.RC3_RUNTIME_SHA256["mos_platform.map"],
            "48dae5890af7b49fe6bcb535a20e9b6432069c8d84666e3d0e8e7e1070282be9",
        )
        self.assertEqual(
            module.FIXTURE_FILES,
            (
                "nurples-uart001.bin",
                "nurples-uart001.symbols",
                "game.agnb",
                "ui.agnb",
            ),
        )


if __name__ == "__main__":
    unittest.main()
