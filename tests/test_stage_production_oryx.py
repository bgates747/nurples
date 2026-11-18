"""Host-only contracts for the stopped Oryx production stager."""

from __future__ import annotations

import importlib.util
from pathlib import Path
import sys
from tempfile import TemporaryDirectory
import unittest


ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT / "tools"
sys.path.insert(0, str(TOOLS))
SPEC = importlib.util.spec_from_file_location(
    "stage_production_oryx", TOOLS / "stage_production_oryx.py"
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("could not load production stager")
STAGER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(STAGER)


class StageProductionOryxTests(unittest.TestCase):
    def make_project(self, root: Path) -> Path:
        project = root / "project"
        target = project / "tgt"
        (target / "fonts").mkdir(parents=True)
        for name in STAGER.FIXED_FILES:
            (target / name).write_bytes(name.encode("ascii"))
        (target / "fonts/Lat38-VGA8_8x8.font").write_bytes(b"font")
        return project

    def test_input_manifest_is_recursive_and_hash_complete(self) -> None:
        with TemporaryDirectory(prefix="nurples-stage-test-") as temp_dir:
            project = self.make_project(Path(temp_dir))
            files, hashes = STAGER.verify_project_inputs(project)
            relatives = {
                path.relative_to(project / "tgt").as_posix() for path in files
            }
            self.assertEqual(relatives, set(hashes))
            self.assertEqual(
                relatives,
                {
                    *STAGER.FIXED_FILES,
                    "fonts/Lat38-VGA8_8x8.font",
                },
            )

    def test_symlinked_font_is_refused(self) -> None:
        with TemporaryDirectory(prefix="nurples-stage-test-") as temp_dir:
            root = Path(temp_dir)
            project = self.make_project(root)
            font = project / "tgt/fonts/Lat38-VGA8_8x8.font"
            font.unlink()
            font.symlink_to(project / "tgt/game.agnb")
            with self.assertRaises(SystemExit):
                STAGER.verify_project_inputs(project)

    def test_autoexec_loads_but_does_not_run(self) -> None:
        self.assertEqual(
            STAGER.AUTOEXEC,
            b"SET KEYBOARD 1\r\n"
            b"cd /mystuff/nurples-dev/apps/nurples\r\n"
            b"load nurples.bin\r\n",
        )
        self.assertNotIn(b"run", STAGER.AUTOEXEC.lower())


if __name__ == "__main__":
    unittest.main()
