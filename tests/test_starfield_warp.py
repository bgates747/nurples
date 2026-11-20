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
CONTROLLER = ROOT / "src" / "asm" / "level_transition_controller.inc"


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

    def test_level_start_reuses_both_speed_ladders_in_112_frames(self) -> None:
        source = CONTROLLER.read_text()
        approach, stage, hold = (
            int(source.split(f"entrance_warp_{name}_frames: equ ", 1)[1].splitlines()[0])
            for name in ("approach", "speed", "hold")
        )

        def speeds(phase):
            line = source.split(f"level_transition_{phase}_speeds:\n    db ", 1)[1].splitlines()[0]
            return tuple(map(int, line.split(",")))

        rising = speeds("accel")
        falling = speeds("decel")
        self.assertEqual(rising, (1, 2, 3, 4, 6, 8, 12, 16))
        self.assertEqual(falling, rising[-2::-1])
        schedule = (
            (1,) * approach
            + tuple(speed for speed in rising for _ in range(stage))
            + (16,) * hold
            + tuple(speed for speed in falling for _ in range(stage))
        )
        self.assertEqual(len(schedule), 112)
        self.assertEqual(schedule[:16], (1,) * 16)
        self.assertEqual(schedule[40:84], (16,) * 44)
        self.assertEqual(schedule[-4:], (1,) * 4)
        self.assertIn("entrance_warp_total_frames-player_entry_total_frames", PRODUCTION.read_text())
        self.assertIn("call level_transition_step", PLAYER.read_text())

    def test_entry_uses_reusable_damped_sine_motion(self) -> None:
        player = PLAYER.read_text()
        motion = MOTION.read_text()
        self.assertIn("call motion_damped_sine_position", player)
        self.assertIn("ld (iy+sprite_x),hl", player)
        self.assertIn("sub (iy+player_entry_timer) ; elapsed frames", player)
        self.assertIn("cp (iy+player_entry_timer)", player)
        self.assertIn("cp (iy+player_entry_amplitude)", player)
        self.assertIn("ld a,h\n    cp d", player)
        self.assertIn("call sin168", motion)
        self.assertIn("call smul168", motion)
        self.assertNotIn("player_", motion)
        self.assertNotIn("sprite_", motion)

        def value(name: str) -> int:
            return int(player.split(f"player_entry_{name}: equ ", 1)[1].split()[0])

        frames, step, cap, ramp = (value(name) for name in (
            "total_frames", "angle_step", "max_amplitude", "ramp_frames",
        ))
        self.assertEqual((frames, step, cap, ramp), (112, 4, 64, 16))

        def positions(target: int) -> list[float]:
            initial_phase = 128 if target > 120 else 0
            endpoint_cap = min(cap, target, 240 - target)
            return [target + math.sin((initial_phase + frame * step) * math.tau / 256)
                    * min(endpoint_cap, frame * (cap // ramp), frames - frame)
                    for frame in range(frames)]

        opening = positions(120)
        self.assertAlmostEqual(opening[0], 120)
        self.assertLess(abs(opening[1] - 120), 0.5)
        self.assertAlmostEqual(opening[16] - opening[48], 128)
        self.assertAlmostEqual(opening[80], 152)
        self.assertLess(max(abs(b-a) for a, b in zip(opening, opening[1:])), 7)
        for target in range(241):
            with self.subTest(target=target):
                trace = positions(target)
                self.assertAlmostEqual(trace[0], target)
                self.assertLessEqual(abs(trace[-1] - target), 1)
                self.assertTrue(all(0 <= x <= 240 for x in trace))
        self.assertGreater(positions(104)[1], 104)
        self.assertLess(positions(136)[1], 136)

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
