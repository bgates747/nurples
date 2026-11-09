"""Generate palette-native SVG masters and PNG previews for game overlays."""

import argparse
from pathlib import Path

from nurples_tools.overlay_art import check_overlay_art, write_overlay_art


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="check without writing")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    if args.check:
        differences = check_overlay_art(root)
        if differences:
            raise SystemExit("\n".join(differences))
        print("Overlay art is up to date.")
    else:
        for output in write_overlay_art(root):
            print(f"Generated {output.relative_to(root)}")


if __name__ == "__main__":
    main()
