#!/usr/bin/env python3
"""Build the standalone hardware-overlay qualification bundle.

This command only assembles and packages files. It never deploys a profile or
launches an emulator.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import shutil
import subprocess
import sys
from tempfile import TemporaryDirectory


PROJECT_ROOT = Path(__file__).resolve().parents[2]
FIXTURE_SOURCE = (
    PROJECT_ROOT / "tests" / "asm" / "hardware_overlay_runtime_fixture.asm"
)
DEFAULT_OUTPUT = PROJECT_ROOT / ".build" / "hardware-overlay-runtime-r01"
PAYLOADS = {
    "game_over_0.rgba2": (240, 64),
    "game_over_1.rgba2": (240, 64),
    "press_any_key_to_continue.rgba2": (208, 16),
    "press_any_key_to_start.rgba2": (208, 16),
}
AUTOEXEC = b"SET KEYBOARD 1\r\nCD /apps/nurples-overlay\r\nLOAD overlay.bin\r\n"


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def validate_sources() -> list[str]:
    errors = []
    if not FIXTURE_SOURCE.is_file():
        errors.append(f"missing fixture source: {FIXTURE_SOURCE}")
    payload_root = PROJECT_ROOT / "assets" / "rgba2" / "overlays"
    for name, (width, height) in PAYLOADS.items():
        path = payload_root / name
        if not path.is_file():
            errors.append(f"missing overlay payload: {path}")
            continue
        expected_size = width * height
        actual_size = path.stat().st_size
        if actual_size != expected_size:
            errors.append(
                f"wrong payload size for {name}: {actual_size}, expected {expected_size}"
            )
    return errors


def resolve_assembler(requested: Path | None) -> Path:
    if requested is not None:
        candidate = requested
    else:
        found = shutil.which("ez80asm")
        candidate = Path(found) if found is not None else Path.home() / ".local/bin/ez80asm"
    if not candidate.is_file():
        raise RuntimeError(f"ez80asm not found: {candidate}")
    return candidate


def write_manifest(output_dir: Path, names: list[str]) -> None:
    lines = [f"{sha256(output_dir / name)}  {name}" for name in sorted(names)]
    (output_dir / "SHA256SUMS").write_text("\n".join(lines) + "\n")


def build_bundle(output_dir: Path, assembler: Path) -> None:
    errors = validate_sources()
    if errors:
        raise RuntimeError("\n".join(errors))
    output_dir.mkdir(parents=True, exist_ok=True)
    with TemporaryDirectory(prefix="nurples-overlay-build-") as temp_dir:
        mirror = Path(temp_dir)
        shutil.copytree(PROJECT_ROOT / "src" / "asm", mirror / "src" / "asm")
        fixture_root = mirror / "tests" / "asm"
        fixture_root.mkdir(parents=True)
        fixture = fixture_root / "f.asm"
        shutil.copy2(FIXTURE_SOURCE, fixture)
        binary = mirror / "overlay.bin"
        result = subprocess.run(
            [str(assembler), fixture.name, str(binary), "-c"],
            cwd=fixture_root,
            capture_output=True,
            check=False,
            text=True,
        )
        if result.returncode != 0:
            raise RuntimeError(result.stdout + result.stderr)
        if not binary.is_file() or binary.stat().st_size == 0:
            raise RuntimeError("assembler produced no overlay fixture binary")
        (output_dir / "overlay.bin").write_bytes(binary.read_bytes())

    payload_root = PROJECT_ROOT / "assets" / "rgba2" / "overlays"
    for name in PAYLOADS:
        (output_dir / name).write_bytes((payload_root / name).read_bytes())
    (output_dir / "autoexec.txt").write_bytes(AUTOEXEC)
    write_manifest(output_dir, ["overlay.bin", *PAYLOADS, "autoexec.txt"])


def check_bundle(output_dir: Path) -> list[str]:
    errors = validate_sources()
    manifest = output_dir / "SHA256SUMS"
    if not manifest.is_file():
        errors.append(f"missing bundle manifest: {manifest}")
        return errors
    expected_names = {"overlay.bin", *PAYLOADS, "autoexec.txt"}
    seen = set()
    for line_number, line in enumerate(manifest.read_text().splitlines(), 1):
        try:
            expected_hash, name = line.split("  ", 1)
        except ValueError:
            errors.append(f"malformed SHA256SUMS line {line_number}")
            continue
        seen.add(name)
        path = output_dir / name
        if not path.is_file():
            errors.append(f"missing bundle file: {name}")
        elif sha256(path) != expected_hash:
            errors.append(f"bundle hash mismatch: {name}")
    if seen != expected_names:
        errors.append(
            "bundle file set mismatch: "
            f"saw {sorted(seen)}, expected {sorted(expected_names)}"
        )
    if (output_dir / "autoexec.txt").is_file():
        if (output_dir / "autoexec.txt").read_bytes() != AUTOEXEC:
            errors.append("autoexec.txt does not match the load-only contract")
    return errors


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--assembler", type=Path)
    action = parser.add_mutually_exclusive_group()
    action.add_argument(
        "--source-check",
        action="store_true",
        help="validate source payloads without requiring an assembler",
    )
    action.add_argument(
        "--check",
        action="store_true",
        help="verify an already-built bundle without writing",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.source_check:
        errors = validate_sources()
        if errors:
            raise SystemExit("\n".join(errors))
        print("Hardware-overlay fixture sources are ready.")
        return 0
    if args.check:
        errors = check_bundle(args.output_dir.resolve())
        if errors:
            raise SystemExit("\n".join(errors))
        print(f"Hardware-overlay fixture bundle verified: {args.output_dir.resolve()}")
        return 0

    try:
        assembler = resolve_assembler(args.assembler)
        build_bundle(args.output_dir.resolve(), assembler)
    except RuntimeError as error:
        sys.stderr.write(f"{error}\n")
        return 1
    print(f"Built hardware-overlay fixture bundle: {args.output_dir.resolve()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
