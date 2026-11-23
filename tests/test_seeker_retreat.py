"""Host contract tests for seeker launch-pad identity and retreat policy."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "asm" / "seeker_retreat.inc"
FIXTURE = ROOT / "tests" / "asm" / "seeker_retreat_fixture.asm"
PRODUCTION = ROOT / "src" / "asm" / "nurples.asm"


@dataclass(frozen=True)
class Pad:
    record_id: int
    generation: int
    tile_type: int = 1
    alive: bool = True
    y: int = 120


def resolve_pad(identity: tuple[int, int], pads: tuple[Pad, ...]) -> Pad | None:
    record_id, generation = identity
    if record_id == 0xFF or generation == 0:
        return None
    match = next((pad for pad in pads if pad.record_id == record_id), None)
    if match is None:
        return None
    if match.generation != generation or match.tile_type != 1 or not match.alive:
        return None
    if match.y + 16 <= 0 or match.y > 335:
        return None
    return match


def retreat_edge(x: int) -> tuple[str, int]:
    return ("left", 192) if x < 120 else ("right", 64)


class SeekerRetreatTests(unittest.TestCase):
    def test_pad_identity_rejects_reuse_inactivity_and_invisibility(self) -> None:
        identity = (1, 7)
        self.assertEqual(resolve_pad(identity, (Pad(1, 7),)), Pad(1, 7))
        self.assertIsNone(resolve_pad(identity, (Pad(1, 8),)))
        self.assertIsNone(resolve_pad(identity, (Pad(1, 7, tile_type=9),)))
        self.assertIsNone(resolve_pad(identity, (Pad(1, 7, alive=False),)))
        self.assertIsNotNone(resolve_pad(identity, (Pad(1, 7, y=335),)))
        self.assertIsNone(resolve_pad(identity, (Pad(1, 7, y=336),)))
        self.assertIsNone(resolve_pad(identity, (Pad(1, 7, y=-16),)))
        self.assertIsNone(resolve_pad((0xFF, 0), (Pad(1, 7),)))

    def test_nearest_edge_tie_policy_is_deterministic(self) -> None:
        self.assertEqual(retreat_edge(0), ("left", 192))
        self.assertEqual(retreat_edge(119), ("left", 192))
        self.assertEqual(retreat_edge(120), ("right", 64))
        self.assertEqual(retreat_edge(240), ("right", 64))

    def test_pose_is_checked_before_collisions_and_both_publication_paths(self) -> None:
        source = SOURCE.read_text()
        retreat = source.split("seeker_retreat_draw_or_collide:", 1)[1]
        self.assertIn("call seeker_within_playfield\n    jp nc,table_deactivate_sprite", retreat)
        edge = source.split("move_enemy_seeker_retreat_edge:", 1)[1]
        edge = edge.split("; end move_enemy_seeker_retreat_edge", 1)[0]
        self.assertIn("jp seeker_retreat_draw_or_collide", edge)
        self.assertNotIn("screen_right", edge)
        self.assertNotIn("origin_left", edge)

        sprites = (ROOT / "src" / "asm" / "sprites.inc").read_text()
        update = sprites.split("move_sprite_group:", 1)[1]
        update, first_show = update.split("@reset_just_spawned:", 1)
        guard = "call sprite_move_publication_allowed"
        self.assertLess(update.index(guard), update.index("call vdu_sprite_move_abs168"))
        self.assertLess(first_show.index(guard), first_show.index("call vdu_sprite_show"))

    def test_forced_edge_diagnostic_is_scoped_to_game_over(self) -> None:
        source = SOURCE.read_text()
        arm = source.split(".if GAME_OVER_REVIEW_EDGE", 1)[1].split(".else", 1)[0]
        self.assertIn("ld de,game_over\n    or a\n    sbc hl,de\n    jp nz,@nearest", arm)
        self.assertIn("call seeker_enter_retreat_edge_ix", arm)
        self.assertIn("@nearest:\n    call seeker_enter_nearest_pad_retreat", arm)

    def test_death_publishes_direct_retreat_and_grace_does_not_reactivate_it(self) -> None:
        player = (ROOT / "src" / "asm" / "player_state.inc").read_text()
        death = player.split("kill_player:", 1)[1].split("; end kill_player", 1)[0]
        self.assertLess(death.index("ld (player_state),hl"), death.index("call seeker_group_enter_death_retreat"))
        self.assertNotIn("seeker_group_enter_orbit", death)
        seeker = (ROOT / "src" / "asm" / "enemy_seeker.inc").read_text()
        resume = seeker.split("seeker_group_enter_seek:", 1)[1].split("; end seeker_group_enter_seek", 1)[0]
        self.assertIn("cp seeker_state_orbit\n    jp nz,@next", resume)
        policy = SOURCE.read_text().split("seeker_retreat_draw_or_collide:", 1)[1]
        self.assertIn("ld de,player_alive\n    or a\n    sbc hl,de\n    jp z,move_enemy_seeker", policy)

    def test_source_uses_record_fields_and_only_warp_retreat_collides(self) -> None:
        source = SOURCE.read_text()
        self.assertIn("seeker_spawn_pad_id: equ seeker_custom_end", source)
        self.assertIn("seeker_spawn_pad_generation: equ seeker_spawn_pad_id+1", source)
        self.assertIn("seeker_completed_orbits: equ seeker_spawn_pad_generation+1", source)
        self.assertIn("cp (iy+actor_record_generation)", source)
        self.assertIn("call tile_table_record_pointer_valid", source)
        self.assertIn("cp seeker_retreat_orbits", source)
        self.assertIn("call check_collision_box", source)
        self.assertNotIn("check_enemy_collision_player", source)
        self.assertNotIn("check_enemy_collision_laser", source)
        policy = source.split("seeker_retreat_draw_or_collide:", 1)[1]
        self.assertIn("ld de,game_level_transition", policy)
        self.assertIn("jp z,move_enemy_seeker\n    jp sprite_frame_heading", policy)

    def test_retreat_is_integrated_through_actor_owned_transitions(self) -> None:
        production = PRODUCTION.read_text()
        seeker = (ROOT / "src" / "asm" / "enemy_seeker.inc").read_text()
        sprites = (ROOT / "src" / "asm" / "sprites.inc").read_text()
        pad = (ROOT / "src" / "asm" / "tile_pad_seeker.inc").read_text()
        self.assertIn('include "seeker_retreat.inc"', production)
        self.assertIn("call seeker_capture_spawn_pad", seeker)
        self.assertIn("call seeker_orbit_note_step", seeker)
        self.assertIn("call seeker_retreat_reset_orbits", seeker)
        self.assertIn("move_enemy_seeker_retreat_pad", sprites)
        self.assertIn("move_enemy_seeker_retreat_edge", sprites)
        self.assertNotIn("bit sprite_alive,(ix+sprite_collisions)", pad)

    def test_fixture_assembles_when_toolchain_is_available(self) -> None:
        assembler = shutil.which("ez80asm")
        if assembler is None:
            self.skipTest("ez80asm is not installed on this host")
        with tempfile.TemporaryDirectory(prefix="nurples-seeker-retreat-asm-") as temp:
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
