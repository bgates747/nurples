"""Explicit orchestrator for Purple Nurples' current deployable application."""

import argparse
from pathlib import Path
import shutil
import subprocess
from tempfile import NamedTemporaryFile, TemporaryDirectory

from nurples_tools.current_assets import check_current_assets, write_current_assets
from nurples_tools.fonts import build_fonts, check_fonts


PROJECT_ROOT = Path(__file__).resolve().parents[2]
ASM_DIR = PROJECT_ROOT / "src/asm"
TARGET_DIR = PROJECT_ROOT / "tgt"
FONT_SOURCE = PROJECT_ROOT / "assets/fonts"
FONT_TARGET = TARGET_DIR / "fonts"
FONT_MANIFEST = ASM_DIR / "fonts_list.inc"


def build_assets() -> None:
    build_fonts(FONT_SOURCE, FONT_TARGET, FONT_MANIFEST)
    for output in write_current_assets(PROJECT_ROOT):
        print(f"Generated {output.relative_to(PROJECT_ROOT)}")


def check_assets() -> None:
    differences = check_fonts(FONT_SOURCE, FONT_TARGET, FONT_MANIFEST)
    differences.extend(check_current_assets(PROJECT_ROOT))
    if differences:
        raise RuntimeError("\n".join(differences))
    print("Current deployable assets are deterministic and up to date.")


def assemble() -> None:
    assembler = shutil.which("ez80asm") or str(
        Path.home() / ".local/bin/ez80asm"
    )
    TARGET_DIR.mkdir(parents=True, exist_ok=True)
    with TemporaryDirectory(prefix="nurples-assemble-") as temp_dir:
        staged_asm = Path(temp_dir) / "asm"
        shutil.copytree(ASM_DIR, staged_asm)
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
        for staged, destination in (
            (staged_binary, TARGET_DIR / "nurples.bin"),
            (staged_symbols, TARGET_DIR / "nurples.symbols"),
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
    print("Generated tgt/nurples.bin and tgt/nurples.symbols")


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
    args = parser.parse_args()

    if args.assets:
        build_assets()
    elif args.check:
        check_assets()
    elif args.assemble:
        assemble()
    else:
        build_assets()
        check_assets()
        assemble()


if __name__ == "__main__":
    main()
