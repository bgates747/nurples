"""Contracts for the bounded production GAME OVER lifecycle."""

from __future__ import annotations

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "asm" / "game_over_lifecycle.inc"
PLAYER = ROOT / "src" / "asm" / "player_state.inc"
PRODUCTION = ROOT / "src" / "asm" / "nurples.asm"
PAD = ROOT / "src" / "asm" / "tile_pad_small.inc"
INIT = ROOT / "src" / "asm" / "state_game_init.inc"
VDU = ROOT / "src" / "asm" / "vdu.inc"
DISPLAY = ROOT / "src" / "asm" / "playfield_display.inc"


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
        verified_display = enter.index("call playfield_display_verify")
        acquire = enter.index("call presentation_renderer_enter")
        player = enter.index("ld iy,player_begin", acquire)
        lasers = enter.index("ld iy,player_lasers_begin", player)
        fireballs = enter.index("ld iy,fireballs_base", lasers)
        retreat = enter.index("call seeker_group_enter_game_over_retreat", fireballs)
        self.assertLess(verified_display, acquire)
        self.assertLess(acquire, player)
        self.assertLess(player, lasers)
        self.assertLess(lasers, fireballs)
        self.assertLess(fireballs, retreat)
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
        self.assertLess(policy.index("ld de,game_over"), policy.index("ld de,player_alive"))
        self.assertIn(
            "@game_over:\n"
            "; Advance both terrain pixels and active-tile coordinates in the scroll control.\n"
            "    .if GAME_OVER_REVIEW_SCROLL\n    jp @travel\n    .endif\n"
            "@paused:\n    xor a\n    ret",
            policy,
        )
        self.assertNotIn("ld (player_state)", policy)

    def test_pad_animation_precedes_the_game_over_launch_gate(self) -> None:
        pad = PAD.read_text().split("move_pad_small:", 1)[1].split("kill_pad_small:")[0]
        self.assertLess(pad.index("call draw_active_tile"), pad.index("ld de,game_over"))
        self.assertLess(pad.index("ret z"), pad.index("call active_tile_launch_timer_tick"))
        self.assertLess(
            pad.index("call active_tile_launch_timer_tick"), pad.index("call enemy_spawn_seeker")
        )

    def test_exit_orders_renderer_before_controller_and_retains_assets(self) -> None:
        exit_block = block(SOURCE.read_text(), "game_over_lifecycle_exit")
        renderer = exit_block.index("call presentation_renderer_exit")
        controller = exit_block.index("call presentation_overlay_exit")
        self.assertLess(renderer, controller)
        self.assertNotIn("vdu_clear_buffer", exit_block)
        self.assertNotIn("vdu_clear_all_buffers", exit_block)

    def test_qualified_lifecycle_is_in_production(self) -> None:
        self.assertIn('include "game_over_lifecycle.inc"', PRODUCTION.read_text())

    def test_playfield_allocation_precedes_asset_load_and_redraw_does_not_reallocate(self) -> None:
        source = INIT.read_text()
        startup = block(source, "game_init")
        check = startup.index("call playfield_display_select\n    jp nc,presentation_flow_fail")
        reset = startup.index("ld a,26")
        cursor = startup.index("call vdu_select_text_cursor")
        self.assertLess(check, reset)
        self.assertLess(reset, cursor)
        self.assertLess(cursor, startup.index("call vdu_cls"))
        self.assertLess(check, startup.index("call fonts_load"))
        redraw = block(source, "game_prepare_playfield")
        self.assertIn("call playfield_display_verify\n    ret nc", redraw)
        self.assertNotIn("call vdu_set_screen_mode", redraw)

    def test_startup_only_sets_mode_if_live_mode_differs(self) -> None:
        source = DISPLAY.read_text()
        select = block(source, "playfield_display_select")
        self.assertIn(
            "call vdu_query_mode\n"
            "    jp nc,playfield_display_failed\n"
            "    ld a,(ix+sysvar_scrMode)\n"
            "    cp PLAYFIELD_MODE\n"
            "    jp z,playfield_display_validate\n"
            "    ld a,PLAYFIELD_MODE\n"
            "    call vdu_set_mode_confirm\n"
            "    jp nc,playfield_display_failed\n"
            "    jp playfield_display_validate",
            select,
        )
        self.assertEqual(select.count("call vdu_set_mode_confirm"), 1)
        validate = block(source, "playfield_display_validate")
        for expected in ("screen_bottom+1", "screen_right+1", "PLAYFIELD_COLOURS"):
            self.assertIn(expected, validate)

    def test_mode_set_acknowledgement_is_separate_from_mode_query(self) -> None:
        source = VDU.read_text()
        setter = block(source, "vdu_set_mode_confirm")
        query = block(source, "vdu_query_mode")
        fence = block(source, "vdu_mode_reply_fence")
        for request in (setter, query):
            self.assertLess(
                request.index("call vdu_mode_reply_fence"),
                request.index("res 4,(ix+sysvar_vpd_pflags)"),
            )
            self.assertIn("ld d,vdp_pflag_mode", request)
        self.assertIn("call vdu_set_screen_mode", setter)
        self.assertNotIn("vdp_mode", setter)
        self.assertIn("db 23,0,vdp_mode", query)
        self.assertIn("db 23,0,vdp_cursor", fence)
        self.assertNotIn("vdu_get_text_colour", fence)


if __name__ == "__main__":
    unittest.main()
