from pathlib import Path
import re
import shutil
import struct
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


def read_symbol_addresses(path: Path) -> dict[str, int]:
    addresses: dict[str, int] = {}
    # The native assembler also emits internal, non-ASCII local symbol names.
    # Parse only the public ASCII records; do not decode the entire file.
    for line in path.read_bytes().splitlines():
        match = re.fullmatch(rb"([!-~]+) \$([0-9A-Fa-f]+)", line)
        if match is not None:
            addresses[match.group(1).decode("ascii")] = int(match.group(2), 16)
    return addresses


class AssemblySafetyTests(unittest.TestCase):
    def test_current_game_assembles(self) -> None:
        assembler = shutil.which("ez80asm")
        if assembler is None:
            self.skipTest("ez80asm is not installed on this host")
        with TemporaryDirectory() as temp_dir:
            temp_asm = Path(temp_dir) / "asm"
            shutil.copytree(
                ASM_DIR,
                temp_asm,
                ignore=shutil.ignore_patterns("*.lst", "*.symbols"),
            )
            output = Path(temp_dir) / "nurples.bin"
            result = subprocess.run(
                [assembler, "-l", "-s", "nurples.asm", str(output)],
                cwd=temp_asm,
                capture_output=True,
                check=False,
                text=True,
            )

            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertGreater(output.stat().st_size, 0)
            symbols = read_symbol_addresses(temp_asm / "nurples.symbols")
            telemetry_begin = symbols["telemetry_begin"]
            expected_offsets = {
                "telemetry_magic": 0x00,
                "telemetry_version_major": 0x04,
                "telemetry_version_minor": 0x05,
                "telemetry_size": 0x06,
                "telemetry_write_seq": 0x08,
                "telemetry_flags": 0x09,
                "telemetry_global_current": 0x10,
                "telemetry_player_current": 0x16,
                "telemetry_enemies_current": 0x1C,
                "telemetry_total_frames": 0x20,
                "telemetry_pending_events": 0x35,
                "telemetry_fault_domains": 0x3C,
                "telemetry_rows_remaining": 0x4D,
                "telemetry_end": 0x50,
            }
            for symbol, expected_offset in expected_offsets.items():
                with self.subTest(telemetry_symbol=symbol):
                    self.assertEqual(
                        symbols[symbol] - telemetry_begin,
                        expected_offset,
                    )

            binary = output.read_bytes()
            for recipe, prefix, folder, master, destination, height in (
                ("splash", "splash_title", "ui", 45, 46, 89),
                ("game_over", "game_over", "overlays", 63_990, 63_991, 64),
            ):
                with self.subTest(title=recipe):
                    commands = (bytes((23, 0, 0xA0))
                                + struct.pack("<HBBHH", destination, 72, 0x10, master, 63_984)
                                + bytes((23, 27, 0x20)) + struct.pack("<H", destination)
                                + bytes((23, 27, 0x21)) + struct.pack("<HHB", 240, height, 1))
                    packet = symbols[f"title_phase_{recipe}_vdu"] - 0x040000
                    self.assertEqual(binary[packet:packet+len(commands)], commands)
                    lookup = symbols[f"{prefix}_lut"] - 0x040000
                    assets = PROJECT_ROOT / "assets/rgba2" / folder
                    self.assertEqual(
                        (assets / f"{prefix}_0.rgba2").read_bytes().translate(
                            binary[lookup:lookup+256],
                        ),
                        (assets / f"{prefix}_1.rgba2").read_bytes(),
                    )

    def test_agnb_startup_uses_two_containers_and_exact_progress_count(self) -> None:
        images = read_asm("images.inc")
        startup = read_asm("state_game_init.inc")
        counts = read_asm("images_agnb.inc")

        self.assertIn('ui_agnb_filename:   db "ui.agnb",0', images)
        self.assertIn('game_agnb_filename: db "game.agnb",0', images)
        self.assertIn("game_num_images: equ 380", counts)
        self.assertIn("ld hl,game_num_images\n    ld (game_load_remaining),hl", images)
        self.assertIn("SPLASH_TITLE_X: equ 28", images)
        self.assertIn("SPLASH_TITLE_Y: equ 50", images)
        self.assertIn("ld (game_load_callback_overflow),a", images)
        self.assertIn("call load_ui_images", startup)
        self.assertIn("call load_game_images", startup)
        self.assertNotIn("call load_backgrounds", startup)
        self.assertNotIn("call load_sprite_images", startup)
        self.assertNotIn("call load_tilesets", startup)

        init = routine(images, "img_load_init", "img_load_agnb_progress:")
        progress = routine(images, "img_load_agnb_progress", "splash_print_progress:")
        self.assertIn("BUF_SPLASH_BG", init)
        self.assertNotIn("BUF_SPLASH_BG", progress)
        self.assertNotIn("vdu_cls", images)
        self.assertNotIn("vdu_flip", images)
        self.assertIn("call splash_update_title\n    call splash_print_progress", progress)
        self.assertIn("call vdu_move_cursor", images)
        self.assertIn('asciz "Loading images: "', images)

        ui_loader = block(images, "load_ui_images", "load_game_images")
        game_loader = routine(images, "load_game_images", 'include "title_phase_vdp.inc"')
        self.assertIn("ret nz\n    call title_phase_build_splash", ui_loader)
        self.assertIn("jr nz,@count_error\n    call title_phase_build_game_over", game_loader)

    def test_game_init_unconditionally_rebuilds_all_vdp_resources(self) -> None:
        active_source = read_asm("nurples.asm") + "".join(
            path.read_text() for path in sorted(ASM_DIR.glob("*.inc"))
        )
        self.assertNotIn("game_assets_loaded", active_source)

        startup = routine(
            read_asm("state_game_init.inc"),
            "game_init",
            "; end game_init",
        )
        system_font = startup.index("ld hl,65535")
        select_font = startup.index("call vdu_font_select", system_font)
        reset_sprites = startup.index("call vdu_sprite_reset", select_font)
        clear_buffers = startup.index(
            "call vdu_clear_all_buffers",
            reset_sprites,
        )
        load_font = startup.index("call fonts_load", clear_buffers)
        load_ui = startup.index("call load_ui_images", load_font)
        load_game = startup.index("call load_game_images", load_ui)
        begin_config = startup.index("call game_start_config_begin", load_game)

        self.assertIn("xor a\n    call vdu_font_select", startup)
        self.assertLess(system_font, select_font)
        self.assertLess(select_font, reset_sprites)
        self.assertLess(reset_sprites, clear_buffers)
        self.assertLess(clear_buffers, load_font)
        self.assertLess(load_font, load_ui)
        self.assertLess(load_ui, load_game)
        self.assertLess(load_game, begin_config)
        flow = read_asm("presentation_flow_states.inc")
        attract = routine(flow, "game_attract_begin", "; end game_attract_begin")
        prepare_playfield = attract.index("call game_prepare_playfield")
        reset_tables = attract.index("call reset_runtime_tables", prepare_playfield)
        enter_attract = attract.index("call attract_lifecycle_enter", reset_tables)
        self.assertLess(prepare_playfield, reset_tables)
        self.assertLess(reset_tables, enter_attract)
        config = routine(flow, "game_start_config", "; end game_start_config")
        self.assertIn("jp game_attract_begin", config)
        start_session = routine(flow, "game_start_session", "; end game_start_session")
        redraw = start_session.index("call game_prepare_playfield")
        initialize_session = start_session.index("call game_new_session_init", redraw)
        self.assertLess(redraw, initialize_session)
        session = routine(
            read_asm("state_game_init.inc"),
            "game_new_session_init",
            "; end game_new_session_init",
        )
        self.assertIn("call reset_runtime_tables", session)
        self.assertEqual(startup.count("call vdu_clear_all_buffers"), 1)
        self.assertNotIn("@assets_loaded", startup)

    def test_process_entry_captures_display_state_and_exit_releases_vdp_state(self) -> None:
        source = read_asm("nurples.asm")
        entry = block(source, "main", "main_loop")
        telemetry_init = entry.index("call telemetry_init")
        runtime_init = entry.index("call runtime_process_init")
        get_mode = entry.index("call vdu_get_screen_mode")
        save_mode = entry.index("ld (original_screen_mode),a")
        get_foreground = entry.index("ld a,128", save_mode)
        get_background = entry.index("ld a,129", get_foreground)

        self.assertLess(telemetry_init, runtime_init)
        self.assertLess(runtime_init, get_mode)
        self.assertLess(get_mode, save_mode)
        self.assertLess(save_mode, get_foreground)
        self.assertLess(get_foreground, get_background)
        self.assertEqual(entry.count("call vdu_get_text_colour"), 2)
        self.assertNotIn("call vdu_clear_all_buffers", entry)
        self.assertEqual(source.count("call vdu_get_screen_mode"), 1)

        shutdown = routine(source, "main_end", "; end main")
        system_font = shutdown.index("ld hl,65535")
        select_font = shutdown.index("call vdu_font_select", system_font)
        reset_sprites = shutdown.index("call vdu_sprite_reset", select_font)
        clear_buffers = shutdown.index(
            "call vdu_clear_all_buffers",
            reset_sprites,
        )
        restore_mode = shutdown.index(
            "ld a,(original_screen_mode)",
            clear_buffers,
        )
        text_cursor = shutdown.index("call vdu_select_text_cursor", restore_mode)
        post_mode_system_font = shutdown.index("ld hl,65535", text_cursor)
        black_background = shutdown.index("ld a,c_black+128", post_mode_system_font)
        white_foreground = shutdown.index("ld a,c_white", black_background)
        clear_screen = shutdown.index("call vdu_cls", white_foreground)
        cursor_on = shutdown.index("call vdu_cursor_on", clear_screen)
        farewell = shutdown.index('asciz "Thank you for playing\\r\\n"', cursor_on)
        restore_background = shutdown.index(
            "ld a,(original_text_background)",
            farewell,
        )
        restore_foreground = shutdown.index(
            "ld a,(original_text_foreground)",
            restore_background,
        )
        self.assertLess(system_font, select_font)
        self.assertLess(select_font, reset_sprites)
        self.assertLess(reset_sprites, clear_buffers)
        self.assertLess(clear_buffers, restore_mode)
        self.assertLess(restore_mode, text_cursor)
        self.assertLess(text_cursor, post_mode_system_font)
        self.assertLess(post_mode_system_font, black_background)
        self.assertLess(black_background, white_foreground)
        self.assertLess(white_foreground, clear_screen)
        self.assertLess(clear_screen, cursor_on)
        self.assertLess(cursor_on, farewell)
        self.assertLess(farewell, restore_background)
        self.assertLess(restore_background, restore_foreground)

    def test_text_colour_query_is_bounded_and_fail_closed(self) -> None:
        query = routine(
            read_asm("vdu.inc"),
            "vdu_get_text_colour",
            "; end vdu_get_text_colour",
        )

        self.assertIn("res 2,(ix+sysvar_vpd_pflags)", query)
        self.assertIn("ld a,$94", query)
        self.assertIn("ld bc,0 ; at most 65,536 polls", query)
        self.assertIn("bit 2,(ix+sysvar_vpd_pflags)", query)
        self.assertIn("ld a,(ix+sysvar_scrpixelIndex)", query)
        self.assertIn("cp 64", query)
        self.assertIn("set vdu_timing_fault_colour_query,(hl)", query)
        self.assertIn("xor a ; deterministic failure value and carry clear", query)
        self.assertIn("scf", query)

    def test_process_init_resets_all_cross_run_state(self) -> None:
        source = read_asm("runtime_init.inc")
        initialization = routine(
            source,
            "runtime_process_init",
            "; end runtime_process_init",
        )
        all_includes = "".join(
            path.read_text() for path in sorted(ASM_DIR.glob("*.inc"))
        )
        declared_faults = set(
            re.findall(
                r"(?m)^([a-z_]+_fault_flags):\s+db\b",
                all_includes,
            )
        )
        expected_faults = {
            "game_fault_flags",
            "enemies_fault_flags",
            "player_fault_flags",
            "sprite_fault_flags",
            "tile_fault_flags",
            "timer_fault_flags",
            "vdu_timing_fault_flags",
        }
        self.assertEqual(declared_faults, expected_faults)
        for symbol in sorted(expected_faults):
            with self.subTest(symbol=symbol):
                self.assertIn(f"ld ({symbol}),a", initialization)

        for symbol in (
            "game_exit_requested",
            "player_event",
            "seekers_count",
            "sprite_screen_edge",
            "oldTimeStamp",
        ):
            with self.subTest(symbol=symbol):
                self.assertIn(f"ld ({symbol}),a", initialization)

        for pointer, handler in (
            ("game_state", "game_init"),
            ("enemies_state", "enemies_alive"),
            ("player_state", "player_alive"),
        ):
            with self.subTest(pointer=pointer):
                self.assertIn(
                    f"ld hl,{handler}\n    ld ({pointer}),hl",
                    initialization,
                )

        self.assertIn("ld (seed1),hl", initialization)
        self.assertIn("ld (seed2),hl", initialization)
        self.assertIn("ld a,$50\n    ld (r_seed),a", initialization)
        self.assertIn("call timestamp_init", initialization)

    def test_font_breadcrumb_follows_successful_creation(self) -> None:
        fonts = read_asm("fonts.inc")
        create = fonts.index("call vdu_font_create")
        select = fonts.index("call vdu_font_select", create)
        breadcrumb = fonts.index("ld a,'.'", select)
        self.assertLess(create, select)
        self.assertLess(select, breadcrumb)
        self.assertIn("jr nc,@load_failed", fonts)

    def test_player_spawn_failures_are_guarded_before_ix_use(self) -> None:
        player_source = read_asm("player_state.inc")
        player_init = routine(player_source, "player_init", "; end player_init")
        self.assertIn(
            "call spawn_sprite_from_coords\n"
            "    jp nc,@allocation_failed ; IX is invalid when the one-record player pool is full",
            player_init,
        )
        self.assertIn("@allocation_failed:\n    pop af\n    xor a\n    ret", player_init)
        self.assertIn(
            "res sprite_alive,(ix+sprite_collisions)",
            player_init,
        )
        self.assertIn("ld hl,player_spawning", player_init)
        self.assertIn("scf ; publish successful initialization to the caller", player_init)

        player_dying = routine(
            player_source,
            "player_dying",
            "; end player_dying",
        )
        self.assertIn(
            "call spawn_explosion_player\n"
            "    jp nc,@initial_spawned",
            player_dying,
        )
        self.assertIn(
            "call spawn_explosion_player\n"
            "    jp nc,@finale_spawned",
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
            "player_dying_finish_frames: equ explosion_lifetime_updates-1",
            read_asm("player_state.inc"),
        )
        self.assertIn(
            "cp player_dying_finish_frames",
            read_asm("player_state.inc"),
        )

    def test_runtime_telemetry_uses_stable_ids_and_coherent_snapshots(self) -> None:
        telemetry = read_asm("telemetry.inc")
        state_ids = read_asm("state_ids.inc")
        main = routine(read_asm("nurples.asm"), "main", "; end main")

        self.assertIn('telemetry_magic:                    db "PNST" ; 00', telemetry)
        self.assertIn(
            "telemetry_size:                     dw telemetry_end-telemetry_begin ; 06",
            telemetry,
        )
        self.assertIn("TELEMETRY_VERSION_MAJOR: equ 1", telemetry)
        self.assertIn("TELEMETRY_VERSION_MINOR: equ 0", telemetry)
        self.assertIn(
            "telemetry_write_seq:                db 1 ; 08; invalid until",
            telemetry,
        )
        self.assertIn("or 1\n    ld (telemetry_write_seq),a", telemetry)
        self.assertIn("and %11111110\n    ld (telemetry_write_seq),a", telemetry)
        self.assertLess(
            main.index("call telemetry_init"),
            main.index("call runtime_process_init"),
        )
        self.assertEqual(main.count("call telemetry_tick"), 2)
        self.assertIn("call telemetry_publish_terminal", main)

        public_end = telemetry.index("telemetry_end:")
        for private_latch in (
            "telemetry_event_latch:",
            "telemetry_lifecycle_latch:",
            "telemetry_forced_outcome:",
            "telemetry_count_frame:",
            "telemetry_legacy_phase:",
        ):
            with self.subTest(private_latch=private_latch):
                self.assertGreater(telemetry.index(private_latch), public_end)
        self.assertIn("ld (telemetry_count_frame),a", telemetry)
        self.assertGreaterEqual(
            telemetry.count("ld a,(telemetry_count_frame)"),
            4,
        )

        for pointer in ("game_state", "player_state", "enemies_state"):
            with self.subTest(pointer=pointer):
                self.assertIn(f"ld hl,({pointer})", telemetry)
        for numeric_id in (
            "STATE_NEW_PLAYING",
            "STATE_NEW_LEVEL_TRANSITION",
            "STATE_NEW_PLAYER_ACTIVE",
            "STATE_NEW_PLAYER_DYING",
            "STATE_TELEMETRY_ENEMY_ACTIVE",
        ):
            with self.subTest(numeric_id=numeric_id):
                self.assertIn(f"{numeric_id}:", state_ids)

        state_core = read_asm("state_core.inc")
        state_scaffold = read_asm("state_new.inc")
        self.assertNotIn("STATE_NEW_PLAYING:", state_core)
        self.assertIn("include state_ids.inc first", state_core)
        self.assertIn('include "state_core.inc"', state_scaffold)
        self.assertIn('include "state_graph.inc"', state_scaffold)

    def test_bounded_startup_publishes_semantic_phases_without_actor_staleness(self) -> None:
        source = read_asm("state_game_init.inc")
        startup = routine(
            source,
            "game_init",
            "; end game_init",
        )
        session = routine(
            source,
            "game_new_session_init",
            "; end game_new_session_init",
        )
        for phase in ("STATE_NEW_LOADING", "STATE_NEW_TITLE"):
            marker = f"ld a,{phase}\n    call telemetry_publish_phase"
            self.assertEqual(startup.count(marker), 1)
        for phase in ("STATE_NEW_NEW_GAME", "STATE_NEW_LEVEL_ENTER"):
            marker = f"ld a,{phase}\n    call telemetry_publish_phase"
            self.assertEqual(session.count(marker), 1)
        self.assertLess(
            startup.index("ld a,STATE_NEW_LOADING"),
            startup.index("ld a,STATE_NEW_TITLE"),
        )
        self.assertLess(
            session.index("ld a,STATE_NEW_NEW_GAME"),
            session.index("ld a,STATE_NEW_LEVEL_ENTER"),
        )
        self.assertLess(
            startup.index("ld a,STATE_NEW_TITLE"),
            startup.index("call game_start_config_begin"),
        )
        self.assertLess(
            startup.index("ld a,STATE_NEW_EVENT_LOAD_COMPLETE"),
            startup.index("ld a,STATE_NEW_TITLE"),
        )
        self.assertNotIn("call waitKeypress", startup.split("@font_load_error:", 1)[0])
        self.assertNotIn("call game_new_session_init", startup)
        flow = read_asm("presentation_flow_states.inc")
        start_session = routine(flow, "game_start_session", "; end game_start_session")
        self.assertIn("call game_new_session_init", start_session)
        self.assertLess(
            session.index("ld a,STATE_NEW_EVENT_START"),
            session.index("ld a,STATE_NEW_NEW_GAME"),
        )
        self.assertLess(
            session.index("ld a,STATE_NEW_LEVEL_ENTER"),
            session.index("call init_tileset"),
        )

        telemetry = read_asm("telemetry.inc")
        self.assertIn("ld a,(telemetry_legacy_phase)", telemetry)
        self.assertIn(
            "bit TELEMETRY_FLAG_ACTORS_VALID,a\n"
            "    jp nz,@map_pointer\n"
            "    ld a,STATE_NEW_PLAYER_INACTIVE",
            telemetry,
        )
        self.assertIn(
            "cp STATE_NEW_PLAYING\n"
            "    jp c,@startup\n"
            "    ret nz ; later phases retain",
            telemetry,
        )
        self.assertIn(
            "bit TELEMETRY_FLAG_ACTORS_VALID,a\n"
            "    jp nz,@map_pointer\n"
            "    ld a,STATE_TELEMETRY_ENEMY_INACTIVE",
            telemetry,
        )

    def test_runtime_events_are_captured_before_legacy_transitions(self) -> None:
        playing = read_asm("state_game_playing.inc")
        level_event = playing.index("ld a,STATE_NEW_EVENT_LEVEL_COMPLETE")
        level_transition = playing.index("ld hl,game_level_transition")
        no_lives_event = playing.index("ld a,STATE_NEW_EVENT_NO_LIVES")
        game_over = playing.index("ld hl,game_over")
        self.assertLess(level_event, level_transition)
        self.assertLess(no_lives_event, game_over)

        player_dying = routine(
            read_asm("player_state.inc"),
            "player_dying",
            "; end player_dying",
        )
        dead_event = player_dying.index("ld a,STATE_NEW_EVENT_PLAYER_DEAD")
        dead_lifecycle = player_dying.index(
            "ld a,TELEMETRY_LIFECYCLE_PLAYER_DEAD"
        )
        dead_state = player_dying.index("ld hl,player_dead")
        self.assertLess(dead_event, dead_state)
        self.assertLess(dead_event, dead_lifecycle)
        self.assertLess(dead_lifecycle, dead_state)

        game_over = routine(read_asm("state.inc"), "game_over", "game_victory:")
        self.assertIn("jp game_over_bounded", game_over)
        flow = read_asm("presentation_flow_states.inc")
        bounded = routine(flow, "game_over_bounded", "; end game_over_bounded")
        self.assertIn("jp game_start_session", bounded)
        start_session = routine(flow, "game_start_session", "; end game_start_session")
        self.assertIn("call game_new_session_init", start_session)
        session = routine(
            read_asm("state_game_init.inc"),
            "game_new_session_init",
            "; end game_new_session_init",
        )
        restart_event = session.index("ld a,STATE_NEW_EVENT_START")
        new_game = session.index("ld a,STATE_NEW_NEW_GAME")
        self.assertLess(restart_event, new_game)

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

        sine_sources = (
            ("runtime", read_asm("fixed168.inc")),
            ("standalone", (PROJECT_ROOT / "fixed168/fixed168.inc").read_text()),
        )
        for name, source in sine_sources:
            with self.subTest(source=name):
                sine = routine(source, "sin168", "polar_to_cartesian:")
                self.assertNotIn("call hlu_abs", sine)
                self.assertIn(
                    "mlt hl\n"
                    "    ld de,0 ; MLT leaves HLU unchanged, so copy only the 16-bit product\n"
                    "    ld d,h\n"
                    "    ld e,l\n"
                    "    ld hl,sin_lut_168",
                    sine,
                )

    def test_tile_activation_table_covers_every_accepted_id(self) -> None:
        self.assertIn('include "active_tile_registry.inc"', read_asm("tiles_active.inc"))
        source = read_asm("active_tile_registry.inc")
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

        self.assertEqual(plotter.count("call tiles_scroll_background"), 1)
        self.assertLess(plotter.index("call tiles_scroll_background"),
                        plotter.index("jp nz,@drain"))
        self.assertLess(plotter.index("jp nz,@drain"), plotter.index("ld hl,(tiles_row_defs)"))
        begin_drain = plotter[plotter.index("@begin_drain:"):plotter.index("@drain:")]
        self.assertIn("ld hl,field_bottom+1", begin_drain)
        self.assertNotIn("(tiles_level_complete)", begin_drain)
        drain = plotter[plotter.index("@drain:"):plotter.index("@level_complete:")]
        self.assertIn("dec hl\n    ld (tiles_drain_remaining),hl", drain)
        self.assertIn("jp z,@level_complete", drain)
        self.assertNotIn("(tiles_row_defs)", drain)
        self.assertIn("tile_bottom: equ field_bottom+1", read_asm("screen.inc"))
        self.assertIn("ld (tiles_drain_remaining),hl", read_asm("levels.inc"))
        self.assertIn("call vdu_scroll_down", scroll)
        self.assertIn("call bg_plot", scroll)

    def test_bar_maxima_are_zero_extended_into_24_bit_values(self) -> None:
        cases = (
            ("player_shields.inc", "init_bar_shields", "player_shields_max"),
            ("player_laser.inc", "init_bar_laser", "laser_power_max"),
            ("player_fuel.inc", "init_bar_fuel", "player_fuel_max"),
        )
        for filename, label, maximum in cases:
            with self.subTest(bar=label):
                init = routine(read_asm(filename), label, f"; end {label}")
                self.assertNotIn(f"ld hl,({maximum})", init)
                self.assertIn(
                    "ld hl,0\n"
                    f"    ld a,({maximum})\n"
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

        initialization = routine(
            read_asm("runtime_init.inc"),
            "runtime_process_init",
            "; end runtime_process_init",
        )
        request_clear = initialization.index("ld (game_exit_requested),a")
        state_start = initialization.index("ld (game_state),hl")

        main = routine(read_asm("nurples.asm"), "main", "; end main")
        self.assertIn("call runtime_process_init", main)
        dispatch = main.index("call do_game")
        request_read = main.index("ld a,(game_exit_requested)", dispatch)
        vblank = main.index("call vdu_vblank")
        self.assertLess(request_clear, state_start)
        self.assertLess(dispatch, request_read)
        self.assertLess(request_read, vblank)
        self.assertIn("or a\n    jp nz,@state_exit", main[request_read:vblank])
        state_exit = main.index("@state_exit:", vblank)
        telemetry = main.index("call telemetry_publish_terminal", state_exit)
        main_end = main.index("jp main_end", telemetry)
        self.assertLess(state_exit, telemetry)
        self.assertLess(telemetry, main_end)

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
        self.assertIn("ld hl,game_over\n    ld (game_state),hl\n    scf\n    ret", playing)
        self.assertIn("jp nz,game_route_terminal_player_event", playing)

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
        self.assertLess(playing.index("call game_update_live_world"),
                        playing.index("ld a,STATE_NEW_EVENT_LEVEL_COMPLETE"))
        self.assertIn("call seeker_group_enter_retreat", transition)
        self.assertIn("jp game_update_live_world", transition)
        self.assertNotIn("player_victory", transition)
        self.assertNotIn("(player_state)", transition)
        self.assertNotIn("seeker_group_enter_seek", transition)

    def test_warp_runs_the_ordinary_ship_laser_and_enemy_collision_paths(self) -> None:
        playing = read_asm("state_game_playing.inc")
        world = routine(playing, "game_update_live_world", "; end game_update_live_world")
        self.assertIn("call do_player", world)
        self.assertIn("call do_enemies", world)
        pilot = routine(read_asm("player_state.inc"), "player_alive", "; end player_alive")
        self.assertIn("call player_input\n    call player_move_weapons", pilot)
        fireball = read_asm("enemy_fireball.inc")
        movement = routine(fireball, "move_enemy_fireball", "; end move_enemy_fireball")
        self.assertLess(movement.index("call move_sprite"),
                        movement.index("call check_enemy_collision_player"))
        seeker = read_asm("enemy_seeker.inc")
        collisions = routine(seeker, "move_enemy_seeker", "; end move_enemy_seeker")
        self.assertIn("call check_enemy_collision_laser", collisions)

    def test_new_session_values_precede_level_display(self) -> None:
        source = read_asm("state_game_init.inc")
        game_init = routine(
            source,
            "game_new_session_init",
            "; end game_new_session_init",
        )
        score_reset = game_init.index("ld (player_score),hl")
        starting_score_reset = game_init.index("ld (player_starting_score),hl")
        level_init = game_init.index("call init_tileset")
        self.assertLess(score_reset, level_init)
        self.assertLess(starting_score_reset, level_init)
        self.assertIn("call reset_runtime_tables", game_init)
        playfield = routine(
            source,
            "game_prepare_playfield",
            "; end game_prepare_playfield",
        )
        background = playfield.index("ld hl,BUF_BG_STARS_0")
        publish_background = playfield.index("ld (bg_bufferId),hl", background)
        plot_background = playfield.index("call init_bg", publish_background)
        self.assertLess(background, publish_background)
        self.assertLess(publish_background, plot_background)
        self.assertNotIn("load_game_images", playfield)
        self.assertNotIn("vdu_clear_all_buffers", playfield)

        player_dead = routine(
            read_asm("player_state.inc"),
            "player_dead",
            "; end player_dead",
        )
        self.assertNotIn("call reset_runtime_tables", player_dead)
        self.assertNotIn("call init_level", player_dead)
        self.assertNotIn("player_starting_score", player_dead)
        self.assertNotIn("ld (player_score),hl", player_dead)
        self.assertNotIn("ld (player_state)", game_init)
        self.assertNotIn("ld (player_state)", player_dead)

    def test_player_init_owns_gameplay_timer_resets(self) -> None:
        player_init = routine(
            read_asm("player_state.inc"),
            "player_init",
            "; end player_init",
        )
        self.assertIn(
            "ld a,(charge_laser_timer_max)\n"
            "    ld (charge_laser_timer),a",
            player_init,
        )
        self.assertIn(
            "ld a,(discharge_fuel_timer_max)\n"
            "    ld (discharge_fuel_timer),a",
            player_init,
        )
        self.assertIn(
            "xor a\n"
            "    ld (laser_power_last),a\n"
            "    ld (player_shields_last),a\n"
            "    ld (player_fuel_last),a\n"
            "; A live player must not expose stale diagnostics from the previous death.\n"
            "    ld (player_dying_phase),a\n"
            "    ld (player_dying_timer),a\n"
            "    ld (player_dying_spawns),a",
            player_init,
        )
        self.assertNotIn(
            "discharge_fuel_timer",
            routine(
                read_asm("player_fuel.inc"),
                "init_bar_fuel",
                "; end init_bar_fuel",
            ),
        )

    def test_player_death_entrance_and_grace_are_actor_owned(self) -> None:
        player = read_asm("player_state.inc")
        for field in (
            "player_death_base_frame",
            "player_death_turn_timer",
            "player_death_wiggle_timer",
            "player_entry_angle",
            "player_entry_timer",
            "player_grace_timer",
            "player_entry_warp_enabled",
            "player_entry_amplitude",
        ):
            self.assertIn(f"{field}:", player)
        self.assertIn("player_play_ceiling: equ 64", player)
        self.assertIn("player_entry_start_y: equ 320", player)
        self.assertIn("player_grace_frames: equ 60", player)
        self.assertIn("player_entry_total_frames: equ 112", player)
        self.assertNotIn("[sprite_bottom*256", player)
        self.assertIn("player_entry_max_amplitude: equ 64", player)
        self.assertIn("player_entry_y_velocity: equ -219", player)
        self.assertIn("player_entry_start_y_offset: equ 96", player)

        setup = routine(player, "player_init", "; end player_init")
        self.assertLess(setup.index("call player_entry_load_start_coords"),
                        setup.index("ld hl,player_defaults"))
        self.assertLess(setup.index("ld hl,player_defaults"),
                        setup.index("call spawn_sprite_from_coords"))
        coords = routine(player, "player_entry_load_start_coords", "; end player_entry_load_start_coords")
        self.assertIn("ld a,(player_spawn_resolved_x)\n    ld bc,0\n    ld b,a", coords)
        self.assertIn("ld hl,player_entry_start_y_offset*256\n    add hl,de\n    ex de,hl", coords)
        self.assertNotIn("add a,player_entry_start_y_offset", coords)

        screen = read_asm("screen.inc")
        self.assertIn(
            ".if [player_entry_target_y+player_entry_start_y_offset]-sprite_bottom",
            screen,
        )

        death_entry = routine(player, "kill_player", "; end kill_player")
        save_pose = death_entry.index("ld (ix+player_death_base_frame),a")
        publish_dying = death_entry.index("ld hl,player_dying")
        self.assertLess(save_pose, publish_dying)
        self.assertIn("ld hl,player_death_forward_speed", death_entry)

        dying = routine(player, "player_dying", "; end player_dying")
        initial = dying.index("@initial:")
        first_effect = dying.index("call spawn_explosion_player", initial)
        self.assertLess(dying.index("call player_dying_move", initial), first_effect)

        spawning = routine(player, "player_spawning", "; end player_spawning")
        self.assertIn("call move_sprite", spawning)
        self.assertIn("call player_entry_damped_swoop", spawning)
        self.assertIn("ld hl,player_grace", spawning)
        self.assertIn("ld h,(iy+player_entry_target_x_actor)", spawning)
        self.assertIn("ld h,(iy+player_entry_target_y_actor)", spawning)
        self.assertNotIn("call level_transition_step", spawning)
        self.assertIn("call player_draw_record\n    ld iy,player_begin\n    dec (iy+player_entry_timer)", spawning)

        level_entry = routine(
            player,
            "player_init_level_entry",
            "; end player_init_level_entry",
        )
        respawn_entry = routine(
            player,
            "player_init_respawn",
            "; end player_init_respawn",
        )
        self.assertIn("ld a,1\n    jp player_init", level_entry)
        self.assertIn("xor a\n    jp player_init", respawn_entry)

        starfield_speed = routine(
            player,
            "player_entry_starfield_speed",
            "; end player_entry_starfield_speed",
        )
        self.assertIn("ld de,player_spawning", starfield_speed)
        self.assertIn("ld a,(iy+player_entry_warp_enabled)", starfield_speed)
        self.assertLess(starfield_speed.index("jp z,@inactive"), starfield_speed.index("call level_transition_step"))
        self.assertIn("ld (ix+player_entry_warp_enabled),a\n    or a\n    call nz,level_transition_init_entrance", setup)

        scripted_draw = routine(
            player,
            "player_draw_record",
            "; end player_draw_record",
        )
        move = scripted_draw.index("call vdu_sprite_move_abs168")
        frame = scripted_draw.index("call vdu_sprite_select_frame")
        publish = scripted_draw.index("call vdu_sprite_update")
        self.assertLess(move, frame)
        self.assertLess(frame, publish)

        grace = routine(player, "player_grace", "; end player_grace")
        input_tick = grace.index("call player_input")
        weapon_tick = grace.index("call player_move_weapons")
        collision_on = grace.index("set sprite_alive,(iy+sprite_collisions)")
        publish_alive = grace.index("ld hl,player_alive")
        seek_resume = grace.index("call nz,seeker_group_enter_seek")
        self.assertLess(input_tick, collision_on)
        self.assertLess(input_tick, weapon_tick)
        self.assertLess(weapon_tick, collision_on)
        self.assertLess(collision_on, publish_alive)
        self.assertLess(publish_alive, seek_resume)

        player_input = routine(
            read_asm("player_input.inc"),
            "player_input",
            "; end player_input",
        )
        self.assertIn("ld (iy+sprite_frame),a", player_input)
        self.assertIn("call player_enforce_play_ceiling", player_input)

    def test_world_travel_runs_for_player_alive_and_grace(self) -> None:
        player = routine(
            read_asm("player_state.inc"),
            "player_world_travel_enabled",
            "; end player_world_travel_enabled",
        )
        self.assertIn("ld de,player_alive", player)
        self.assertIn("ld de,player_grace", player)
        self.assertLess(player.index("ld de,player_alive"), player.index("@travel:"))
        self.assertLess(player.index("ld de,player_grace"), player.index("@travel:"))

        plotter = routine(read_asm("tiles.inc"), "tiles_plot", "; end tiles_plot")
        gate = plotter.index("call player_world_travel_enabled")
        scroll = plotter.index("call tiles_scroll_background")
        self.assertLess(gate, scroll)
        self.assertIn("jp c,@travel\n    xor a\n    ret", plotter[:scroll])

        mover = routine(
            read_asm("tiles_active.inc"),
            "move_active_tile",
            "; end move_active_tile",
        )
        gate = mover.index("call player_world_travel_enabled")
        y_write = mover.index("ld (iy+tile_y),hl")
        self.assertLess(gate, y_write)
        self.assertIn("ld a,1\n    or a\n    ret", mover[:y_write])

    def test_respawn_retains_score_and_level_progress(self) -> None:
        player_dead = routine(
            read_asm("player_state.inc"),
            "player_dead",
            "; end player_dead",
        )
        for forbidden in (
            "player_starting_score",
            "ld (player_score),hl",
            "call reset_runtime_tables",
            "call init_level",
        ):
            self.assertNotIn(forbidden, player_dead)
        self.assertIn("call player_init_respawn", player_dead)
        self.assertNotIn("call player_init_level_entry", player_dead)
        self.assertNotIn(
            "charge_laser_timer",
            routine(
                read_asm("player_laser.inc"),
                "init_bar_laser",
                "; end init_bar_laser",
            ),
        )

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
            "air_defaults",
            "fireball_defaults",
            "explosion_defaults",
            "pad_seeker_defaults",
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

    def test_actor_record_schema_and_lifetime_boundaries_are_explicit(self) -> None:
        contract = read_asm("actor_record.inc")
        self.assertIn("actor_record_size:             equ 64", contract)
        self.assertIn("actor_record_shared_end:       equ 41", contract)
        self.assertIn("actor_record_generation:       equ 41", contract)
        self.assertIn("actor_record_custom:           equ 42", contract)
        self.assertIn(
            "actor_record_custom_size:      equ actor_record_size-actor_record_custom",
            contract,
        )
        begin = routine(
            contract,
            "actor_record_begin_lifetime_ix",
            "; end actor_record_begin_lifetime_ix",
        )
        self.assertIn("inc a\n    jp nz,@generation_ready", begin)
        self.assertIn("ld (ix+actor_record_generation),a", begin)
        self.assertIn("lea hl,ix+actor_record_custom", begin)
        self.assertIn("ld bc,actor_record_custom_size", begin)
        self.assertIn("call clear_mem", begin)

        schemas = {
            "player_state.inc": ("player_custom_begin", "player_custom_end"),
            "player_laser.inc": (
                "player_laser_custom_begin",
                "player_laser_custom_end",
            ),
            "enemy_seeker.inc": ("seeker_custom_begin", "seeker_custom_end"),
            "enemy_fireball.inc": (
                "fireball_custom_begin",
                "fireball_custom_end",
            ),
            "explosion.inc": ("explosion_custom_begin", "explosion_custom_end"),
            "tile_pad_seeker.inc": (
                "pad_seeker_custom_begin",
                "pad_seeker_custom_end",
            ),
            "tile_turret_fireball.inc": (
                "turret_fireball_custom_begin",
                "turret_fireball_custom_end",
            ),
            "tile_electrode.inc": (
                "electrode_custom_begin",
                "electrode_custom_end",
            ),
            "tile_lightning.inc": (
                "lightning_custom_begin",
                "lightning_custom_end",
            ),
            "tile_crater.inc": ("crater_custom_begin", "crater_custom_end"),
        }
        for name, (begin_symbol, end_symbol) in schemas.items():
            with self.subTest(actor=name):
                source = read_asm(name)
                self.assertIn(f"{begin_symbol}: equ actor_record_custom", source)
                self.assertRegex(source, rf"(?m)^{end_symbol}: equ .+$")
                self.assertIn(f".if {end_symbol}-{begin_symbol}", source)

        constructors = (
            read_asm("sprites.inc")
            + read_asm("tile_table.inc")
            + read_asm("tiles_active.inc")
        )
        self.assertEqual(
            constructors.count("call actor_record_begin_lifetime_ix"),
            6,
        )

    def test_actor_spawn_sources_and_sprite_pools_fail_before_publication(self) -> None:
        sprites = read_asm("sprites.inc")
        pool = routine(sprites, "sprite_pool_valid", "; end sprite_pool_valid")
        for base, count in (
            ("player_lasers_begin", "player_lasers_max"),
            ("player_begin", "1"),
            ("seekers_base", "seekers_max"),
            ("fireballs_base", "fireballs_max"),
            ("explosions_base", "explosions_max"),
        ):
            with self.subTest(pool=base):
                self.assertIn(f"ld de,{base}", pool)
                self.assertIn(f"cp {count}", pool)

        sprite_from_sprite = routine(
            sprites,
            "spawn_sprite_from_sprite",
            "; end spawn_sprite_from_sprite",
        )
        self.assertLess(
            sprite_from_sprite.index("call sprite_table_record_pointer_valid"),
            sprite_from_sprite.index("call table_get_next_id"),
        )

        tile_table = read_asm("tile_table.inc")
        tile_from_sprite = routine(
            tile_table,
            "spawn_active_tile_from_sprite",
            "; end spawn_active_tile_from_sprite",
        )
        self.assertLess(
            tile_from_sprite.index("call sprite_table_record_pointer_valid"),
            tile_from_sprite.index("call tile_table_get_next_id"),
        )

        active = read_asm("tiles_active.inc")
        sprite_from_tile = routine(
            active,
            "spawn_sprite_from_tile",
            "; end spawn_sprite_from_tile",
        )
        self.assertLess(
            sprite_from_tile.index("call tile_table_record_pointer_valid"),
            sprite_from_tile.index("call table_get_next_id"),
        )
        transform = routine(
            active,
            "spawn_active_tile_from_tile",
            "; end spawn_active_tile_from_tile",
        )
        self.assertLess(
            transform.index("call tile_table_record_pointer_valid"),
            transform.index("call actor_record_begin_lifetime_ix"),
        )
        self.assertLess(
            transform.index("call actor_record_begin_lifetime_ix"),
            transform.index("ldir"),
        )

    def test_seeker_behavior_transition_initializes_before_publication(self) -> None:
        seeker = read_asm("enemy_seeker.inc")
        transition = routine(
            seeker,
            "seeker_enter_orbit",
            "; end seeker_enter_orbit",
        )
        publish = transition.index("ld (iy+sprite_move_program),hl")
        for initialized_field in (
            "ld (iy+seeker_orbit_origin_x),a",
            "ld (iy+seeker_orbit_angle),a",
            "ld (iy+seeker_state),a",
        ):
            self.assertLess(transition.index(initialized_field), publish)
        self.assertIn("ld hl,move_enemy_seeker_orbit_stage1", transition)
        resume = routine(
            seeker,
            "seeker_enter_seek",
            "; end seeker_enter_seek",
        )
        publish = resume.index("ld (iy+sprite_move_program),hl")
        for initialized_field in (
            "ld (iy+sprite_move_step),a",
            "ld (iy+seeker_state),a",
            "ld (iy+sprite_move_timer),a",
        ):
            self.assertLess(resume.index(initialized_field), publish)
        self.assertIn("ld hl,en_nav_seek_zag", resume)
        self.assertNotIn(
            "change_sprite_group_move_program",
            read_asm("player_state.inc") + read_asm("sprites.inc"),
        )
        self.assertIn("call seeker_group_enter_death_retreat", read_asm("player_state.inc"))
        self.assertIn("call nz,seeker_group_enter_seek", read_asm("player_state.inc"))

    def test_confirmed_actor_visual_defects_are_repaired_at_source(self) -> None:
        explosion = routine(
            read_asm("explosion.inc"),
            "move_explosion",
            "; end move_explosion",
        )
        self.assertNotIn("call move_sprite", explosion)
        self.assertNotIn("neg_hlu", explosion)
        for axis in ("x", "y"):
            self.assertIn(
                f"ld hl,(iy+sprite_{axis})\n    ld de,(iy+sprite_{axis}vel)\n"
                f"    add hl,de\n    ld (iy+sprite_{axis}),hl", explosion,
            )
        reject = "call sprite_footprint_within_playfield\n    jp nc,table_deactivate_sprite"
        self.assertLess(explosion.index("ld (iy+sprite_y),hl"), explosion.index(reject))
        self.assertLess(explosion.index(reject), explosion.index("dec (iy+sprite_animation_timer)"))

        # First-show and post-callback publication both go through this gate.
        guard = routine(read_asm("sprites.inc"), "sprite_move_publication_allowed",
                        "; end sprite_move_publication_allowed")
        self.assertIn("cp explosion\n    jp nz,@allowed", guard)
        self.assertIn("jp z,@hidden\n    call sprite_footprint_within_playfield", guard)
        self.assertIn("seeker_within_playfield:\nsprite_footprint_within_playfield:",
                      read_asm("seeker_retreat.inc"))
        defaults = block(read_asm("explosion.inc"), "explosion_defaults", "spawn_explosion")
        counts = [int(n) for n in re.findall(r"^\s*db (\d+)\s*; rendered", defaults, re.M)]
        self.assertEqual(counts, [16, 16])
        for frame in "abcde":
            png = PROJECT_ROOT / f"assets/img/proc/sprites/explosion_{frame}.png"
            self.assertEqual(struct.unpack(">II", png.read_bytes()[16:24]), tuple(counts))

        fireball = routine(
            read_asm("enemy_fireball.inc"),
            "enemy_spawn_fireball",
            "; end enemy_spawn_fireball",
        )
        heading_write = fireball.index("ld (ix+sprite_heading),hl")
        initial_frame = fireball.index("call sprite_frame_heading_animation")
        self.assertLess(heading_write, initial_frame)
        self.assertIn("ld hl,(Heading_p)", fireball[:heading_write])
        self.assertNotIn("ld hl,(Bearing_t)", fireball[:heading_write])
        self.assertIn("call vdu_sprite_select_frame", fireball[initial_frame:])

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
            "tile_pad_seeker.inc",
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
            ("tile_pad_seeker", "move_pad_seeker"),
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

    def test_lethal_contact_waits_until_every_tile_has_moved(self) -> None:
        damage = routine(read_asm("player_shields.inc"), "update_shields", "; end update_shields")
        self.assertNotIn("kill_player", damage)
        self.assertIn("ld hl,player_death_pending\n    ld (hl),1", damage)
        contact = routine(read_asm("collisions.inc"), "check_enemy_collision_player",
                          "; end check_enemy_collision_player")
        self.assertLess(contact.index("ret nz"), contact.index("call check_collision"))
        world = routine(read_asm("state_game_playing.inc"), "game_update_live_world",
                        "; end game_update_live_world")
        steps = ["call do_enemies", "call move_tiles", "ld a,(player_death_pending)",
                 "jp z,@contacts_done", "ld (player_death_pending),a", "call kill_player"]
        positions = [world.index(step) for step in steps]
        self.assertEqual(positions, sorted(positions))

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
        for handler in (
            "player_alive",
            "player_spawning",
            "player_grace",
            "player_dying",
            "player_dead",
        ):
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
            "move_enemy_seeker_orbit_stage1",
            "move_enemy_fireball",
            "move_explosion",
        ):
            self.assertIn(f"ld de,{handler}", whitelist)
        self.assertIn("cp seeker_state_seek", whitelist)
        self.assertIn("cp seeker_state_orbit", whitelist)
        self.assertNotIn("ld de,move_enemy_seeker_orbit\n", whitelist)

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
            "    jr nc,@load_failed",
            fonts,
        )
        self.assertIn("@load_failed:\n; Balance the loop counter", read_asm("fonts.inc"))

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
        timestamp = routine(timer, "timestamp_init", "; end timestamp_init")
        self.assertIn("MOSCALL mos_sysvars", timestamp)
        self.assertIn("ld hl,(ix+sysvar_time)", timestamp)
        self.assertIn("ld (timestamp_now),hl", timestamp)
        self.assertIn("ld (timestamp_old),hl", timestamp)
        self.assertIn("ld hl,0\n    ld (timestamp_chg),hl", timestamp)

        delay = block(
            timer,
            "multiPurposeDelay",
            "timer_fault_delay_timeout",
        )
        phase_capture = delay.index("ld (oldTimeStamp),a")
        wait_loop = delay.index("waitLoop:")
        self.assertLess(phase_capture, wait_loop)
        self.assertIn("ld a,32", delay)
        self.assertIn("ld b,0\n    ld d,0", delay)
        self.assertIn("dec d\n    jp nz,waitLoop\n    djnz waitLoop", delay)
        self.assertIn("ld (timer_fault_flags),a", delay)
        self.assertIn("push de", delay)
        self.assertIn("pop de", delay)


if __name__ == "__main__":
    unittest.main()
