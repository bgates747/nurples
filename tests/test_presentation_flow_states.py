"""Contracts for the dormant cooperative presentation-flow state layer."""

from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess
from tempfile import TemporaryDirectory
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "asm" / "presentation_flow_states.inc"
STATE = ROOT / "src" / "asm" / "state.inc"
PRODUCTION = ROOT / "src" / "asm" / "nurples.asm"
FIXTURE = ROOT / "tests" / "asm" / "presentation_flow_states_fixture.asm"


def block(source: str, label: str) -> str:
    match = re.search(
        rf"^{re.escape(label)}:\n(.*?); end {re.escape(label)}$",
        source,
        flags=re.MULTILINE | re.DOTALL,
    )
    if match is None:
        raise AssertionError(f"missing block {label}")
    return match.group(1)


class PresentationFlowStateTests(unittest.TestCase):
    def test_attract_defines_ordinary_prefix_before_overlay_entry(self) -> None:
        begin = block(SOURCE.read_text(), "game_attract_begin")
        prepare = begin.index("call game_prepare_playfield")
        prefix = begin.index("call reset_runtime_tables", prepare)
        overlay = begin.index("call attract_lifecycle_enter", prefix)
        publish = begin.index("ld (game_state),hl", overlay)
        self.assertLess(prepare, prefix)
        self.assertLess(prefix, overlay)
        self.assertLess(overlay, publish)

    def test_attract_and_game_over_transition_through_bounded_config(self) -> None:
        source = SOURCE.read_text()
        attract = block(source, "game_attract")
        self.assertIn("call attract_lifecycle_update", attract)
        self.assertIn("call attract_lifecycle_exit", attract)
        self.assertIn("jp game_start_config_begin", attract)
        game_over = block(source, "game_over_bounded")
        self.assertIn("call game_over_lifecycle_enter", game_over)
        self.assertIn("call game_over_lifecycle_update", game_over)
        self.assertIn("call game_over_lifecycle_exit", game_over)
        self.assertIn("jp game_start_config_begin", game_over)

    def test_configuration_is_nonblocking_and_uses_resident_session_boundary(self) -> None:
        config = block(SOURCE.read_text(), "game_start_config")
        self.assertEqual(config.count("call presentation_input_poll"), 1)
        self.assertEqual(config.count("call start_choice_update"), 1)
        self.assertIn("call player_joystick_enable", config)
        self.assertIn("call player_joystick_disable", config)
        self.assertIn("call game_prepare_playfield", config)
        self.assertIn("call game_new_session_init", config)
        code = "\n".join(line.split(";", 1)[0] for line in config.splitlines())
        self.assertNotIn("game_init", code)
        self.assertNotIn("load_game_images", code)
        self.assertNotIn("vdu_clear_all_buffers", code)
        self.assertNotIn("waitKeypress", code)
        self.assertNotIn("vdu_vblank", code)

    def test_failures_exit_without_asset_reload_retry(self) -> None:
        source = SOURCE.read_text()
        failure = block(source, "presentation_flow_fail")
        self.assertIn("set game_fault_presentation,(hl)", failure)
        self.assertIn("ld (game_exit_requested),a", failure)
        self.assertNotIn("game_init", failure)
        config = block(source, "game_start_config")
        self.assertIn("set game_fault_session_init,(hl)", config)
        self.assertIn("ld (game_exit_requested),a", config)
        state = STATE.read_text()
        self.assertIn("game_fault_presentation: equ 1", state)
        self.assertIn("game_fault_session_init: equ 2", state)

    def test_process_shutdown_restores_feature_before_global_resource_cleanup(self) -> None:
        shutdown = block(SOURCE.read_text(), "presentation_flow_shutdown")
        renderer = shutdown.index("call presentation_renderer_exit")
        orphan = shutdown.index("call overlay_feature_end", renderer)
        controller = shutdown.index("call presentation_overlay_exit", orphan)
        choice = shutdown.index("call start_choice_exit", controller)
        self.assertLess(renderer, orphan)
        self.assertLess(orphan, controller)
        self.assertLess(controller, choice)
        self.assertNotIn("vdu_sprite_reset", shutdown)
        self.assertNotIn("vdu_clear_all_buffers", shutdown)

    def test_qualified_state_layer_owns_production_start_and_shutdown(self) -> None:
        production = PRODUCTION.read_text()
        self.assertIn('include "presentation_flow_states.inc"', production)
        shutdown = production.index("call presentation_flow_shutdown")
        sprite_reset = production.index("call vdu_sprite_reset", shutdown)
        clear_buffers = production.index("call vdu_clear_all_buffers", sprite_reset)
        self.assertLess(shutdown, sprite_reset)
        self.assertLess(sprite_reset, clear_buffers)
        init = (ROOT / "src" / "asm" / "state_game_init.inc").read_text()
        self.assertIn("call game_attract_begin", init)
        game_init = block(init, "game_init")
        self.assertNotIn('asciz "Press any key to continue."', game_init)
        self.assertNotIn("call waitKeypress", game_init.split("@font_load_error:", 1)[0])
        self.assertNotIn("call choose_joystick", game_init)
        self.assertNotIn("call game_new_session_init", game_init)
        runtime = (ROOT / "src" / "asm" / "runtime_init.inc").read_text()
        self.assertIn("call presentation_flow_process_init", runtime)

    def test_production_dispatch_and_telemetry_admit_bounded_flow(self) -> None:
        state = STATE.read_text()
        whitelist = block(state, "game_state_pointer_valid")
        self.assertIn("ld de,game_attract", whitelist)
        self.assertIn("ld de,game_start_config", whitelist)
        game_over = state[state.index("game_over:") : state.index("game_victory:")]
        self.assertIn("jp game_over_bounded", game_over)
        self.assertNotIn("waitKeypress", game_over)
        telemetry = (ROOT / "src" / "asm" / "telemetry.inc").read_text()
        self.assertIn("dl game_attract\n    db STATE_NEW_TITLE", telemetry)
        self.assertIn("dl game_start_config\n    db STATE_NEW_TITLE", telemetry)

    def test_native_transition_fixture_assembles_when_toolchain_is_available(self) -> None:
        assembler = shutil.which("ez80asm")
        if assembler is None:
            self.skipTest("ez80asm is not installed on this host")
        with TemporaryDirectory(prefix="nurples-flow-states-") as temp_dir:
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
