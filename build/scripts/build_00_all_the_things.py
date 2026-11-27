"""Explicit orchestrator for Purple Nurples' current deployable application."""

import argparse
from pathlib import Path
import shutil
import subprocess
from tempfile import NamedTemporaryFile, TemporaryDirectory

from nurples_tools.announcer import audio_container
from nurples_tools.active_tiles import registry
from nurples_tools.campaign_audio import prepare_audio, verify_audio
from nurples_tools.loading_music import configure as configure_loading_music
from nurples_tools.current_assets import check_current_assets, write_current_assets
from nurples_tools.fonts import build_fonts, check_fonts
from nurples_tools.lode_font import check_lode_font, write_lode_font
from nurples_tools.maps import check_map_include, write_map_include


PROJECT_ROOT = Path(__file__).resolve().parents[2]
ASM_DIR = PROJECT_ROOT / "src/asm"
TARGET_DIR = PROJECT_ROOT / "tgt"
FONT_SOURCE = PROJECT_ROOT / "assets/fonts"
FONT_TARGET = TARGET_DIR / "fonts"
FONT_MANIFEST = ASM_DIR / "fonts_list.inc"
CAMPAIGN_MANIFEST = PROJECT_ROOT / "maps/campaign.toml"


def build_assets() -> None:
    registry(PROJECT_ROOT, write=True)
    bank = audio_container(PROJECT_ROOT / "assets/announcer")
    TARGET_DIR.mkdir(parents=True, exist_ok=True)
    (TARGET_DIR / "voice.agnb").write_bytes(bank)
    for output in write_lode_font(PROJECT_ROOT):
        print(f"Generated {output.relative_to(PROJECT_ROOT)}")
    build_fonts(FONT_SOURCE, FONT_TARGET, FONT_MANIFEST)
    for output in write_current_assets(PROJECT_ROOT):
        print(f"Generated {output.relative_to(PROJECT_ROOT)}")
    campaign_output = write_map_include(CAMPAIGN_MANIFEST)
    print(f"Generated {campaign_output.relative_to(PROJECT_ROOT)}")


def check_assets() -> None:
    registry(PROJECT_ROOT)
    differences = check_lode_font(PROJECT_ROOT)
    voice = TARGET_DIR / "voice.agnb"
    if not voice.is_file() or voice.read_bytes() != audio_container(PROJECT_ROOT / "assets/announcer"):
        differences.append("tgt/voice.agnb differs from the approved announcer WAVs")
    differences.extend(check_fonts(FONT_SOURCE, FONT_TARGET, FONT_MANIFEST))
    differences.extend(check_current_assets(PROJECT_ROOT))
    differences.extend(check_map_include(CAMPAIGN_MANIFEST))
    if differences:
        raise RuntimeError("\n".join(differences))
    print("Current deployable assets are deterministic and up to date.")


