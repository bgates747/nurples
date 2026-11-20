"""Host contract tests for deterministic safe player spawn endpoints."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "asm" / "player_spawn_resolver.inc"
FIXTURE = ROOT / "tests" / "asm" / "player_spawn_resolver_fixture.asm"
PRODUCTION = ROOT / "src" / "asm" / "nurples.asm"

X_CANDIDATES = (
    120,
    104,
    136,
    88,
    152,
    72,
    168,
    56,
    184,
    40,
    200,
    24,
    216,
    8,
    232,
    0,
    240,
)
Y_CANDIDATES = (224, 208, 192, 176)


@dataclass(frozen=True)
class Tile:
    x: int
    y: int
    width: int
    height: int
    damage: int = 1


def overlaps_inclusive(a: tuple[int, int, int, int], tile: Tile) -> bool:
    ax, ay, aw, ah = a
    return not (
        ax + aw < tile.x
        or tile.x + tile.width < ax
        or ay + ah < tile.y
        or tile.y + tile.height < ay
    )


def resolve(tiles: tuple[Tile, ...]) -> tuple[int, int, int] | None:
    collidables = tuple(tile for tile in tiles if tile.damage != 0)
    for x in X_CANDIDATES:
        for y in Y_CANDIDATES:
            probe = (x, y - 64, 16, 80)
            if not any(overlaps_inclusive(probe, tile) for tile in collidables):
                direction = 0 if x >= 120 else 1
                return x, y, direction
    return None


class PlayerSpawnResolverTests(unittest.TestCase):
    def test_search_order_and_wake_geometry(self) -> None:
        self.assertEqual(resolve(()), (120, 224, 0))
        self.assertEqual(resolve((Tile(0, 112, 255, 128, 0),)), (120, 224, 0))
        self.assertEqual(resolve((Tile(127, 112, 1, 128),)), (104, 224, 1))
        self.assertEqual(
            resolve((Tile(127, 112, 1, 128), Tile(111, 112, 1, 128))),
            (136, 224, 0),
        )
        self.assertEqual(resolve((Tile(127, 239, 1, 1),)), (120, 208, 0))
        self.assertIsNone(resolve((Tile(0, 112, 255, 128),)))

    def test_search_grid_covers_frozen_legal_positions(self) -> None:
        self.assertEqual(len(X_CANDIDATES), len(set(X_CANDIDATES)))
        self.assertEqual(X_CANDIDATES[:3], (120, 104, 136))
        self.assertEqual(X_CANDIDATES[-2:], (0, 240))
        self.assertTrue(all(0 <= value <= 240 for value in X_CANDIDATES))
        self.assertEqual(Y_CANDIDATES, (224, 208, 192, 176))

    def test_source_reuses_validators_and_collision_contract(self) -> None:
        source = SOURCE.read_text()
        self.assertIn("call tile_stack_pointer_valid", source)
        self.assertIn("call tile_table_record_pointer_valid", source)
        self.assertIn("call check_collision_box", source)
        self.assertIn("bit sprite_alive,(iy+tile_collisions)", source)
        self.assertIn("ld a,(iy+tile_shield_damage)", source)
        self.assertIn("sub 64", source)
        self.assertIn("ld a,80", source)
        self.assertIn("set player_spawn_fault_no_candidate", source)

    def test_resolver_is_integrated_through_player_owned_fields(self) -> None:
        production = PRODUCTION.read_text()
        player = (ROOT / "src" / "asm" / "player_state.inc").read_text()
        self.assertIn('include "player_spawn_resolver.inc"', production)
        self.assertIn("call player_spawn_resolve_endpoint", player)
        self.assertIn("player_entry_target_x_actor:", player)
        self.assertIn("player_entry_target_y_actor:", player)
        self.assertIn("call player_entry_init_lateral", player)
        lateral = player.split("player_entry_init_lateral:\n", 1)[1].split(
            "; end player_entry_init_lateral", 1,
        )[0]
        self.assertIn("ld a,(ix+player_entry_target_x_actor)", lateral)
        self.assertIn("ld a,sprite_right\n    sub b", lateral)
        self.assertIn("ld (ix+player_entry_angle),c", lateral)
        self.assertIn("ld (ix+player_entry_amplitude),a", lateral)
        self.assertLess(
            player.index("call player_spawn_resolve_endpoint"),
            player.index("ld hl,player_spawning"),
        )
        self.assertNotIn("call player_spawn_resolve_endpoint", production)

    def test_fixture_assembles_when_toolchain_is_available(self) -> None:
        assembler = shutil.which("ez80asm")
        if assembler is None:
            self.skipTest("ez80asm is not installed on this host")
        with tempfile.TemporaryDirectory(prefix="nurples-player-spawn-asm-") as temp:
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
