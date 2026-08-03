from pathlib import Path
import re
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


def block(source: str, label: str, next_label: str) -> str:
    start = source.index(f"{label}:")
    end = source.index(f"{next_label}:", start)
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

    def test_completed_level_keeps_passive_terrain_scrolling_safely(self) -> None:
        source = read_asm("tiles.inc")
        plotter = routine(source, "tiles_plot", "; end tiles_plot")
        scroll = routine(
            source,
            "tiles_scroll_background",
            "; end tiles_scroll_background",
        )

        latched_path = plotter[
            plotter.index("ld a,(tiles_level_complete)") :
            plotter.index("@row_available:")
        ]
        self.assertEqual(plotter.count("call tiles_scroll_background"), 2)
        self.assertIn("call tiles_scroll_background", latched_path)
        self.assertIn("ld a,1", latched_path)
        self.assertNotIn("(tiles_row_defs)", latched_path)
        self.assertIn("call vdu_scroll_down", scroll)
        self.assertIn("call bg_plot", scroll)

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

    def test_unimplemented_game_states_are_deterministic_noops(self) -> None:
        source = read_asm("state.inc")
        state_pairs = (
            ("game_main_menu", "game_paused"),
            ("game_paused", "game_level_transition"),
            ("game_victory", "game_loading"),
            ("game_cutscene", "game_inventory"),
            ("game_inventory", "game_map"),
            ("game_map", "game_exit"),
        )
        for label, next_label in state_pairs:
            with self.subTest(state=label):
                state = block(source, label, next_label)
                self.assertNotIn("call ", state)
                self.assertNotIn("jp ", state)
                self.assertIn("ret", state)

    def test_game_exit_returns_through_the_dispatcher(self) -> None:
        state = read_asm("state.inc")
        exit_state = block(state, "game_exit", "check_input_menu")
        self.assertIn("call save_game", exit_state)
        self.assertIn("ld (game_exit_requested),a", exit_state)
        self.assertNotIn("jp exit_game", exit_state)

        main = routine(read_asm("nurples.asm"), "main", "; end main")
        request_clear = main.index("ld (game_exit_requested),a")
        state_start = main.index("ld (game_state),hl")
        dispatch = main.index("call do_game")
        request_read = main.index("ld a,(game_exit_requested)", dispatch)
        vblank = main.index("call vdu_vblank")
        self.assertLess(request_clear, state_start)
        self.assertLess(dispatch, request_read)
        self.assertLess(request_read, vblank)
        self.assertIn("or a\n    jp nz,main_end", main[request_read:vblank])

    def test_player_outcome_is_consumed_before_world_continues(self) -> None:
        player_source = read_asm("player_state.inc")
        player_dead = routine(player_source, "player_dead", "; end player_dead")
        self.assertNotIn("jp z,game_over", player_dead)
        self.assertNotIn("ld (game_state)", player_dead)
        self.assertIn("ld (player_event),a", player_dead)

        playing = read_asm("state_game_playing.inc")
        player_tick = playing.index("call do_player")
        event_read = playing.index("ld a,(player_event)")
        enemy_tick = playing.index("call do_enemies")
        self.assertLess(player_tick, event_read)
        self.assertLess(event_read, enemy_tick)
        self.assertIn("ld hl,game_over\n    ld (game_state),hl\n    ret", playing)

    def test_level_completion_returns_to_the_global_state(self) -> None:
        tiles = routine(read_asm("tiles.inc"), "tiles_plot", "; end tiles_plot")
        self.assertNotIn("jp next_level", tiles)
        self.assertIn("ld (tiles_level_complete),a", tiles)

        playing = read_asm("state_game_playing.inc")
        self.assertIn(
            "ld hl,game_level_transition\n"
            "    ld (game_state),hl\n"
            "    ret",
            playing,
        )
        transition = block(
            read_asm("state.inc"),
            "game_level_transition",
            "game_over",
        )
        self.assertIn("call next_level", transition)
        self.assertNotIn("call load_next_level", transition)

    def test_new_session_values_precede_level_display(self) -> None:
        game_init = routine(
            read_asm("state_game_init.inc"),
            "game_init",
            "; end game_init",
        )
        score_reset = game_init.index("ld (player_score),hl")
        starting_score_reset = game_init.index("ld (player_starting_score),hl")
        level_init = game_init.index("call init_tileset")
        self.assertLess(score_reset, level_init)
        self.assertLess(starting_score_reset, level_init)
        self.assertIn("call reset_runtime_tables", game_init)

        player_dead = routine(
            read_asm("player_state.inc"),
            "player_dead",
            "; end player_dead",
        )
        self.assertIn("call reset_runtime_tables", player_dead)
        self.assertNotIn("ld (player_state)", game_init)
        self.assertNotIn("ld (player_state)", player_dead)

    def test_defaults_copy_only_initialized_definition_bytes(self) -> None:
        sprites = read_asm("sprites.inc")
        tiles = read_asm("tile_table.inc")
        active_tiles = read_asm("tiles_active.inc")
        self.assertIn(
            "sprite_definitions_size:   equ  sprite_frame-sprite_definitions+1",
            sprites,
        )
        self.assertIn(
            "tile_definitions_size:   equ  tile_frame-tile_definitions+1",
            tiles,
        )
        self.assertEqual(sprites.count("ld bc,sprite_definitions_size"), 2)
        self.assertEqual(active_tiles.count("ld bc,sprite_definitions_size"), 1)
        self.assertEqual(tiles.count("ld bc,tile_definitions_size"), 2)
        self.assertEqual(active_tiles.count("ld bc,tile_definitions_size"), 1)
        self.assertNotIn("table_record_size-sprite_definitions", sprites)
        self.assertNotIn("table_record_size-sprite_definitions", active_tiles)
        self.assertNotIn("tile_table_record_size-tile_definitions", tiles)
        self.assertNotIn("tile_table_record_size-tile_definitions", active_tiles)

        expected_defaults = {
            "player_defaults",
            "laser_defaults",
            "seeker_defaults",
            "fireball_defaults",
            "explosion_defaults",
            "pad_small_defaults",
            "turret_fireball_defaults",
            "electrode_defaults",
            "lightning_defaults",
            "crater_defaults",
        }
        measured_defaults: dict[str, int] = {}
        for path in ASM_DIR.glob("*.inc"):
            lines = path.read_text().splitlines()
            for index, line in enumerate(lines):
                match = re.fullmatch(r"([A-Za-z0-9_]+_defaults):", line)
                if match is None:
                    continue
                size = 0
                for data_line in lines[index + 1 :]:
                    directive = data_line.split(";", 1)[0].strip()
                    if directive.startswith("db "):
                        size += len(directive[3:].split(","))
                    elif directive.startswith("dl "):
                        size += 3 * len(directive[3:].split(","))
                    else:
                        break
                measured_defaults[match.group(1)] = size
        self.assertEqual(set(measured_defaults), expected_defaults)
        self.assertTrue(
            all(size == 19 for size in measured_defaults.values()),
            measured_defaults,
        )

    def test_crater_spawn_and_transform_have_separate_contexts(self) -> None:
        crater = read_asm("tile_crater.inc")
        activate = block(
            crater,
            "activate_crater",
            "transform_active_tile_to_crater",
        )
        transform = routine(
            crater,
            "transform_active_tile_to_crater",
            "; end transform_active_tile_to_crater",
        )
        self.assertIn("jp spawn_active_tile", activate)
        self.assertNotIn("spawn_active_tile_from_tile", activate)
        self.assertIn("jp spawn_active_tile_from_tile", transform)

        for name in (
            "tile_electrode.inc",
            "tile_pad_small.inc",
            "tile_turret_fireball.inc",
        ):
            with self.subTest(source=name):
                self.assertIn(
                    "jp transform_active_tile_to_crater",
                    read_asm(name),
                )

    def test_tile_defaults_use_tile_domain_types(self) -> None:
        self.assertIn(
            "db tile_turret_fireball ; type",
            read_asm("tile_turret_fireball.inc"),
        )
        self.assertIn(
            "db tile_electrode       ; type",
            read_asm("tile_electrode.inc"),
        )

    def test_level_and_tileset_selectors_are_count_bounded(self) -> None:
        source = read_asm("levels.inc")
        self.assertIn(
            "tileset_ptrs_count: equ [tileset_ptrs_end-tileset_ptrs]/3",
            source,
        )

        tileset_lookup = routine(
            source,
            "get_tileset_ptr",
            "; end get_tileset_ptr",
        )
        self.assertLess(
            tileset_lookup.index("cp tileset_ptrs_count"),
            tileset_lookup.index("mlt de"),
        )
        self.assertIn("ld (tileset_ptr_idx),a", tileset_lookup)

        level_lookup = routine(source, "get_level_ptr", "; end get_level_ptr")
        count_refresh = level_lookup.index("ld a,(ix-1)")
        bounds_check = level_lookup.index("cp b")
        pointer_multiply = level_lookup.index("mlt de")
        self.assertLess(count_refresh, bounds_check)
        self.assertLess(bounds_check, pointer_multiply)
        self.assertIn("ld (cur_level),a", level_lookup)

        for label, end_comment in (
            ("next_tileset", "; end next_tileset"),
            ("next_level", "; end next_level"),
        ):
            with self.subTest(routine=label):
                transition = routine(source, label, end_comment)
                self.assertNotIn("sign_hlu", transition)
                self.assertNotIn("list terminator", transition)

        for path in sorted(ASM_DIR.glob("levels_tileset_*.inc")):
            data = path.read_text()
            declarations = list(
                re.finditer(
                    r"(?m)^([A-Za-z0-9_]+)_num_levels:\s+db\s+(\d+)\s*$",
                    data,
                )
            )
            self.assertTrue(declarations, path.name)
            for declaration in declarations:
                prefix = declaration.group(1)
                declared_count = int(declaration.group(2))
                self.assertGreater(declared_count, 0, prefix)
                table_start = data.index(f"{prefix}_levels:")
                terminator = re.search(
                    r"(?m)^\s*dl\s+0\b",
                    data[table_start:],
                )
                self.assertIsNotNone(terminator, prefix)
                table = data[
                    table_start : table_start + terminator.start()
                ]
                entries = re.findall(r"(?m)^\s*dl\s+(?!0\b)\S+", table)
                self.assertEqual(len(entries), declared_count, prefix)

    def test_tile_allocator_searches_for_a_free_physical_record(self) -> None:
        source = read_asm("tile_table.inc")
        allocator = routine(
            source,
            "tile_table_get_next_id",
            "; end tile_table_get_next_id",
        )
        alive_check = allocator.index(
            "bit sprite_alive,(ix+tile_collisions)"
        )
        found = allocator.index("@found:")
        count_publish = allocator.index("inc (hl)", found)
        self.assertLess(alive_check, found)
        self.assertLess(found, count_publish)
        self.assertIn("ld b,0 ; DJNZ with zero visits all 256 tile records", allocator)
        self.assertIn("djnz @search", allocator)

        deactivate = routine(
            source,
            "table_deactivate_tile",
            "; end table_deactivate_tile",
        )
        self.assertIn("call tile_stack_pointer_valid", deactivate)
        self.assertIn("call tile_table_record_pointer_valid", deactivate)
        self.assertIn("res sprite_alive,(iy+tile_collisions)", deactivate)
        self.assertIn("jp tile_stack_remove_current_entry", deactivate)

        record_validator = routine(
            source,
            "tile_table_record_pointer_valid",
            "; end tile_table_record_pointer_valid",
        )
        self.assertIn("and tile_table_record_size-1", record_validator)
        self.assertIn("cp (iy+tile_id)", record_validator)

    def test_tile_stack_alignment_guard_matches_modulo_three(self) -> None:
        validator = routine(
            read_asm("tile_table.inc"),
            "tile_stack_pointer_valid",
            "; end tile_stack_pointer_valid",
        )
        self.assertIn("ld e,171\n    mlt de\n    ld a,e\n    cp 86", validator)

        for offset in range(768):
            byte_sum = (offset & 0xFF) + (offset >> 8)
            folded = (byte_sum & 0xFF) + (byte_sum >> 8)
            accepted = ((folded * 171) & 0xFF) < 86
            self.assertEqual(accepted, offset % 3 == 0, offset)

    def test_tile_traversal_validates_records_and_callbacks(self) -> None:
        source = read_asm("tiles_active.inc")
        mover = routine(source, "move_tiles", "; end move_tiles")
        cursor_check = mover.index("call tile_stack_pointer_valid")
        record_load = mover.index("ld iy,(iy)")
        record_check = mover.index("call tile_table_record_pointer_valid")
        first_record_field = mover.index("bit 7,(iy+tile_type)")
        callback_check = mover.index("call tile_move_program_valid")
        callback = mover.index("CALL_HL")
        self.assertLess(cursor_check, record_load)
        self.assertLess(record_load, record_check)
        self.assertLess(record_check, first_record_field)
        self.assertLess(callback_check, callback)
        self.assertIn("call tile_stack_quarantine_entry", mover)
        self.assertIn("push iy ; callbacks may use index registers", mover)
        self.assertIn("CALL_HL\n    pop iy", mover)

        whitelist = routine(
            source,
            "tile_move_program_valid",
            "; end tile_move_program_valid",
        )
        for tile_type, handler in (
            ("tile_pad_small", "move_pad_small"),
            ("tile_turret_fireball", "move_turret_fireball"),
            ("tile_electrode", "move_electrode"),
            ("tile_lightning", "move_lightning"),
            ("tile_crater", "move_crater"),
        ):
            with self.subTest(tile_type=tile_type):
                self.assertIn(f"cp {tile_type}", whitelist)
                self.assertIn(f"ld de,{handler}", whitelist)

    def test_electrode_scans_are_bounded_and_stack_balanced(self) -> None:
        source = read_asm("tile_electrode.inc")
        kill = routine(source, "kill_electrode", "; end move_electrode")
        source_save = kill.index("ld (@source_tile_pointer),iy")
        vdu_setup = kill.index("call vdu_buff_select")
        self.assertLess(source_save, vdu_setup)
        self.assertGreaterEqual(kill.count("ld (@tiles_remaining),a"), 4)
        self.assertGreaterEqual(kill.count("call tile_stack_pointer_valid"), 2)
        self.assertIn("call tile_table_record_pointer_valid", kill)
        self.assertIn("call tile_stack_quarantine_entry", kill)
        self.assertIn("@source_not_found:", kill)
        self.assertNotIn("ret z ; end of stack", kill)

    def test_lethal_shield_path_preserves_collision_record_indices(self) -> None:
        kill = routine(
            read_asm("player_state.inc"),
            "kill_player",
            "; end kill_player",
        )
        self.assertIn(
            "push af\n"
            "    push bc\n"
            "    push ix\n"
            "    push iy",
            kill,
        )
        self.assertIn(
            "pop iy\n"
            "    pop ix\n"
            "    pop bc\n"
            "    pop af",
            kill,
        )

    def test_reused_tile_records_clear_uninitialized_motion_state(self) -> None:
        source = read_asm("tile_table.inc")
        self.assertEqual(source.count("ld (ix+tile_vel),hl"), 2)
        self.assertEqual(source.count("ld (ix+tile_heading),hl"), 2)
        self.assertEqual(source.count("ld (ix+tile_orientation),hl"), 2)

        sprite_sources = read_asm("sprites.inc") + read_asm("tiles_active.inc")
        self.assertEqual(sprite_sources.count("ld (ix+sprite_xvel),hl"), 3)
        self.assertEqual(sprite_sources.count("ld (ix+sprite_yvel),hl"), 3)
        self.assertEqual(sprite_sources.count("ld (ix+sprite_vel),hl"), 3)
        self.assertEqual(sprite_sources.count("ld (ix+sprite_heading),hl"), 3)
        self.assertEqual(
            sprite_sources.count("ld (ix+sprite_orientation),hl"),
            3,
        )

    def test_legacy_state_dispatches_require_exact_handlers(self) -> None:
        game = read_asm("state.inc")
        do_game = block(game, "do_game", "game_state_pointer_valid")
        self.assertLess(
            do_game.index("call game_state_pointer_valid"),
            do_game.index("CALL_HL"),
        )
        self.assertIn("ld hl,game_init", do_game)
        game_whitelist = routine(
            game,
            "game_state_pointer_valid",
            "; end game_state_pointer_valid",
        )
        for handler in (
            "game_main_menu",
            "game_paused",
            "game_level_transition",
            "game_over",
            "game_victory",
            "game_loading",
            "game_cutscene",
            "game_inventory",
            "game_map",
            "game_exit",
            "game_init",
            "game_playing",
        ):
            with self.subTest(domain="game", handler=handler):
                self.assertIn(f"ld de,{handler}", game_whitelist)

        player = read_asm("player_state.inc")
        do_player = routine(player, "do_player", "; end do_player")
        self.assertLess(
            do_player.index("call player_state_pointer_valid"),
            do_player.index("CALL_HL"),
        )
        self.assertIn("ld (player_event),a", do_player)
        player_whitelist = routine(
            player,
            "player_state_pointer_valid",
            "; end player_state_pointer_valid",
        )
        for handler in ("player_alive", "player_dying", "player_dead"):
            with self.subTest(domain="player", handler=handler):
                self.assertIn(f"ld de,{handler}", player_whitelist)

        enemies = read_asm("enemies.inc")
        do_enemies = block(
            enemies,
            "do_enemies",
            "enemies_state_pointer_valid",
        )
        self.assertLess(
            do_enemies.index("call enemies_state_pointer_valid"),
            do_enemies.index("CALL_HL"),
        )
        self.assertIn("ld hl,enemies_dead", do_enemies)
        enemy_whitelist = routine(
            enemies,
            "enemies_state_pointer_valid",
            "; end enemies_state_pointer_valid",
        )
        for handler in (
            "enemies_alive",
            "enemies_despawning",
            "enemies_dead",
            "enemies_respawning",
        ):
            with self.subTest(domain="enemies", handler=handler):
                self.assertIn(f"ld de,{handler}", enemy_whitelist)

    def test_sprite_dispatch_checks_physical_pool_type_and_program(self) -> None:
        source = read_asm("sprites.inc")
        record_validator = routine(
            source,
            "sprite_table_record_pointer_valid",
            "; end sprite_table_record_pointer_valid",
        )
        self.assertIn("ld de,table_end-table_base", record_validator)
        self.assertIn("and table_record_size-1", record_validator)
        self.assertIn("cp (iy+sprite_id)", record_validator)

        whitelist = routine(
            source,
            "sprite_move_program_valid",
            "; end sprite_move_program_valid",
        )
        for pool_end in (
            "player_lasers_end_sprite_id",
            "seekers_end_sprite_id",
            "fireballs_end_sprite_id",
            "explosions_end_sprite_id",
        ):
            self.assertIn(f"cp {pool_end}", whitelist)
        for sprite_type in (
            "player_air",
            "enemy_air",
            "enemy_projectile",
            "explosion",
        ):
            self.assertIn(f"cp {sprite_type}", whitelist)
        for handler in (
            "move_player_laser",
            "en_nav_seek_zag",
            "move_enemy_seeker_orbit",
            "move_enemy_seeker_orbit_stage1",
            "move_enemy_fireball",
            "move_explosion",
        ):
            self.assertIn(f"ld de,{handler}", whitelist)

        mover = routine(source, "move_sprite_group", "; end move_sprite_group")
        self.assertLess(
            mover.index("call sprite_table_record_pointer_valid"),
            mover.index("bit 7,(iy+sprite_type)"),
        )
        self.assertLess(
            mover.index("call sprite_move_program_valid"),
            mover.index("CALL_HL"),
        )
        self.assertIn("CALL_HL ; run the behavior subroutine\n    pop iy", mover)
        self.assertIn("call table_deactivate_sprite", mover)

        seeker = read_asm("enemy_seeker.inc")
        self.assertIn("move_enemy_seeker_orbit_stage1:", seeker)
        self.assertNotIn("ld hl,@stage1", seeker)

        tables = read_asm("tables.inc")
        self.assertIn(
            "table_num_records: equ "
            "[table_end-table_base]/table_record_size",
            tables,
        )
        self.assertIn(
            "table_num_enemy_records: equ "
            "[enemy_sprites_end-enemy_sprites_begin]/table_record_size",
            tables,
        )

    def test_player_sprite_frame_loop_uses_the_djnz_register(self) -> None:
        sprites = read_asm("sprites.inc")
        initialization = routine(
            sprites,
            "sprites_init",
            "; end sprites_init",
        )
        self.assertIn(
            "ld b,3 ; DJNZ consumes B; loading BC with 3 would put zero in B",
            initialization,
        )
        self.assertNotIn("ld bc,3", initialization)
        self.assertIn("djnz @sprite_player_loop", initialization)

    def test_sprites_are_defined_before_the_contiguous_set_is_activated(self) -> None:
        initialization = routine(
            read_asm("sprites.inc"),
            "sprites_init",
            "; end sprites_init",
        )

        self.assertNotIn("@sprite_loop", initialization)
        self.assertNotIn("BUF_CRATER_0", initialization)
        self.assertEqual(initialization.count("call vdu_sprite_activate"), 1)
        self.assertLess(
            initialization.index("call init_explosions"),
            initialization.index("call vdu_sprite_activate"),
        )
        self.assertIn(
            "ld a,table_num_records\n\tcall vdu_sprite_activate",
            initialization,
        )

    def test_fireball_sprite_frames_match_declared_animation_banks(self) -> None:
        source = read_asm("enemy_fireball.inc")
        initialization = routine(
            source,
            "init_enemy_fireballs",
            "; end init_enemy_fireballs",
        )

        self.assertIn("db 32                   ; num_orientations", source)
        self.assertIn("db 2                    ; num_animations", source)
        self.assertIn(
            "ld b,2*32 ; 2 animations * 32 orientations",
            initialization,
        )
        self.assertNotIn("ld b,4*32", initialization)

    def test_seeker_edge_collision_cannot_move_twice_in_one_frame(self) -> None:
        behavior = routine(
            read_asm("enemy_seeker.inc"),
            "en_nav_seek_zag",
            "; end en_nav_seek_zag",
        )
        self.assertEqual(behavior.count("call move_sprite"), 1)
        self.assertIn(
            "call move_sprite ; move the sprite\n"
            "    or a ; a will be non-zero if have collided with screen edge\n"
            "    jp nz,@edge_switch",
            behavior,
        )
        edge_start = behavior.index("@edge_switch:")
        edge_end = behavior.index("@switch_zag_heading:", edge_start)
        edge_path = behavior[edge_start:edge_end]
        self.assertIn("call @switch_zag_heading", edge_path)
        self.assertIn("call @reset_zig_zag_timer", edge_path)
        self.assertIn("jp move_enemy_seeker", edge_path)
        self.assertNotIn("@zig_zag_no_switch", edge_path)

    def test_file_loaders_fail_closed_and_preserve_font_cursor(self) -> None:
        fonts = routine(read_asm("fonts.inc"), "fonts_load", "; all done")
        self.assertIn(
            "push ix ; mos_fread does not preserve the font-list cursor\n"
            "    call vdu_load_buffer_from_file\n"
            "    pop ix\n"
            "    jp nc,@next_font",
            fonts,
        )

        source = read_asm("vdu.inc")
        loader = block(
            source,
            "vdu_load_buffer_from_file",
            "@filehandle",
        )
        self.assertIn("MOSCALL mos_fopen\n    or a\n    ret z", loader)
        self.assertIn(
            "ld a,(@filehandle)\n"
            "    ld c,a\n"
            "    MOSCALL mos_fclose\n"
            "    scf",
            loader,
        )

        image = block(source, "vdu_load_img", "vdu_load_buffer_from_file")
        self.assertIn("call vdu_load_buffer_from_file\n    jp c,@loaded", image)
        self.assertIn(
            "pop de\n"
            "\tpop bc\n"
            "\tpop af\n"
            "    or a ; preserve A while reasserting the carry-clear failure contract\n"
            "    ret",
            image,
        )
        self.assertIn("call vdu_bmp_create\n    scf\n    ret", image)

    def test_clock_waits_have_bounded_fail_open_paths(self) -> None:
        source = read_asm("vdu.inc")
        vblank = block(source, "vdu_vblank", "vdu_set_gfx_origin")
        self.assertIn("ld bc,0 ; 65,536 polls before failing open", vblank)
        self.assertIn("dec c\n    jp nz,@wait\n    djnz @wait", vblank)
        self.assertIn("ld (vdu_timing_fault_flags),a", vblank)
        self.assertIn("push bc\n    push de\n    push ix", vblank)
        self.assertIn("pop ix\n    pop de\n    pop bc", vblank)

        timer = read_asm("timer.inc")
        delay = block(
            timer,
            "multiPurposeDelay",
            "timer_fault_delay_timeout",
        )
        self.assertIn("ld a,32", delay)
        self.assertIn("ld b,0\n    ld d,0", delay)
        self.assertIn("dec d\n    jp nz,waitLoop\n    djnz waitLoop", delay)
        self.assertIn("ld (timer_fault_flags),a", delay)
        self.assertIn("push de", delay)
        self.assertIn("pop de", delay)


if __name__ == "__main__":
    unittest.main()
