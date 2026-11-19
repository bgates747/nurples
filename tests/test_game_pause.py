"""Source contracts for a screenshot pause without a simulated-time jump."""

from pathlib import Path
import unittest


ASM = Path(__file__).resolve().parents[1] / "src" / "asm"


class GamePauseTests(unittest.TestCase):
    def test_paused_frame_bypasses_clock_dispatch_and_telemetry_not_escape(self) -> None:
        loop = (ASM / "nurples.asm").read_text().split("main_loop:", 1)[1].split("@escape:", 1)[0]
        order = (
            "call game_pause_poll", "jp c,@wait_frame", "call timestamp_tick",
            "call do_game", "@wait_frame:", "call vdu_vblank", "jp z,@escape",
            "ld a,(game_pause_active)", "jp nz,main_loop", "call telemetry_tick",
        )
        self.assertEqual(sorted(loop.index(item) for item in order), [loop.index(item) for item in order])

    def test_pause_uses_held_key_edge_without_vdu_or_game_state_writes(self) -> None:
        pause = (ASM / "game_pause.inc").read_text().lower()
        self.assertIn("game_pause_key_byte: equ 6", pause)
        self.assertIn("game_pause_key_bit: equ 7", pause)
        self.assertIn("call input_gate_update\n    jp nc,@status", pause)
        self.assertIn("xor 1\n    ld (game_pause_active),a\n    call z,timestamp_rebase", pause)
        self.assertNotIn("ld (game_state)", pause)
        self.assertNotIn("vdu_", pause)
        self.assertIn("call game_pause_process_init", (ASM / "runtime_init.inc").read_text())

    def test_pause_rebases_only_raw_clock_and_tick_adds_raw_delta(self) -> None:
        source = (ASM / "timer.inc").read_text()
        rebase = source.split("timestamp_rebase:", 1)[1].split("; end timestamp_rebase", 1)[0]
        self.assertIn("ld (timestamp_raw),hl", rebase)
        for logical_field in ("timestamp_now", "timestamp_old", "timestamp_chg"):
            self.assertNotIn(logical_field, rebase)
        tick = source.split("timestamp_tick:", 1)[1].split("; end timestamp_tick", 1)[0]
        self.assertRegex(tick, r"ld de,\(timestamp_raw\)[^\n]*\n\s+ld \(timestamp_raw\),hl[^\n]*\n")
        self.assertRegex(tick, r"sbc hl,de[^\n]*\n\s+ld \(timestamp_chg\),hl")
        self.assertRegex(tick, r"add hl,de\n\s+ld \(timestamp_now\),hl")
