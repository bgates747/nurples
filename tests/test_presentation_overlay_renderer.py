"""Contracts for the dormant hardware-sprite presentation renderer."""

from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
from tempfile import TemporaryDirectory
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "asm" / "presentation_overlay_renderer.inc"
SPRITE_API = ROOT / "src" / "asm" / "vdu_sprites.inc"
FIXTURE = ROOT / "tests" / "asm" / "presentation_renderer_fixture.asm"
PRODUCTION = ROOT / "src" / "asm" / "nurples.asm"


class PresentationOverlayRendererTests(unittest.TestCase):
    def test_ownership_uses_reserved_slots_and_exact_physical_positions(self) -> None:
        source = SOURCE.read_text()
        self.assertIn("PRESENTATION_SPRITE_TITLE: equ 20", source)
        self.assertIn("PRESENTATION_SPRITE_PROMPT: equ 21", source)
        self.assertIn("PRESENTATION_ACTIVE_ACTORS: equ 20", source)
        self.assertIn("PRESENTATION_ACTIVE_ALL: equ 22", source)
        self.assertIn("ld bc,136\n    ld de,116", source)
        self.assertIn("ld bc,152\n    ld de,210", source)

    def test_enter_defines_before_publishing_active_prefix(self) -> None:
        source = SOURCE.read_text()
        enter = source[source.index("presentation_renderer_enter:") : source.index(
            "; end presentation_renderer_enter"
        )]
        begin = enter.index("call overlay_feature_begin")
        deactivate = enter.index("call vdu_sprite_activate", begin)
        title_hardware = enter.index("call vdu_sprite_mark_hardware", deactivate)
        prompt_hardware = enter.index("call vdu_sprite_mark_hardware", title_hardware + 1)
        activate = enter.index("ld a,PRESENTATION_ACTIVE_ALL", prompt_hardware)
        publish = enter.index("ld (presentation_renderer_active),a", activate)
        self.assertIn("xor a\n    call vdu_sprite_activate", enter[begin:title_hardware])
        self.assertLess(begin, deactivate)
        self.assertLess(deactivate, title_hardware)
        self.assertLess(title_hardware, prompt_hardware)
        self.assertLess(prompt_hardware, activate)
        self.assertLess(activate, publish)

    def test_exit_detaches_and_demotes_before_feature_restore(self) -> None:
        source = SOURCE.read_text()
        exit_block = source[
            source.index("presentation_renderer_exit:") : source.index(
                "; end presentation_renderer_exit"
            )
        ]
        prompt_hide = exit_block.index("call vdu_sprite_hide")
        title_hide = exit_block.index("call vdu_sprite_hide", prompt_hide + 1)
        deactivate = exit_block.index("call vdu_sprite_activate", title_hide)
        prompt_clear = exit_block.index("call vdu_sprite_clear_frames", deactivate)
        prompt_demote = exit_block.index("call vdu_sprite_mark_software", prompt_clear)
        title_clear = exit_block.index("call vdu_sprite_clear_frames", prompt_demote)
        title_demote = exit_block.index("call vdu_sprite_mark_software", title_clear)
        restore_prefix = exit_block.index(
            "ld a,PRESENTATION_ACTIVE_ACTORS", title_demote
        )
        restore = exit_block.index("call overlay_feature_end", restore_prefix)
        self.assertLess(prompt_hide, title_hide)
        self.assertIn("xor a\n    call vdu_sprite_activate", exit_block[title_hide:prompt_clear])
        self.assertLess(title_hide, deactivate)
        self.assertLess(deactivate, prompt_clear)
        self.assertLess(prompt_clear, prompt_demote)
        self.assertLess(prompt_demote, title_clear)
        self.assertLess(title_clear, title_demote)
        self.assertLess(title_demote, restore_prefix)
        self.assertLess(restore_prefix, restore)
        self.assertLess(title_demote, restore)
        self.assertNotIn("vdu_clear_buffer", exit_block)

    def test_sprite_api_exposes_explicit_hardware_and_software_commands(self) -> None:
        source = SPRITE_API.read_text()
        self.assertIn("vdu_sprite_mark_hardware:", source)
        self.assertIn("@cmd: db 23,27,19", source)
        self.assertIn("vdu_sprite_mark_software:", source)
        self.assertIn("@cmd: db 23,27,20", source)

    def test_qualified_renderer_is_in_production(self) -> None:
        self.assertIn(
            'include "presentation_overlay_renderer.inc"', PRODUCTION.read_text()
        )

    def test_fixture_assembles_with_short_name(self) -> None:
        assembler = shutil.which("ez80asm")
        if assembler is None:
            self.skipTest("ez80asm is not installed on this host")
        with TemporaryDirectory(prefix="nurples-renderer-") as temp_dir:
            mirror = Path(temp_dir)
            shutil.copytree(ROOT / "src" / "asm", mirror / "src" / "asm")
            fixture_root = mirror / "tests" / "asm"
            fixture_root.mkdir(parents=True)
            short_source = fixture_root / "f.asm"
            shutil.copy2(FIXTURE, short_source)
            output = fixture_root / "f.bin"
            result = subprocess.run(
                [assembler, short_source.name, str(output), "-c"],
                cwd=fixture_root,
                capture_output=True,
                check=False,
                text=True,
            )
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertGreater(output.stat().st_size, 0)


if __name__ == "__main__":
    unittest.main()
