from pathlib import Path
import shutil
import subprocess
from tempfile import TemporaryDirectory
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[1]
ASM_DIR = PROJECT_ROOT / "src/asm"


def read_asm(name: str) -> str:
    return (ASM_DIR / name).read_text()


def routine(source: str, label: str, end_comment: str) -> str:
    start = source.index(f"{label}:")
    end = source.index(end_comment, start)
    return source[start:end]


class AssemblySafetyTests(unittest.TestCase):
    def test_current_game_assembles(self) -> None:
        assembler = shutil.which("ez80asm") or str(
            Path.home() / ".local/bin/ez80asm"
        )
        with TemporaryDirectory() as temp_dir:
            temp_asm = Path(temp_dir) / "asm"
            shutil.copytree(
                ASM_DIR,
                temp_asm,
                ignore=shutil.ignore_patterns("*.lst"),
            )
            output = Path(temp_dir) / "nurples.bin"
            result = subprocess.run(
                [assembler, "-l", "nurples.asm", str(output)],
                cwd=temp_asm,
                capture_output=True,
                check=False,
                text=True,
            )

            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertGreater(output.stat().st_size, 0)

    def test_player_spawn_failures_are_guarded_before_ix_use(self) -> None:
        player_source = read_asm("player_state.inc")
        player_init = routine(player_source, "player_init", "; end player_init")
        self.assertIn(
            "call spawn_sprite_from_coords\n"
            "    ret nc ; IX is invalid when the one-record player pool is full",
            player_init,
        )
        self.assertIn(
            "set sprite_alive,(ix+sprite_collisions)\n"
            "    scf ; publish successful initialization to the caller",
            player_init,
        )

        player_dying = routine(
            player_source,
            "player_dying",
            "; end player_dying",
        )
        self.assertIn(
            "call spawn_explosion_player\n"
            "    jp nc,@animate_initial",
            player_dying,
        )
        self.assertIn(
            "call spawn_explosion_player\n"
            "    jp nc,@next_finale_spawn",
            player_dying,
        )

        explosion_source = read_asm("explosion.inc")
        spawn_player = routine(
            explosion_source,
            "spawn_explosion_player",
            "; end spawn_explosion_player",
        )
        self.assertIn("call spawn_sprite_from_sprite\n    ret c", spawn_player)
        self.assertIn("call table_deactivate_sprite\n    xor a", spawn_player)

    def test_explosion_lifetime_counter_is_explicit(self) -> None:
        explosion_source = read_asm("explosion.inc")
        self.assertIn(
            "explosion_lifetime_updates: equ "
            "explosion_animation_count*explosion_animation_ticks+1",
            explosion_source,
        )
        self.assertIn(
            "ld b,explosion_lifetime_updates",
            read_asm("player_state.inc"),
        )

    def test_adl_pointer_multiply_results_are_zero_extended(self) -> None:
        levels = read_asm("levels.inc")
        self.assertEqual(
            levels.count(
                "ld de,0 ; MLT writes DE[15:0] only; clear DEU before pointer arithmetic"
            ),
            2,
        )

        tiles = read_asm("tiles.inc")
        self.assertIn(
            "ld hl,tile_activation_routines\n"
            "    ld de,0 ; MLT writes DE[15:0] only; clear DEU before pointer arithmetic",
            tiles,
        )

        tile_table = read_asm("tile_table.inc")
        self.assertIn(
            "ld de,0 ; MLT writes DE[15:0] only; clear DEU before pointer arithmetic",
            tile_table,
        )
        self.assertIn(
            "ld hl,0 ; MLT writes HL[15:0] only; clear HLU before pointer arithmetic",
            tile_table,
        )

        sine = routine(read_asm("fixed168.inc"), "sin168", "polar_to_cartesian:")
        self.assertIn(
            "mlt hl\n"
            "    ld de,0 ; MLT leaves HLU unchanged, so copy only the 16-bit product\n"
            "    ld d,h\n"
            "    ld e,l\n"
            "    ld hl,sin_lut_168",
            sine,
        )

    def test_tile_activation_table_covers_every_accepted_id(self) -> None:
        source = read_asm("tiles_active.inc")
        table = routine(
            source,
            "tile_activation_routines",
            "tile_activation_routines_end:",
        )
        entries = [
            line.split(";", 1)[0].strip()
            for line in table.splitlines()[1:]
            if line.strip().startswith("dl ")
        ]
        self.assertEqual(len(entries), 17)
        self.assertEqual(entries[0], "dl activate_null_tile")
        self.assertEqual(entries[16], "dl activate_crater")
        self.assertIn(
            "cp tile_activation_routines_count",
            read_asm("tiles.inc"),
        )

    def test_shield_maximum_is_loaded_at_byte_width(self) -> None:
        source = read_asm("player_shields.inc")
        init = routine(source, "init_bar_shields", "; end init_bar_shields")
        self.assertNotIn("ld hl,(player_shields_max)", init)
        self.assertIn(
            "ld hl,0\n"
            "    ld a,(player_shields_max)\n"
            "    ld l,a\n"
            "    ld (ix+bar_val),hl",
            init,
        )


if __name__ == "__main__":
    unittest.main()
