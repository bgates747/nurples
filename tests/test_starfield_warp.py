"""Host contract tests for the starfield-only multi-scanline scroller."""

from __future__ import annotations

from pathlib import Path
import math
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "asm" / "starfield_warp.inc"
FIXTURE = ROOT / "tests" / "asm" / "starfield_warp_fixture.asm"
PRODUCTION = ROOT / "src" / "asm" / "nurples.asm"
GAMEPLAY = ROOT / "src" / "asm" / "state_game_playing.inc"
TERRAIN = ROOT / "src" / "asm" / "tiles.inc"
PLAYER = ROOT / "src" / "asm" / "player_state.inc"
MOTION = ROOT / "src" / "asm" / "motion_damped_sine.inc"
ENTRY_WARP_SPEEDS = (16, 12, 8, 6, 4, 3, 2, 1)
ENTRY_WARP_STAGE_FRAMES = 7
ENTRY_FAST_HOLD_FRAMES = 56


def warp_chunks(phase: int, displacement: int) -> tuple[list[int], int, int]:
    if not 0 <= displacement <= 16:
        raise ValueError(displacement)
    chunks: list[int] = []
    tile_changes = 0
    remaining = displacement
    while remaining:
        if phase == 255:
            tile_changes += 1
            chunks.append(remaining)
            phase = remaining - 1
            remaining = 0
            continue
        capacity = 255 - phase
        chunk = min(remaining, capacity)
        chunks.append(chunk)
        phase += chunk
        remaining -= chunk
    return chunks, phase, tile_changes


class StarfieldWarpTests(unittest.TestCase):
    def test_chunking_is_exact_at_tile_seams(self) -> None:
        self.assertEqual(warp_chunks(255, 1), ([1], 0, 1))
        self.assertEqual(warp_chunks(250, 8), ([5, 3], 2, 1))
        self.assertEqual(warp_chunks(0, 16), ([16], 16, 0))
        self.assertEqual(warp_chunks(254, 16), ([1, 15], 14, 1))
        self.assertEqual(warp_chunks(33, 0), ([], 33, 0))

    def test_every_valid_step_is_bounded_and_conserves_displacement(self) -> None:
        for phase in range(256):
            for displacement in range(17):
                chunks, new_phase, tile_changes = warp_chunks(phase, displacement)
                self.assertEqual(sum(chunks), displacement)
                self.assertLessEqual(len(chunks), 2)
                self.assertTrue(all(1 <= chunk <= 16 for chunk in chunks))
                self.assertEqual(new_phase, (phase + displacement) & 0xFF)
                self.assertIn(tile_changes, (0, 1))

    def test_source_is_starfield_only_and_uses_chunk_sized_viewports(self) -> None:
        source = SOURCE.read_text()
        code = "\n".join(line.split(";", 1)[0] for line in source.splitlines())
        self.assertIn("starfield_warp_max_step: equ 16", source)
        self.assertIn("call vdu_scroll_down", source)
        self.assertIn("ld iyl,a", source)
        self.assertIn("ld hl,(bg_bufferId)", source)
        self.assertIn("ld de,(bg_y_plot)", source)
        self.assertNotIn("tiles_plot", code)
        self.assertNotIn("do_active_tiles", code)

    def test_level_start_decelerates_to_gameplay_speed_before_terrain(self) -> None:
        deceleration = tuple(
            speed
            for speed in ENTRY_WARP_SPEEDS
            for _frame in range(ENTRY_WARP_STAGE_FRAMES)
        )
        schedule = (16,) * ENTRY_FAST_HOLD_FRAMES + deceleration
        self.assertEqual(len(schedule), 112)
        self.assertEqual(schedule[:63], (16,) * 63)
        self.assertEqual(schedule[-7:], (1,) * 7)
        self.assertTrue(all(left >= right for left, right in zip(schedule, schedule[1:])))

        player = PLAYER.read_text()
        self.assertIn("db 16,12,8,6,4,3,2,1", player)
        self.assertIn("player_entry_fast_hold_frames: equ 56", player)
        self.assertIn("player_entry_warp_stage_frames: equ 7", player)
        self.assertIn("player_entry_warp_stage_count: equ 8", player)

    def test_entry_uses_reusable_damped_sine_motion(self) -> None:
        player = PLAYER.read_text()
        motion = MOTION.read_text()
        self.assertIn("call motion_damped_sine_position", player)
        self.assertIn("ld (iy+sprite_x),hl", player)
        self.assertIn("dec a\n    ld (iy+player_entry_amplitude),a", player)
        self.assertIn("call sin168", motion)
        self.assertIn("call smul168", motion)
        self.assertNotIn("player_", motion)
        self.assertNotIn("sprite_", motion)

        positions = []
        for frame in range(112):
            angle = (192 + frame * 8) & 0xFF
            amplitude = 112 - frame
            positions.append(
                120 + math.sin(angle * math.tau / 256) * amplitude
            )
        self.assertAlmostEqual(positions[0], 8.0)
        self.assertTrue(all(8 <= position <= 232 for position in positions))
        self.assertLess(abs(positions[-1] - 120), 1)
        direction_changes = sum(
            (middle - left) * (right - middle) < 0
            for left, middle, right in zip(
                positions,
                positions[1:],
                positions[2:],
            )
        )
        self.assertGreaterEqual(direction_changes, 6)

    def test_starfield_fill_closes_seams_with_vdp_guard_scanline(self) -> None:
        startup = (ROOT / "src" / "asm" / "state_game_init.inc").read_text()
        levels = (ROOT / "src" / "asm" / "levels.inc").read_text()
        warp = SOURCE.read_text()
        self.assertIn("ld c,26+128 ; violet", startup)
        self.assertIn("ld c,16+128 ; darkest blue", levels)
        self.assertNotIn("ld a,26+128 ; violet\n    call vdu_gcol", startup)
        viewport = warp.index("call vdu_set_gfx_viewport")
        self.assertIn(
            "ld a,(starfield_warp_chunk)\n    ld iyl,a",
            warp[:viewport],
        )
        self.assertNotIn(
            "ld a,(starfield_warp_chunk)\n    dec a\n    ld iyl,a",
            warp[:viewport],
        )

    def test_production_routes_warp_around_the_terrain_pipeline(self) -> None:
        production = PRODUCTION.read_text()
        gameplay = GAMEPLAY.read_text()
        terrain = TERRAIN.read_text()
        self.assertIn('include "starfield_warp.inc"', production)
        speed = gameplay.index("call player_entry_starfield_speed")
        warp = gameplay.index("call starfield_warp_scroll")
        terrain_call = gameplay.index("call tiles_plot")
        self.assertLess(speed, warp)
        self.assertLess(warp, terrain_call)
        self.assertIn("jp nc,@terrain", gameplay[speed:warp])
        self.assertNotIn("starfield_warp_scroll", terrain)
        self.assertNotIn("starfield_warp", terrain)

    def test_fixture_assembles_when_toolchain_is_available(self) -> None:
        assembler = shutil.which("ez80asm")
        if assembler is None:
            self.skipTest("ez80asm is not installed on this host")
        with tempfile.TemporaryDirectory(prefix="nurples-starfield-warp-asm-") as temp:
            output = Path(temp) / "fixture.bin"
            result = subprocess.run(
                [assembler, FIXTURE.name, str(output), "-c"],
                cwd=FIXTURE.parent,
                capture_output=True,
                check=False,
                text=True,
            )
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertGreater(output.stat().st_size, 0)


if __name__ == "__main__":
    unittest.main()
