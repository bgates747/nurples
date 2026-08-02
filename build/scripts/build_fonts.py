#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path

from nurples_tools.fonts import build_fonts, check_fonts


PROJECT_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SOURCE = PROJECT_ROOT / "assets/fonts"
DEFAULT_TARGET = PROJECT_ROOT / "tgt/fonts"
DEFAULT_MANIFEST = PROJECT_ROOT / "src/asm/fonts_list.inc"


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build Purple Nurples fonts from vendored assets."
    )
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--target", type=Path, default=DEFAULT_TARGET)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    args = parser.parse_args()

    if args.check:
        errors = check_fonts(args.source, args.target, args.manifest)
        if errors:
            for error in errors:
                print(error)
            return 1
        print("Vendored fonts, target files, and manifest match.")
        return 0

    build_fonts(args.source, args.target, args.manifest)
    print(f"Built fonts from {args.source}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
