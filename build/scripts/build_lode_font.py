"""Generate the exact 8x8 Lode Runner/Nurples hybrid font."""

import argparse
from pathlib import Path

from nurples_tools.lode_font import check_lode_font, write_lode_font


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="check without writing")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    if args.check:
        differences = check_lode_font(root)
        if differences:
            raise SystemExit("\n".join(differences))
        print("Lode Runner hybrid font is up to date.")
    else:
        for output in write_lode_font(root):
            print(f"Generated {output.relative_to(root)}")


if __name__ == "__main__":
    main()
