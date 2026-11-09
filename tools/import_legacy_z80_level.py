"""Import only the original Z80 Nurples main-level data into a TMX map."""

import argparse
from pathlib import Path

from nurples_tools.legacy_level import LegacyLevelError, import_main_level


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Decode only tiles_level_1 from a recovered Z80 Nurples assembly "
            "source; no original executable code is imported."
        )
    )
    parser.add_argument("source", type=Path, help="recovered nurples.asm")
    parser.add_argument("output", type=Path, help="destination TMX map")
    args = parser.parse_args()
    try:
        import_main_level(args.source, args.output)
    except (OSError, LegacyLevelError) as error:
        parser.exit(2, f"error: {error}\n")
    print(f"Generated {args.output}")


if __name__ == "__main__":
    main()
