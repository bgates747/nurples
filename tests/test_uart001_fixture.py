from pathlib import Path
import tempfile
import unittest

from build.scripts.build_uart001_fixture import (
    DISPATCH_ANCHOR,
    ESCAPE_ANCHOR,
    FIXTURE,
    FULL_FIXTURE,
    INCLUDE_ANCHOR,
    PLAYER_INPUT_ANCHOR,
    PROCESS_INIT_ANCHOR,
    stage_fixture,
)


class Uart001FixtureBuildTests(unittest.TestCase):
    def test_isolated_overlay_redirects_only_the_main_dispatch(self) -> None:
        source = (
            '    include "runtime_init.inc"\n'
            "    call runtime_process_init\n"
            "main_loop:\n"
            "    call do_game\n"
            "@escape:\n"
            "    call telemetry_request_exit\n"
            "    rst.lil $18\n"
            "    RST.LIL 10h\n"
        )
        with tempfile.TemporaryDirectory() as directory:
            staged = Path(directory)
            (staged / "nurples.asm").write_text(source, encoding="utf-8")
            (staged / "player_state.inc").write_text(
                "player_grace:\n    call player_input\n    ret\n"
                "player_alive:\n    call player_input\n    rst.lil 18h\n"
                "    rst.lil $10\n",
                encoding="utf-8",
            )
            stage_fixture(staged)
            patched = (staged / "nurples.asm").read_text(encoding="utf-8")
            self.assertNotIn(DISPATCH_ANCHOR, patched)
            self.assertIn("    call uart001_do_game\n", patched)
            self.assertEqual(patched.count(INCLUDE_ANCHOR), 1)
            self.assertIn('    include "uart001_fixture.inc"\n', patched)
            self.assertEqual(patched.count(PROCESS_INIT_ANCHOR), 1)
            self.assertIn("    call uart001_process_init\n", patched)
            self.assertNotIn(ESCAPE_ANCHOR, patched)
            self.assertIn("@escape:\n    call uart001_escape\n", patched)
            self.assertIn("    call uart001_vdu_write\n", patched)
            self.assertIn("    call uart001_vdu_write_a\n", patched)
            player = (staged / "player_state.inc").read_text(encoding="utf-8")
            self.assertNotIn(PLAYER_INPUT_ANCHOR, player)
            self.assertIn("player_alive:\n    call uart001_player_input\n", player)
            self.assertIn("player_grace:\n    call player_input\n", player)
            self.assertNotRegex(player, r"(?i)rst\.lil\s+(?:\$18|18h)")
            self.assertNotRegex(player, r"(?i)rst\.lil\s+(?:\$10|10h)")
            self.assertEqual(
                (staged / FIXTURE.name).read_bytes(),
                FIXTURE.read_bytes(),
            )
            self.assertEqual(
                (staged / FULL_FIXTURE.name).read_bytes(),
                FULL_FIXTURE.read_bytes(),
            )

    def test_packet_contract_is_eight_records_plus_sprite_update(self) -> None:
        source = FIXTURE.read_text(encoding="utf-8")
        self.assertIn("UART001_SPRITE_COUNT: equ explosions_max", source)
        self.assertIn("UART001_COMMAND_BYTES_PER_SPRITE: equ 15", source)
        self.assertIn("UART001_UPDATE_BYTES: equ 3", source)
        self.assertIn(
            "UART001_PACKET_BYTES: equ "
            "UART001_SPRITE_COUNT*UART001_COMMAND_BYTES_PER_SPRITE+"
            "UART001_UPDATE_BYTES",
            source,
        )
        self.assertIn("uart001_update_command:\n    db 23,27,15", source)
        self.assertIn("call vdu_sprite_update\n@packet_ready:", source)
        self.assertIn("ld a,025h\n    rst.lil 08h", source)

    def test_full_frame_contract_owns_all_vdu_output(self) -> None:
        source = FULL_FIXTURE.read_text(encoding="utf-8")
        self.assertIn("UART001_FULL_FRAMES_PER_MODE: equ 1200", source)
        self.assertIn("UART001_FULL_WARMUP_FRAMES: equ 120", source)
        self.assertIn("uart001_vdu_write:", source)
        self.assertIn("uart001_vdu_write_a:", source)
        self.assertIn("uart001_full_tick:", source)
        self.assertIn("call uart001_full_hash_and_check", source)
        self.assertIn("call uart001_full_send_packet", source)


if __name__ == "__main__":
    unittest.main()
