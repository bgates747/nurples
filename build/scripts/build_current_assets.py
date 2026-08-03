"""Build or check the deployable assets used by the current game."""

import argparse
from pathlib import Path

from nurples_tools.current_assets import check_current_assets, write_current_assets
from nurples_tools.fonts import build_fonts, check_fonts


PROJECT_ROOT = Path(__file__).resolve().parents[2]
FONT_SOURCE = PROJECT_ROOT / "assets/fonts"
FONT_TARGET = PROJECT_ROOT / "tgt/fonts"
FONT_MANIFEST = PROJECT_ROOT / "src/asm/fonts_list.inc"


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Build only Purple Nurples' currently consumed assets."
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="regenerate in temporary storage and report drift without writing",
    )
    args = parser.parse_args()

    if args.check:
        differences = check_fonts(FONT_SOURCE, FONT_TARGET, FONT_MANIFEST)
        differences.extend(check_current_assets(PROJECT_ROOT))
        if differences:
            for difference in differences:
                print(difference)
            raise SystemExit(1)
        print("Current deployable assets are deterministic and up to date.")
        return

    build_fonts(FONT_SOURCE, FONT_TARGET, FONT_MANIFEST)
    outputs = write_current_assets(PROJECT_ROOT)
    for output in outputs:
        print(f"Generated {output.relative_to(PROJECT_ROOT)}")


if __name__ == "__main__":
    main()
