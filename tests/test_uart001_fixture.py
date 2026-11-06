from pathlib import Path
import tempfile
import unittest

from build.scripts.build_uart001_fixture import (
    DISPATCH_ANCHOR,
    FIXTURE,
    INCLUDE_ANCHOR,
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
        )
        with tempfile.TemporaryDirectory() as directory:
            staged = Path(directory)
            (staged / "nurples.asm").write_text(source, encoding="utf-8")
            stage_fixture(staged)
            patched = (staged / "nurples.asm").read_text(encoding="utf-8")
            self.assertNotIn(DISPATCH_ANCHOR, patched)
            self.assertIn("    call uart001_do_game\n", patched)
            self.assertEqual(patched.count(INCLUDE_ANCHOR), 1)
            self.assertIn('    include "uart001_fixture.inc"\n', patched)
            self.assertEqual(patched.count(PROCESS_INIT_ANCHOR), 1)
            self.assertIn("    call uart001_process_init\n", patched)
            self.assertEqual(
                (staged / FIXTURE.name).read_bytes(),
                FIXTURE.read_bytes(),
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


if __name__ == "__main__":
    unittest.main()
