"""Contracts for zero-safe active-tile launch countdowns."""

from __future__ import annotations

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[1]
ASM = ROOT / "src" / "asm"


def routine(source: str, label: str) -> str:
    match = re.search(
        rf"^{re.escape(label)}:\n(.*?); end {re.escape(label)}$",
        source,
        flags=re.MULTILINE | re.DOTALL,
    )
    if match is None:
        raise AssertionError(f"missing routine {label}")
    return match.group(1)


def tick(timer: int) -> tuple[int, bool]:
    """Return the stored timer and whether the launch is ready."""
    if timer == 0:
        return 0, True
    timer -= 1
    return timer, timer == 0


class ActiveTileLaunchTimerTests(unittest.TestCase):
    def test_zero_is_ready_without_wrapping(self) -> None:
        self.assertEqual(tick(0), (0, True))
        self.assertEqual(tick(1), (0, True))
        self.assertEqual(tick(2), (1, False))
        self.assertEqual(tick(31), (30, False))

    def test_shared_assembly_helper_checks_before_decrement(self) -> None:
        source = (ASM / "tiles_active.inc").read_text()
        helper = routine(source, "active_tile_launch_timer_tick")
        load = helper.index("ld a,(iy+tile_move_timer)")
        zero = helper.index("or a", load)
        ready = helper.index("ret z", zero)
        decrement = helper.index("dec a", ready)
        store = helper.index("ld (iy+tile_move_timer),a", decrement)
        self.assertLess(load, zero)
        self.assertLess(zero, ready)
        self.assertLess(ready, decrement)
        self.assertLess(decrement, store)
        self.assertNotIn("dec (iy+tile_move_timer)", helper)

    def test_pad_and_turret_use_the_shared_zero_safe_tick(self) -> None:
        pad = (ASM / "tile_pad_small.inc").read_text()
        turret = (ASM / "tile_turret_fireball.inc").read_text()
        for source in (pad, turret):
            self.assertEqual(source.count("call active_tile_launch_timer_tick"), 1)
            self.assertNotIn("dec (iy+tile_move_timer)", source)

    def test_post_launch_ranges_remain_nonzero_and_bounded(self) -> None:
        pad = (ASM / "tile_pad_small.inc").read_text()
        turret = (ASM / "tile_turret_fireball.inc").read_text()
        self.assertIn("and %00011111", pad)
        self.assertIn("add a,64", pad)
        self.assertIn("and %00011111", turret)
        self.assertIn("add a,24", turret)
        self.assertEqual((64, 64 + 31), (64, 95))
        self.assertEqual((24, 24 + 31), (24, 55))

    def test_child_spawns_return_to_parent_before_rearming(self) -> None:
        for filename, move, spawn, reset in (
            ("tile_pad_small.inc", "move_pad_small", "seeker", "pad_small"),
            ("tile_turret_fireball.inc", "move_turret_fireball", "fireball", "turret_fireball"),
        ):
            with self.subTest(spawn=spawn):
                code = routine((ASM / filename).read_text(), move)
                child = code.index(f"call enemy_spawn_{spawn}")
                save = code.rindex("push iy", 0, child)
                restore = code.index("pop iy", child)
                rearm = code.index(f"{reset}_reset_move_timer:", restore)
                store = code.index("ld (iy+tile_move_timer),a", rearm)
                self.assertNotIn("pop iy", code[save:child])
                self.assertNotRegex(code[child:store], r"(?m)^\s*(?:ret|jp)\b")

    def test_cooldown_contract_across_initial_repeat_and_full_pool(self) -> None:
        for cooldown in (64, 95, 24, 55):
            with self.subTest(cooldown=cooldown):
                timer = 0
                attempts = []
                spawned = []
                for frame in range(3 * cooldown + 1):
                    timer, ready = tick(timer)
                    if not ready:
                        continue
                    attempts.append(frame)
                    if frame != 2 * cooldown:  # full only at the third expiry
                        spawned.append(frame)
                    timer = cooldown  # parent's cooldown re-arms either way
                self.assertEqual(attempts, [0, cooldown, 2 * cooldown, 3 * cooldown])
                self.assertEqual(spawned, [0, cooldown, 3 * cooldown])

    def test_first_seeker_leg_is_armed_with_matching_heading_and_step(self) -> None:
        source = (ASM / "enemy_seeker.inc").read_text()
        spawn = routine(source, "enemy_spawn_seeker")
        self.assertRegex(spawn, r"jp nz,seeker_enter_orbit\n\s+jp seeker_enter_live_launch")
        live = routine(source, "seeker_enter_live_launch")
        self.assertRegex(
            live,
            r"cp 120[^\n]*\n\s+ld a,0[^\n]*\n\s+ld hl,seeker_zag_heading_left\n"
            r"\s+jp nc,@store\n\s+inc a[^\n]*\n\s+ld hl,seeker_zag_heading_right",
        )
        self.assertRegex(
            live,
            r"ld \(iy\+sprite_heading\),hl\n\s+ld \(iy\+sprite_move_step\),a\n"
            r"\s+call seeker_draw_heading[^\n]*\n\s+jp seeker_arm_zag",
        )


if __name__ == "__main__":
    unittest.main()
