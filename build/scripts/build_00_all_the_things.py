"""Explicit orchestrator for Purple Nurples' current deployable application."""

import argparse
from pathlib import Path
import shutil
import subprocess

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
    subprocess.run(
        [assembler, "-l", "nurples.asm", "../../tgt/nurples.bin"],
        cwd=ASM_DIR,
        check=True,
    )
    print("Generated tgt/nurples.bin")


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