def assemble(*, scanout_fixture=False, earth_jit=False, pace=None, staggered=False, spawn_review=False,
             test_game=False, batch=0, auto_pair=False, frame_trace=False, pose=None, pose_cost=False) -> None:
    registry(PROJECT_ROOT)
    assembler = shutil.which("ez80asm") or str(
        Path.home() / ".local/bin/ez80asm"
    )
    output = PROJECT_ROOT / ".build/scanout-r01" if scanout_fixture else TARGET_DIR
    if earth_jit:
        output = PROJECT_ROOT / ".build/earth-jit-r01"
    if pace:
        output = PROJECT_ROOT / ".build" / f"aligned-pace-{pace}"
    if staggered:
        output = PROJECT_ROOT / ".build/staggered-pace-control"
    if spawn_review:
        output = PROJECT_ROOT / ".build/map005-review"
    if test_game:
        output = PROJECT_ROOT / ".build/test-game"
    if batch:
        output = PROJECT_ROOT / (".build/earth-jit-256" if batch == 256 else ".build/earth-jit-1k")
    if auto_pair:
        output = output / "auto-pair-r02"
    if frame_trace:
        output = output / "frame-trace"
    if pose:
        output = PROJECT_ROOT / (".build/earth-pose-cost-r02" if pose_cost else ".build/earth-pose-r02") / pose
    output.mkdir(parents=True, exist_ok=True)
    with TemporaryDirectory(prefix="nurples-assemble-") as temp_dir:
        staged_asm = Path(temp_dir) / "asm"
        shutil.copytree(ASM_DIR, staged_asm)
        if auto_pair:
            # The accepted timing baseline predates splash-buffer cleanup.
            pin = "af46123ca1c8ceb54acf0ab263d522f402aa9fa5"
            for name in ("images.inc", "presentation_flow_states.inc"):
                (staged_asm / name).write_bytes(subprocess.check_output(
                    ["git", "show", f"{pin}:src/asm/{name}"], cwd=PROJECT_ROOT))
        if batch == 256:
            from nurples_tools.earth_jit import stage_alignment_reader
            stage_alignment_reader(staged_asm, PROJECT_ROOT.parent / "agon-utils")
        if test_game:
            from nurples_tools.loading_music import edit
            edit(staged_asm, "game_pause.inc", "\ngame_pause_poll:\n", "\nordinary_pause_poll:\n")
            with (staged_asm/"game_pause.inc").open("a") as stream:
                stream.write('\n    include "test_death_key.inc"\n')
        if spawn_review:
            from nurples_tools.maps import compile_map_manifest, overlap_audition
            campaign = compile_map_manifest(CAMPAIGN_MANIFEST)
            (staged_asm/campaign.output_path.name).write_text(overlap_audition(campaign))
        modules = prepare_audio(PROJECT_ROOT, staged_asm)
        configure_loading_music(staged_asm, candidate=True, small_reads=True, whole=True,
                                all_phases=True, combat_cues=True)
        if scanout_fixture or earth_jit:
            from nurples_tools.scanout_fixture import instrument
            instrument(staged_asm)
            if batch and not auto_pair:
                # Carry the ordinary D-key test game's collision damage, not
                # the older invulnerable scanout fixture's override.
                shutil.copy2(ASM_DIR / "player_shields.inc", staged_asm / "player_shields.inc")
        if earth_jit:
            from nurples_tools.earth_jit import configure
            configure(staged_asm, TARGET_DIR, output, 4 if batch == 1024 else 1, batch == 256)
        if pace:
            from nurples_tools.loading_music import edit
            if staggered:
                from nurples_tools.maps import compile_map_manifest
                level_dir = PROJECT_ROOT / "level-design/levels/01"
                maps = compile_map_manifest(CAMPAIGN_MANIFEST, earth_source=level_dir/"staggered.tmx")
                if maps.levels[0].runtime_rows != compile_map_manifest(CAMPAIGN_MANIFEST).levels[0].runtime_rows:
                    raise ValueError("Staggered audition expects unchanged city art")
                (staged_asm/maps.output_path.name).write_text(maps.text)
                edit(staged_asm, "earth_march.inc", "equ 5*16", "equ 6*16")
                path = staged_asm/"enemy_side_air.inc"
                source = path.read_text()
                start = source.index("    ; Admission sees tiles_row")
                end = source.index("@wait:\n", start)
                edit(staged_asm, path.name, source[start:end], """    ld a,(tiles_row)
@wave_row:
    cp 48
    jr c,@rank
    sub 48
    jr @wave_row
@rank:
    sub 38 ; rear rank (Tiled Y+1 modulo 48)
    cp 6
    jr c,@valid
    xor a
@valid:
    or a
    jr z,@wait
    rlca
    rlca
    rlca
    rlca
    ld hl,256
""")
            edit(staged_asm, "nurples.asm", "main_loop:\n",
                 "    call pace_init\nmain_loop:\n    call pace_classify\n")
            edit(staged_asm, "nurples.asm", "    call vdu_vblank\n",
                 "    call vdu_vblank\n    call pace_sample\n")
            edit(staged_asm, "nurples.asm", "    call jit_report",
                 "    call jit_report\n    call pace_report")
            edit(staged_asm, "nurples.asm", '    include "earth_jit.inc"',
                 '    include "earth_jit.inc"\n    include "frame_pace_fixture.inc"')
            if batch == 256:
                edit(staged_asm, "nurples.asm", "    call timestamp_tick",
                     "    call claim_count_start\n    call timestamp_tick")
                edit(staged_asm, "levels.inc", "init_level:\n", "init_level:\n"
                     "    ld a,1\n    ld (claim_reset),a ; level setup is not pending frame credit\n")
                edit(staged_asm, "frame_pace_fixture.inc", "pace_sample:\n    call pace_read_clock",
                     "pace_sample:\n    ld hl,(claim_epoch) ; the exact tick admitted by the scheduler")
                edit(staged_asm, "nurples.asm", "    call pace_init", "    call wait_clock_init\n    call pace_init")
                edit(staged_asm, "nurples.asm", "    call vdu_vblank\n    call pace_sample",
                     "    call wait_clock_frame\n    call pace_sample")
                edit(staged_asm, "nurples.asm", "    call pace_report", "    call pace_report\n    call wait_clock_report")
                edit(staged_asm, "nurples.asm", '    include "frame_pace_fixture.inc"',
                     '    include "frame_pace_fixture.inc"\n    include "vblank_wait_clock.inc"')
            if pace == "control":
                # Replace the four-byte ADL call without moving other code.
                edit(staged_asm, "enemy_side_air.inc", "    call air_try_bomb",
                     "    db 0,0,0,0 ; control: no air bombs")
        if auto_pair:
            from nurples_tools.earth_jit import configure_auto_pair
            configure_auto_pair(staged_asm)
        if frame_trace:
            from hashlib import sha256
            from nurples_tools.earth_jit import configure_frame_trace
            # Only instrument the matched R02 base (same as R01 minus the
            # accepted playfield clear); R01 hardware evidence stays immutable.
            subprocess.run([assembler, "nurples.asm", "tracebase.bin"], cwd=staged_asm, check=True)
            if sha256((staged_asm / "tracebase.bin").read_bytes()).hexdigest() != (
                "8ba499a5129133b638086c76dbf27d39f23a4bd3d604f6160b7e595233e40efe"
            ):
                raise RuntimeError("Pre-trace executable differs from the R02 fixture baseline")
            configure_frame_trace(staged_asm)
        if pose:
            from nurples_tools.earth_pose import configure_pose
            configure_pose(staged_asm, pose, PROJECT_ROOT, output)
            if pose_cost:
                from nurples_tools.pose_cost import configure_cost
                configure_cost(staged_asm, pose)
        staged_binary = staged_asm / "nurples.bin"
        staged_symbols = staged_asm / "nurples.symbols"
        staged_binary.unlink(missing_ok=True)
        staged_symbols.unlink(missing_ok=True)
        subprocess.run(
            [assembler, "-l", "-s", "nurples.asm", staged_binary.name],
            cwd=staged_asm,
            check=True,
        )
        if not staged_binary.is_file() or not staged_symbols.is_file():
            raise RuntimeError("assembly did not produce a binary/symbol pair")
        if pose == "native":
            linked = {name: int(value, 16) for line in staged_symbols.read_text().splitlines()
                      if " $" in line for name, value in (line.split(" $", 1),)}
            if (linked["fireballs_end_sprite_id"], linked["air_end_sprite_id"]) != (12, 44):
                raise ValueError("native module does not match linked air-slot bounds")
        verify_audio(staged_binary.read_bytes(), modules)
        for staged, destination in (
            (staged_binary, output / "nurples.bin"),
            (staged_symbols, output / "nurples.symbols"),
        ):
            with NamedTemporaryFile(
                dir=output,
                prefix=f".{destination.name}.",
                suffix=".tmp",
                delete=False,
            ) as temporary_output:
                publish_path = Path(temporary_output.name)
            try:
                shutil.copy2(staged, publish_path)
                publish_path.replace(destination)
            finally:
                publish_path.unlink(missing_ok=True)
    print(f"Generated nurples.bin and nurples.symbols in {output}")


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Build current Purple Nurples assets and/or assemble without "
            "deploying or launching an emulator."
        )
    )
    actions = parser.add_mutually_exclusive_group(required=True)
    actions.add_argument("--assets", action="store_true", help="build assets")
    actions.add_argument(
        "--check",
        action="store_true",
        help="verify generated assets without writing",
    )
    actions.add_argument(
        "--assemble",
        action="store_true",
        help="assemble the existing generated inputs only",
    )
    actions.add_argument(
        "--all",
        action="store_true",
        help="build assets, verify them, and assemble",
    )
    actions.add_argument("--scanout-fixture", action="store_true",
                         help="assemble the diagnostic game outside tgt/")
    actions.add_argument("--earth-jit", action="store_true",
                         help="build single-row Earth audition in .build/earth-jit-r01")
    actions.add_argument("--aligned-pace", choices=("control", "bombing"))
    actions.add_argument("--earth-jit-1k", action="store_true", help="Eirene RC3 async Earth audition")
    actions.add_argument("--earth-jit-256", action="store_true", help="one async Earth row per scroll")
    actions.add_argument("--earth-auto-pair", action="store_true", help="unattended Earth alignment comparison")
    actions.add_argument("--earth-frame-trace", action="store_true", help="Earth A/B with one timing row per update")
    from nurples_tools.earth_pose import POSE_ARMS
    actions.add_argument("--earth-pose", choices=POSE_ARMS, help="matched full-Earth pose transport fixture")
    actions.add_argument("--earth-pose-cost", choices=("direct", "sync", "golem"),
                         help="separate sparse CPU-side pose and MOS-call timing observer")
    actions.add_argument("--test-game", action="store_true", help="ordinary gameplay with D-to-die")
    actions.add_argument("--staggered-pace", action="store_true")
    actions.add_argument("--sprite-spawn-review", action="store_true",
                         help="MAP-005 campaign audition: draw toggles and one DG conduit under a ship")
    args = parser.parse_args()

    if args.assets:
        build_assets()
    elif args.check:
        check_assets()
    elif args.scanout_fixture:
        assemble(scanout_fixture=True)
    elif args.earth_auto_pair or args.earth_frame_trace or args.earth_pose or args.earth_pose_cost:
        assemble(earth_jit=True, pace="bombing", test_game=True, batch=256, auto_pair=True,
                 frame_trace=bool(args.earth_frame_trace or args.earth_pose or args.earth_pose_cost),
                 pose=args.earth_pose or args.earth_pose_cost, pose_cost=bool(args.earth_pose_cost))
    elif args.earth_jit_1k or args.earth_jit_256:
        assemble(earth_jit=True, pace="bombing", test_game=True, batch=256 if args.earth_jit_256 else 1024)
    elif args.test_game:
        assemble(test_game=True)
    elif args.sprite_spawn_review:
        assemble(scanout_fixture=True, spawn_review=True)
    elif args.staggered_pace:
        assemble(earth_jit=True, pace="control", staggered=True)
    elif args.aligned_pace:
        assemble(earth_jit=True, pace=args.aligned_pace)
    elif args.earth_jit:
        assemble(earth_jit=True)
    elif args.assemble:
        assemble()
    else:
        build_assets()
        check_assets()
        assemble()


if __name__ == "__main__":
    main()
