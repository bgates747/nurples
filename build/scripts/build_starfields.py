"""Scale/slice the sparse starfield master and encode the four runtime tiles."""

import argparse
from pathlib import Path

from nurples_tools.starfields import check_starfields, write_starfields


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="check without writing")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    if args.check:
        differences = check_starfields(root)
        if differences:
            raise SystemExit("\n".join(differences))
        print("Starfields are up to date.")
    else:
        for output in write_starfields(root):
            print(f"Generated {output.relative_to(root)}")


if __name__ == "__main__":
    main()
