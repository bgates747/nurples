"""Assemble the isolated UART-001 game-output fixture.

The normal campaign source tree and binary are never rewritten. The builder
copies the assembly tree, adds the fixture include, redirects the one main-loop
dispatch call inside that temporary tree, and publishes a distinct binary and
symbol pair under ``tgt/``.
"""

from __future__ import annotations

import argparse
from hashlib import sha256
from pathlib import Path
import shutil
import subprocess
from tempfile import NamedTemporaryFile, TemporaryDirectory


PROJECT_ROOT = Path(__file__).resolve().parents[2]
ASM_DIR = PROJECT_ROOT / "src" / "asm"
FIXTURE = PROJECT_ROOT / "tests" / "asm" / "uart001_fixture.inc"
TARGET_DIR = PROJECT_ROOT / "tgt"
OUTPUT_BINARY = TARGET_DIR / "nurples-uart001.bin"
OUTPUT_SYMBOLS = TARGET_DIR / "nurples-uart001.symbols"

INCLUDE_ANCHOR = '    include "runtime_init.inc"\n'
PROCESS_INIT_ANCHOR = "    call runtime_process_init\n"
DISPATCH_ANCHOR = "    call do_game\n"


def replace_once(source: str, old: str, new: str, description: str) -> str:
    count = source.count(old)
    if count != 1:
        raise RuntimeError(f"expected one {description}, found {count}")
    return source.replace(old, new, 1)


def stage_fixture(staged_asm: Path) -> None:
    shutil.copy2(FIXTURE, staged_asm / FIXTURE.name)
    entry = staged_asm / "nurples.asm"
    source = entry.read_text(encoding="utf-8")
    source = replace_once(
        source,
        INCLUDE_ANCHOR,
        INCLUDE_ANCHOR + '    include "uart001_fixture.inc"\n',
        "runtime-init include anchor",
    )
    source = replace_once(
        source,
        PROCESS_INIT_ANCHOR,
        PROCESS_INIT_ANCHOR + "    call uart001_process_init\n",
        "process-init call",
    )
    source = replace_once(
        source,
        DISPATCH_ANCHOR,
        "    call uart001_do_game\n",
        "main-loop game dispatch",
    )
    entry.write_text(source, encoding="utf-8")


def publish(staged: Path, destination: Path) -> None:
    with NamedTemporaryFile(
        dir=destination.parent,
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


def prepare_staged_asm(staged_asm: Path) -> None:
    if staged_asm.exists():
        raise RuntimeError(f"staging path already exists: {staged_asm}")
    staged_asm.parent.mkdir(parents=True, exist_ok=True)
    shutil.copytree(ASM_DIR, staged_asm)
    stage_fixture(staged_asm)


def publish_pair(staged_asm: Path) -> tuple[Path, Path]:
    staged_binary = staged_asm / OUTPUT_BINARY.name
    staged_symbols = staged_asm / "nurples.symbols"
    if not staged_binary.is_file() or not staged_symbols.is_file():
        raise RuntimeError("staging directory lacks the assembled UART-001 pair")
    TARGET_DIR.mkdir(parents=True, exist_ok=True)
    publish(staged_binary, OUTPUT_BINARY)
    publish(staged_symbols, OUTPUT_SYMBOLS)
    return OUTPUT_BINARY, OUTPUT_SYMBOLS


def assemble() -> tuple[Path, Path]:
    assembler = shutil.which("ez80asm") or "/home/smith/.local/bin/ez80asm"
    TARGET_DIR.mkdir(parents=True, exist_ok=True)
    with TemporaryDirectory(prefix="nurples-uart001-") as temp_dir:
        staged_asm = Path(temp_dir) / "asm"
        prepare_staged_asm(staged_asm)

        staged_binary = staged_asm / OUTPUT_BINARY.name
        staged_symbols = staged_asm / "nurples.symbols"
        staged_symbols.unlink(missing_ok=True)
        subprocess.run(
            [assembler, "-l", "-s", "nurples.asm", staged_binary.name],
            cwd=staged_asm,
            check=True,
        )
        if not staged_binary.is_file() or not staged_symbols.is_file():
            raise RuntimeError("assembler did not produce the UART-001 pair")
        return publish_pair(staged_asm)


def report(binary: Path, symbols: Path) -> None:
    print(f"Generated {binary.relative_to(PROJECT_ROOT)}")
    print(f"Generated {symbols.relative_to(PROJECT_ROOT)}")
    print(f"SHA-256  {sha256(binary.read_bytes()).hexdigest()}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    actions = parser.add_mutually_exclusive_group()
    actions.add_argument(
        "--stage-only",
        type=Path,
        help="prepare an isolated assembly tree without invoking ez80asm",
    )
    actions.add_argument(
        "--publish-from",
        type=Path,
        help="publish an already assembled isolated tree into tgt",
    )
    args = parser.parse_args()

    if args.stage_only is not None:
        prepare_staged_asm(args.stage_only)
        print(f"Staged UART-001 assembly tree at {args.stage_only}")
        return
    if args.publish_from is not None:
        report(*publish_pair(args.publish_from))
        return
    report(*assemble())


if __name__ == "__main__":
    main()
