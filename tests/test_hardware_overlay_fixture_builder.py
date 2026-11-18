from pathlib import Path
from tempfile import TemporaryDirectory
import unittest
from unittest.mock import patch

from build.scripts.build_hardware_overlay_fixture import (
    AUTOEXEC,
    PAYLOADS,
    build_bundle,
    check_bundle,
    validate_sources,
)


class CompletedAssembler:
    returncode = 0
    stdout = ""
    stderr = ""


class HardwareOverlayFixtureBuilderTests(unittest.TestCase):
    def test_source_payloads_have_exact_rgba2222_sizes(self) -> None:
        self.assertEqual(validate_sources(), [])
        self.assertEqual(PAYLOADS["game_over_0.rgba2"], (240, 64))
        self.assertEqual(PAYLOADS["press_any_key_to_start.rgba2"], (208, 16))

    def test_build_writes_a_self_verifying_load_only_bundle(self) -> None:
        def fake_assembler(command, **kwargs):
            del kwargs
            Path(command[2]).write_bytes(b"fixture-binary")
            return CompletedAssembler()

        with TemporaryDirectory(prefix="nurples-overlay-builder-") as temp_dir:
            output = Path(temp_dir) / "bundle"
            with patch(
                "build.scripts.build_hardware_overlay_fixture.subprocess.run",
                side_effect=fake_assembler,
            ):
                build_bundle(output, Path("/fake/ez80asm"))
            self.assertEqual(check_bundle(output), [])
            self.assertEqual((output / "autoexec.txt").read_bytes(), AUTOEXEC)
            self.assertEqual((output / "overlay.bin").read_bytes(), b"fixture-binary")

    def test_check_detects_bundle_drift(self) -> None:
        def fake_assembler(command, **kwargs):
            del kwargs
            Path(command[2]).write_bytes(b"fixture-binary")
            return CompletedAssembler()

        with TemporaryDirectory(prefix="nurples-overlay-builder-") as temp_dir:
            output = Path(temp_dir) / "bundle"
            with patch(
                "build.scripts.build_hardware_overlay_fixture.subprocess.run",
                side_effect=fake_assembler,
            ):
                build_bundle(output, Path("/fake/ez80asm"))
            (output / "game_over_0.rgba2").write_bytes(b"drift")
            errors = check_bundle(output)
            self.assertIn("bundle hash mismatch: game_over_0.rgba2", errors)


if __name__ == "__main__":
    unittest.main()
