from pathlib import Path
import re
import shutil
import subprocess
from tempfile import TemporaryDirectory
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SOURCE_PATH = PROJECT_ROOT / "src" / "asm" / "hardware_overlay_feature.inc"
FIXTURE_PATH = PROJECT_ROOT / "tests" / "asm" / "hardware_overlay_feature_fixture.asm"
RUNTIME_FIXTURE_PATH = (
    PROJECT_ROOT / "tests" / "asm" / "hardware_overlay_runtime_fixture.asm"
)

CONSTANTS = {
    "OVERLAY_FEATURE_VARIABLE": 2,
    "OVERLAY_BUF_RESTORE_PROGRAM": 63985,
    "OVERLAY_BUF_MARK_PRESENT": 63987,
    "OVERLAY_BUF_SNAPSHOT_DISPATCH": 63988,
}


def assemble_with_short_name(
    assembler: str, fixture: Path, output: Path
) -> subprocess.CompletedProcess[str]:
    """Mirror the include tree because ez80asm rejects long source basenames."""
    with TemporaryDirectory(prefix="nurf-") as temp_dir:
        mirror = Path(temp_dir)
        shutil.copytree(PROJECT_ROOT / "src" / "asm", mirror / "src" / "asm")
        fixture_root = mirror / "tests" / "asm"
        fixture_root.mkdir(parents=True)
        short_fixture = fixture_root / "f.asm"
        shutil.copy2(fixture, short_fixture)
        return subprocess.run(
            [assembler, short_fixture.name, str(output), "-c"],
            cwd=fixture_root,
            capture_output=True,
            check=False,
            text=True,
        )


def parse_value(token: str) -> int:
    token = token.strip().lower()
    if token.upper() in CONSTANTS:
        return CONSTANTS[token.upper()]
    if token.endswith("h"):
        return int(token[:-1], 16)
    return int(token, 10)


def payload(source: str, label: str) -> bytes:
    match = re.search(
        rf"(?ms)^{re.escape(label)}:\s*\n(.*?)^{re.escape(label)}_end:",
        source,
    )
    if match is None:
        raise AssertionError(f"payload block not found: {label}")
    result = bytearray()
    for raw_line in match.group(1).splitlines():
        line = raw_line.split(";", 1)[0].strip()
        if not line:
            continue
        directive, arguments = line.split(None, 1)
        values = [parse_value(value) for value in arguments.split(",")]
        if directive == "db":
            result.extend(values)
        elif directive == "dw":
            for value in values:
                result.extend((value & 0xFF, value >> 8))
        else:
            raise AssertionError(f"unexpected directive in {label}: {line}")
    return bytes(result)


class HardwareOverlayFeatureTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.source = SOURCE_PATH.read_text()

    def test_exact_vdp_resident_transaction_payloads(self) -> None:
        cases = {
            "overlay_restore_program": bytes([23, 0, 0xF9, 2, 0, 0, 0]),
            "overlay_mark_present": bytes(
                [
                    23,
                    0,
                    0xA0,
                    0xF1,
                    0xF9,
                    5,
                    2,
                    2,
                    0,
                    0xF8,
                    23,
                    0,
                    0xA0,
                    0xF1,
                    0xF9,
                    48,
                    0x80,
                    5,
                    0,
                    2,
                    0,
                ]
            ),
            "overlay_snapshot_dispatch": bytes(
                [23, 0, 0xA0, 0xF3, 0xF9, 6, 0x40, 2, 0]
            ),
            "overlay_feature_enable": bytes([23, 0, 0xF8, 2, 0, 0, 0]),
        }
        for label, expected in cases.items():
            with self.subTest(label=label):
                self.assertEqual(payload(self.source, label), expected)

    def test_begin_snapshots_before_enabling_feature(self) -> None:
        begin = self.source[
            self.source.index("overlay_feature_begin:") : self.source.index(
                "; end overlay_feature_begin"
            )
        ]
        snapshot = begin.index("ld hl,OVERLAY_BUF_SNAPSHOT_DISPATCH")
        call_snapshot = begin.index("call vdu_call_buffer", snapshot)
        enable = begin.index("ld hl,overlay_feature_enable", call_snapshot)
        publish_active = begin.index("ld (overlay_feature_active),a", enable)
        self.assertLess(snapshot, call_snapshot)
        self.assertLess(call_snapshot, enable)
        self.assertLess(enable, publish_active)

    def test_restore_does_not_branch_on_an_ordinary_buffer_value(self) -> None:
        self.assertNotIn("OVERLAY_BUF_ORIGINAL_PRESENT", self.source)
        self.assertNotIn("overlay_restore_dispatch", self.source)
        self.assertEqual(self.source.count("db 6,040h"), 1)

    def test_process_init_is_ram_only_before_vdp_resource_acquisition(self) -> None:
        process_init = self.source[
            self.source.index("overlay_feature_process_init:") : self.source.index(
                "; end overlay_feature_process_init"
            )
        ]
        self.assertIn("ld (overlay_feature_active),a", process_init)
        self.assertNotIn("overlay_feature_clear_control_buffers", process_init)
        self.assertNotIn("vdu_", process_init)

    def test_end_restores_before_clearing_controls(self) -> None:
        end = self.source[
            self.source.index("overlay_feature_end:") : self.source.index(
                "; end overlay_feature_end"
            )
        ]
        program = end.index("ld hl,OVERLAY_BUF_RESTORE_PROGRAM")
        restore = end.index("call vdu_call_buffer", program)
        clear = end.index("call overlay_feature_clear_control_buffers", restore)
        revoke_active = end.index("ld (overlay_feature_active),a", clear)
        self.assertLess(program, restore)
        self.assertLess(restore, clear)
        self.assertLess(clear, revoke_active)

    def test_qualified_module_is_in_production_binary(self) -> None:
        production = (PROJECT_ROOT / "src" / "asm" / "nurples.asm").read_text()
        self.assertIn('include "hardware_overlay_feature.inc"', production)

    def test_source_module_assembles_when_toolchain_is_available(self) -> None:
        assembler = shutil.which("ez80asm")
        if assembler is None:
            self.skipTest("ez80asm is not installed on this host")
        with TemporaryDirectory(prefix="nurples-overlay-feature-") as temp_dir:
            output = Path(temp_dir) / "f.bin"
            result = assemble_with_short_name(assembler, FIXTURE_PATH, output)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertGreater(output.stat().st_size, 0)

    def test_runtime_fixture_preserves_teardown_order(self) -> None:
        fixture = RUNTIME_FIXTURE_PATH.read_text()
        teardown = fixture[
            fixture.index("fixture_destroy_overlay_sprites:") : fixture.index(
                "fixture_run_overlay:"
            )
        ]
        hide_prompt = teardown.index("ld a,OVERLAY_SPRITE_PROMPT")
        hide_title = teardown.index("ld a,OVERLAY_SPRITE_TITLE", hide_prompt)
        deactivate = teardown.index("call fixture_sprite_activate", hide_title)
        detach_prompt = teardown.index("ld a,OVERLAY_SPRITE_PROMPT", deactivate)
        detach_title = teardown.index("ld a,OVERLAY_SPRITE_TITLE", detach_prompt)
        restore_prefix = teardown.index("ld a,OVERLAY_ACTIVE_ACTORS", detach_title)
        release_bitmap = teardown.index("ld hl,OVERLAY_BUF_TITLE_0", restore_prefix)
        self.assertLess(hide_prompt, hide_title)
        self.assertIn(
            "xor a\n    call fixture_sprite_activate",
            teardown[hide_title:detach_prompt],
        )
        self.assertLess(hide_title, deactivate)
        self.assertLess(deactivate, detach_prompt)
        self.assertLess(detach_prompt, detach_title)
        self.assertLess(detach_title, restore_prefix)
        self.assertLess(restore_prefix, release_bitmap)
        self.assertLess(detach_title, release_bitmap)

    def test_runtime_fixture_stops_engine_before_overlay_configuration(self) -> None:
        fixture = RUNTIME_FIXTURE_PATH.read_text()
        configure = fixture[
            fixture.index("fixture_define_overlay_sprites:") : fixture.index(
                "fixture_destroy_overlay_sprites:"
            )
        ]
        deactivate = configure.index("call fixture_sprite_activate")
        witness_clear = configure.index("call fixture_sprite_clear_frames")
        title_hardware = configure.index("call fixture_sprite_mark_hardware")
        prompt_hardware = configure.index(
            "call fixture_sprite_mark_hardware", title_hardware + 1
        )
        activate = configure.index("ld a,OVERLAY_ACTIVE_ALL", prompt_hardware)
        self.assertIn(
            "xor a\n    call fixture_sprite_activate",
            configure[:witness_clear],
        )
        self.assertLess(deactivate, witness_clear)
        self.assertLess(witness_clear, title_hardware)
        self.assertLess(title_hardware, prompt_hardware)
        self.assertLess(prompt_hardware, activate)

    def test_runtime_fixture_reloads_status_buffer_after_clear(self) -> None:
        fixture = RUNTIME_FIXTURE_PATH.read_text()
        status = fixture[
            fixture.index("fixture_write_status_programs:") : fixture.index(
                "fixture_write_verifier:"
            )
        ]
        self.assertIn(
            "ld hl,OVERLAY_BUF_PASS\n"
            "    call vdu_clear_buffer\n"
            "    ld hl,OVERLAY_BUF_PASS\n"
            "    ld bc,fixture_status_pass_end-fixture_status_pass",
            status,
        )
        self.assertIn(
            "ld hl,OVERLAY_BUF_FAIL\n"
            "    call vdu_clear_buffer\n"
            "    ld hl,OVERLAY_BUF_FAIL\n"
            "    ld bc,fixture_status_fail_end-fixture_status_fail",
            status,
        )

    def test_runtime_fixture_assembles_when_toolchain_is_available(self) -> None:
        assembler = shutil.which("ez80asm")
        if assembler is None:
            self.skipTest("ez80asm is not installed on this host")
        with TemporaryDirectory(prefix="nurples-overlay-runtime-") as temp_dir:
            output = Path(temp_dir) / "f.bin"
            result = assemble_with_short_name(assembler, RUNTIME_FIXTURE_PATH, output)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertGreater(output.stat().st_size, 0)


if __name__ == "__main__":
    unittest.main()
