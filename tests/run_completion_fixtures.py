#!/usr/bin/env python3
"""Assemble and repeatedly run all device-free game-completion fixtures."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys
from tempfile import TemporaryDirectory


PROJECT_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_HEADLESS = Path("/home/smith/Agon/mystuff/agon-headless-test")
DEFAULT_CARGO_TARGET = Path("/tmp/nurples-completion-fixtures-cargo")
ASSEMBLER_CANDIDATES = (
    Path("/home/smith/.local/bin/ez80asm"),
    Path(
        "/home/smith/Agon/mystuff/agon-fsim/tests/"
        "0053-flight011-agon/ez80asm"
    ),
)
FIXTURES = (
    ("input gate", "input_gate_fixture.asm"),
    ("heading slew", "heading_slew_fixture.asm"),
    ("safe spawn resolver", "player_spawn_resolver_fixture.asm"),
    ("turret tracking", "turret_tracking_fixture.asm"),
    ("seeker retreat", "seeker_retreat_fixture.asm"),
    ("starfield warp", "starfield_warp_fixture.asm"),
    ("level transition", "level_transition_controller_fixture.asm"),
    ("player victory", "player_victory_fixture.asm"),
    ("presentation overlay", "presentation_overlay_controller_fixture.asm"),
)


def positive_int(value: str) -> int:
    parsed = int(value)
    if parsed <= 0:
        raise argparse.ArgumentTypeError("value must be positive")
    return parsed


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="assemble and run the device-free completion fixture batch"
    )
    parser.add_argument("--repeat", type=positive_int, default=20)
    parser.add_argument("--assembler", type=Path)
    parser.add_argument(
        "--headless-repo",
        type=Path,
        default=Path(os.environ.get("AGON_HEADLESS_TEST", DEFAULT_HEADLESS)),
    )
    parser.add_argument("--cargo-target-dir", type=Path, default=DEFAULT_CARGO_TARGET)
    parser.add_argument(
        "--assemble-only",
        action="store_true",
        help="assemble every fixture in the short-name mirror without running it",
    )
    parser.add_argument(
        "--runner",
        type=Path,
        help="use an existing agon-headless-test executable instead of cargo run",
    )
    return parser.parse_args()


def find_assembler(explicit: Path | None) -> Path:
    candidates: list[Path] = []
    if explicit is not None:
        candidates.append(explicit)
    discovered = shutil.which("ez80asm")
    if discovered is not None:
        candidates.append(Path(discovered))
    candidates.extend(ASSEMBLER_CANDIDATES)
    for candidate in candidates:
        if candidate.is_file() and os.access(candidate, os.X_OK):
            return candidate
    raise SystemExit("ez80asm not found; pass --assembler with an executable path")


def assemble(assembler: Path, source: Path, binary: Path) -> None:
    result = subprocess.run(
        [str(assembler), source.name, str(binary), "-c"],
        cwd=source.parent,
        capture_output=True,
        check=False,
        text=True,
    )
    if result.returncode != 0:
        sys.stderr.write(result.stdout)
        sys.stderr.write(result.stderr)
        raise SystemExit(result.returncode)


def runner_command(args: argparse.Namespace, binary: Path) -> tuple[list[str], Path]:
    direct_args = [
        "run-direct",
        "--binary",
        str(binary),
        "--load-address",
        "0x040000",
        "--entry-address",
        "0x040000",
        "--adl",
        "true",
        "--madl",
        "false",
        "--mbase",
        "0",
        "--max-instructions",
        "200000",
        "--max-cycles",
        "2000000",
        "--wall-timeout-ms",
        "1000",
        "--expect-guest-status",
        "0x2a",
    ]
    if args.runner is not None:
        return [str(args.runner), *direct_args], PROJECT_ROOT
    built_runner = args.headless_repo / "target" / "debug" / "agon-headless-test"
    if built_runner.is_file():
        return [str(built_runner), *direct_args], PROJECT_ROOT
    if not (args.headless_repo / "Cargo.toml").is_file():
        raise SystemExit(f"headless repository not found: {args.headless_repo}")
    return (
        [
            "cargo",
            "run",
            "--locked",
            "--offline",
            "-q",
            "-p",
            "agon-headless-test",
            "--",
            *direct_args,
        ],
        args.headless_repo,
    )


def run_fixture(
    args: argparse.Namespace,
    label: str,
    binary: Path,
) -> None:
    command, cwd = runner_command(args, binary)
    environment = os.environ.copy()
    environment["CARGO_TARGET_DIR"] = str(args.cargo_target_dir)
    last_output = ""
    for run_number in range(1, args.repeat + 1):
        result = subprocess.run(
            command,
            cwd=cwd,
            env=environment,
            capture_output=True,
            check=False,
            text=True,
        )
        last_output = result.stdout + result.stderr
        if result.returncode != 0:
            sys.stderr.write(f"{label} run {run_number}/{args.repeat} failed\n")
            sys.stderr.write(last_output)
            raise SystemExit(result.returncode)
        if "PASS direct guest status 0x2a matched" not in last_output:
            sys.stderr.write(
                f"{label} run {run_number}/{args.repeat} returned no PASS record\n"
            )
            sys.stderr.write(last_output)
            raise SystemExit(1)
    print(f"PASS {label}: {args.repeat} deterministic runs")


def main() -> int:
    args = parse_args()
    assembler = find_assembler(args.assembler)
    fixture_root = PROJECT_ROOT / "tests" / "asm"

    with TemporaryDirectory(prefix="nurples-completion-fixtures-") as temp:
        mirror = Path(temp)
        shutil.copytree(PROJECT_ROOT / "src" / "asm", mirror / "src" / "asm")
        short_fixture_root = mirror / "tests" / "asm"
        short_fixture_root.mkdir(parents=True)

        built: list[tuple[str, Path]] = []
        for index, (label, filename) in enumerate(FIXTURES):
            source = fixture_root / filename
            if not source.is_file():
                raise SystemExit(f"fixture source not found: {source}")
            short_source = short_fixture_root / f"f{index}.asm"
            shutil.copy2(source, short_source)
            binary = short_fixture_root / f"f{index}.bin"
            assemble(assembler, short_source, binary)
            built.append((label, binary))

        if args.assemble_only:
            print(f"PASS completion fixture assembly: {len(built)} fixtures")
            return 0

        for label, binary in built:
            run_fixture(args, label, binary)

    print(
        f"PASS completion fixture batch: {len(FIXTURES)} fixtures, "
        f"{len(FIXTURES) * args.repeat} runs"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
