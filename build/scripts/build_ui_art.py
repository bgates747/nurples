"""Scale cel UI art, convert to Agon64 and encode the existing RGBA2222 slots."""

import argparse
from pathlib import Path

from nurples_tools.ui_art import check_ui_art, write_ui_art


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="check without writing")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    if args.check:
        differences = check_ui_art(root)
        if differences:
            raise SystemExit("\n".join(differences))
        print("Prepared UI assets are up to date.")
    else:
        for output in write_ui_art(root):
            print(f"Generated {output.relative_to(root)}")


if __name__ == "__main__":
    main()
