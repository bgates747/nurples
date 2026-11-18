"""Contracts for the bounded production TITLE/ATTRACT lifecycle."""

from __future__ import annotations

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "asm" / "attract_lifecycle.inc"
PRODUCTION = ROOT / "src" / "asm" / "nurples.asm"


def block(source: str, label: str) -> str:
    match = re.search(
        rf"^{re.escape(label)}:\n(.*?); end {re.escape(label)}$",
        source,
        flags=re.MULTILINE | re.DOTALL,
    )
    if match is None:
        raise AssertionError(f"missing block {label}")
    return match.group(1)


class AttractLifecycleTests(unittest.TestCase):
    def test_update_scrolls_one_blank_starfield_step_and_one_overlay_step(self) -> None:
        update = block(SOURCE.read_text(), "attract_lifecycle_update")
        self.assertIn("ld a,1\n    call starfield_warp_scroll", update)
        self.assertEqual(update.count("call presentation_input_poll"), 1)
        self.assertEqual(update.count("call presentation_overlay_update"), 1)
        self.assertEqual(update.count("call presentation_renderer_update"), 1)
        self.assertNotIn("tiles_plot", update)
        self.assertNotIn("do_player", update)
        self.assertNotIn("do_enemies", update)
        self.assertNotIn("move_tiles", update)
        self.assertNotIn("vdu_vblank", update)
        self.assertNotIn("waitKeypress", update)

    def test_entry_and_exit_preserve_renderer_controller_ownership_order(self) -> None:
        source = SOURCE.read_text()
        enter = block(source, "attract_lifecycle_enter")
        self.assertLess(
            enter.index("call presentation_overlay_enter"),
            enter.index("call presentation_renderer_enter"),
        )
        exit_block = block(source, "attract_lifecycle_exit")
        self.assertLess(
            exit_block.index("call presentation_renderer_exit"),
            exit_block.index("call presentation_overlay_exit"),
        )
        self.assertNotIn("vdu_clear_buffer", exit_block)
        self.assertNotIn("vdu_clear_all_buffers", exit_block)

    def test_qualified_lifecycle_is_in_production(self) -> None:
        self.assertIn('include "attract_lifecycle.inc"', PRODUCTION.read_text())


if __name__ == "__main__":
    unittest.main()
