"""Assemble the standalone map suite without touching campaign level data."""

from __future__ import annotations

import argparse
from hashlib import sha256
from pathlib import Path
import re
import shutil
import subprocess
from tempfile import NamedTemporaryFile, TemporaryDirectory

from nurples_tools.maps import CompiledLevel, MapCompileError, compile_map_manifest


PROJECT_ROOT = Path(__file__).resolve().parents[2]
ASM_DIR = PROJECT_ROOT / "src/asm"
TARGET_DIR = PROJECT_ROOT / "tgt"
CAMPAIGN_LEVELS = ASM_DIR / "levels_tileset_0.inc"
DEFAULT_MANIFEST = PROJECT_ROOT / "tests/maps/dg/suite.toml"
OUTPUT_BINARY = TARGET_DIR / "nurples-map-tests.bin"
OUTPUT_SYMBOLS = TARGET_DIR / "nurples-map-tests.symbols"

TABLE_RE = re.compile(
    r"(?ms)^(tileset_00_levels:\s*\n)"
    r"(?P<entries>(?:[ \t]*dl[ \t]+(?!0\b)[^\n]+\n)+)"
    r"(?P<terminator>[ \t]*dl[ \t]+0\b[^\n]*\n)"
)
ENTRY_RE = re.compile(
    r"(?m)^[ \t]*dl[ \t]+([A-Za-z_][A-Za-z0-9_]*)(?:[ \t]*;[^\n]*)?$"
)
MAP_NAME_RE = re.compile(r"^PN-MAP-(\d{2})\b")


def rotate_pointer_table(source: str, start_position: int, expected_count: int) -> str:
    """Rotate only the level LUT so a selected fixture runs first."""

    table_match = TABLE_RE.search(source)
    if table_match is None:
        raise MapCompileError("test include lacks the canonical tileset_00 table")
    labels = ENTRY_RE.findall(table_match.group("entries").rstrip("\n"))
    if len(labels) != expected_count or len(set(labels)) != expected_count:
        raise MapCompileError("test include pointer table is incomplete or duplicated")
    if not 0 <= start_position < expected_count:
        raise MapCompileError(f"start position must be 0..{expected_count - 1}")

    rotated = labels[start_position:] + labels[:start_position]
    replacement = (
        table_match.group(1)
        + "".join(f"\tdl {label}\n" for label in rotated)
        + table_match.group("terminator")
    )
    return source[: table_match.start()] + replacement + source[table_match.end() :]


def qualification_start_position(
    levels: tuple[CompiledLevel, ...],
    start_map: int,
) -> int:
    """Resolve a PN-MAP ID to its position in deliberate runtime order."""

    if not 0 <= start_map <= 8:
        raise MapCompileError("start map must be a PN-MAP ID from 0 through 8")

    positions: dict[int, int] = {}
    for position, level in enumerate(levels):
        match = MAP_NAME_RE.match(level.name)
        if match is None:
            raise MapCompileError(f"qualification level lacks a PN-MAP ID: {level.name}")
        map_id = int(match.group(1), 10)
        if map_id in positions:
            raise MapCompileError(f"duplicate PN-MAP ID: {map_id:02d}")
        positions[map_id] = position

    if set(positions) != set(range(9)):
        raise MapCompileError("qualification suite must contain PN-MAP IDs 00 through 08")
    return positions[start_map]


def assemble(manifest_path: Path, start_map: int) -> Path:
    """Compile map data in memory and assemble through an isolated source tree."""

    compiled = compile_map_manifest(manifest_path)
    if len(compiled.levels) != 9:
        raise MapCompileError(
            f"qualification suite must contain exactly 9 maps, got {len(compiled.levels)}"
        )
    start_position = qualification_start_position(compiled.levels, start_map)
    suite_source = rotate_pointer_table(
        compiled.text,
        start_position,
        len(compiled.levels),
    )
    assembler = shutil.which("ez80asm") or str(Path.home() / ".local/bin/ez80asm")
    TARGET_DIR.mkdir(parents=True, exist_ok=True)

    campaign_before = CAMPAIGN_LEVELS.read_bytes()
    with TemporaryDirectory(prefix="nurples-map-suite-") as temp_dir:
        staged_asm = Path(temp_dir) / "asm"
        shutil.copytree(ASM_DIR, staged_asm)
        (staged_asm / CAMPAIGN_LEVELS.name).write_text(
            suite_source,
            encoding="utf-8",
        )
        staged_binary = staged_asm / OUTPUT_BINARY.name
        staged_symbols = staged_asm / "nurples.symbols"
        staged_symbols.unlink(missing_ok=True)
        subprocess.run(
            [assembler, "-l", "-s", "nurples.asm", staged_binary.name],
            cwd=staged_asm,
            check=True,
        )
        if (
            not staged_binary.is_file()
            or staged_binary.stat().st_size == 0
            or not staged_symbols.is_file()
            or staged_symbols.stat().st_size == 0
        ):
            raise RuntimeError("assembler did not produce the staged map-test pair")
        if CAMPAIGN_LEVELS.read_bytes() != campaign_before:
            raise RuntimeError("campaign level include changed during isolated assembly")

        for staged, destination in (
            (staged_binary, OUTPUT_BINARY),
            (staged_symbols, OUTPUT_SYMBOLS),
        ):
            with NamedTemporaryFile(
                dir=TARGET_DIR,
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

    if not OUTPUT_BINARY.is_file() or OUTPUT_BINARY.stat().st_size == 0:
        raise RuntimeError("map-test binary was not copied into tgt")
    if not OUTPUT_SYMBOLS.is_file() or OUTPUT_SYMBOLS.stat().st_size == 0:
        raise RuntimeError("map-test symbols were not copied into tgt")
    return OUTPUT_BINARY


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "assemble the standalone nine-map suite through a temporary source "
            "overlay; never replace campaign level data"
        )
    )
    parser.add_argument(
        "--manifest",
        type=Path,
        default=DEFAULT_MANIFEST,
        help="ordered map manifest (default: tests/maps/dg/suite.toml)",
    )
    parser.add_argument(
        "--start-map",
        type=int,
        default=0,
        help="PN-MAP index to run first while retaining cyclic suite order (0..8)",
    )
    args = parser.parse_args()

    try:
        binary = assemble(args.manifest, args.start_map)
    except MapCompileError as error:
        parser.exit(2, f"error: {error}\n")
    digest = sha256(binary.read_bytes()).hexdigest()
    print(f"Generated {binary.relative_to(PROJECT_ROOT)}")
    print(f"SHA-256  {digest}")


if __name__ == "__main__":
    main()
