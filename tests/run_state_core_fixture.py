#!/usr/bin/env python3
"""Assemble and repeatedly execute the pure-CPU state coordinator fixture."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys
from tempfile import TemporaryDirectory


PROJECT_ROOT = Path(__file__).resolve().parents[1]
FIXTURE = PROJECT_ROOT / "tests" / "asm" / "state_core_fixture.asm"
DEFAULT_HEADLESS = Path("/home/smith/Agon/mystuff/agon-headless-test")
DEFAULT_CARGO_TARGET = Path("/tmp/nurples-state-core-cargo")


def positive_int(value: str) -> int:
    parsed = int(value)
    if parsed <= 0:
        raise argparse.ArgumentTypeError("value must be positive")
    return parsed


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "assemble the self-checking ASM-001B fixture and run it through "
            "the stable Headless direct runner"
        )
    )
    parser.add_argument("--repeat", type=positive_int, default=20)
    parser.add_argument(
        "--headless-repo",
        type=Path,
        default=Path(os.environ.get("AGON_HEADLESS_TEST", DEFAULT_HEADLESS)),
    )
    parser.add_argument(
        "--cargo-target-dir",
        type=Path,
        default=DEFAULT_CARGO_TARGET,
    )
    parser.add_argument(
        "--runner",
        type=Path,
        help="use an existing agon-headless-test executable instead of cargo run",
    )
    return parser.parse_args()


def assemble(binary: Path) -> None:
    assembler = shutil.which("ez80asm") or "/home/smith/.local/bin/ez80asm"
    result = subprocess.run(
        [assembler, FIXTURE.name, str(binary), "-c"],
        cwd=FIXTURE.parent,
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
        "250000",
        "--max-cycles",
        "2000000",
        "--wall-timeout-ms",
        "2000",
        "--expect-guest-status",
        "0x2a",
    ]
    if args.runner is not None:
        return [str(args.runner), *direct_args], PROJECT_ROOT
    built_runner = args.headless_repo / "target" / "debug" / "agon-headless-test"
    if built_runner.is_file():
        return [str(built_runner), *direct_args], PROJECT_ROOT
    command = [
        "cargo",
        "run",
        "--locked",
        "--offline",
        "-q",
        "-p",
        "agon-headless-test",
        "--",
        *direct_args,
    ]
    return command, args.headless_repo


def main() -> int:
    args = parse_args()
    if not FIXTURE.is_file():
        raise SystemExit(f"fixture source not found: {FIXTURE}")
    if args.runner is None and not (args.headless_repo / "Cargo.toml").is_file():
        raise SystemExit(f"Headless repository not found: {args.headless_repo}")

    with TemporaryDirectory(prefix="nurples-state-core-") as temp_dir:
        binary = Path(temp_dir) / "state_core_fixture.bin"
        assemble(binary)
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
                sys.stderr.write(f"fixture run {run_number}/{args.repeat} failed\n")
                sys.stderr.write(last_output)
                return result.returncode
            if "PASS direct guest status 0x2a matched" not in last_output:
                sys.stderr.write(
                    f"fixture run {run_number}/{args.repeat} returned no PASS record\n"
                )
                sys.stderr.write(last_output)
                return 1

    detail = last_output.strip()
    print(f"PASS state coordinator fixture: {args.repeat} deterministic runs")
    if detail:
        print(detail)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
