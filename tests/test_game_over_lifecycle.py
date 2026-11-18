"""Contracts for the bounded production GAME OVER lifecycle."""

from __future__ import annotations

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "asm" / "game_over_lifecycle.inc"
PLAYER = ROOT / "src" / "asm" / "player_state.inc"
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


class GameOverLifecycleTests(unittest.TestCase):
    def test_entry_acquires_overlay_before_removing_hostiles(self) -> None:
        enter = block(SOURCE.read_text(), "game_over_lifecycle_enter")
        acquire = enter.index("call presentation_renderer_enter")
        player = enter.index("ld iy,player_begin", acquire)
        lasers = enter.index("ld iy,player_lasers_begin", player)
        fireballs = enter.index("ld iy,fireballs_base", lasers)
        self.assertLess(acquire, player)
        self.assertLess(player, lasers)
        self.assertLess(lasers, fireballs)
        self.assertNotIn("reset_runtime_tables", enter)
        self.assertNotIn("seeker_group_enter_orbit", enter)

    def test_update_has_one_world_step_and_no_player_or_vblank(self) -> None:
        update = block(SOURCE.read_text(), "game_over_lifecycle_update")
        self.assertEqual(update.count("call tiles_plot"), 1)
        self.assertEqual(update.count("call do_enemies"), 1)
        self.assertEqual(update.count("call move_explosions"), 1)
        self.assertEqual(update.count("call move_tiles"), 1)
        self.assertEqual(update.count("call presentation_input_poll"), 1)
        self.assertEqual(update.count("call presentation_overlay_update"), 1)
        self.assertEqual(update.count("call presentation_renderer_update"), 1)
        self.assertNotIn("call do_player", update)
        self.assertNotIn("vdu_vblank", update)
        self.assertNotIn("waitKeypress", update)
        self.assertNotIn("multiPurposeDelay", update)

    def test_world_policy_reads_global_game_over_without_mutating_player(self) -> None:
        policy = block(PLAYER.read_text(), "player_world_travel_enabled")
        self.assertIn("ld hl,(game_state)", policy)
        self.assertIn("ld de,game_over", policy)
        self.assertNotIn("ld (player_state)", policy)

    def test_exit_orders_renderer_before_controller_and_retains_assets(self) -> None:
        exit_block = block(SOURCE.read_text(), "game_over_lifecycle_exit")
        renderer = exit_block.index("call presentation_renderer_exit")
        controller = exit_block.index("call presentation_overlay_exit")
        self.assertLess(renderer, controller)
        self.assertNotIn("vdu_clear_buffer", exit_block)
        self.assertNotIn("vdu_clear_all_buffers", exit_block)

    def test_qualified_lifecycle_is_in_production(self) -> None:
        self.assertIn('include "game_over_lifecycle.inc"', PRODUCTION.read_text())


if __name__ == "__main__":
    unittest.main()
