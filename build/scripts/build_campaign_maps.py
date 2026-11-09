"""Build or check Purple Nurples' ordered production campaign maps."""

import argparse
from pathlib import Path

from nurples_tools.maps import (
    MapCompileError,
    check_map_include,
    compile_map_manifest,
    write_map_include,
)


PROJECT_ROOT = Path(__file__).resolve().parents[2]
CAMPAIGN_MANIFEST = PROJECT_ROOT / "maps/campaign.toml"
CAMPAIGN_INCLUDE = (PROJECT_ROOT / "src/asm/levels_tileset_0.inc").resolve()


def _validate_campaign_contract() -> None:
    compiled = compile_map_manifest(CAMPAIGN_MANIFEST)
    if compiled.output_path != CAMPAIGN_INCLUDE:
        raise MapCompileError(
            f"campaign output must be {CAMPAIGN_INCLUDE}; "
            f"got {compiled.output_path}"
        )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Build only the ordered production Purple Nurples campaign."
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="report generated campaign-include drift without writing",
    )
    args = parser.parse_args()
    try:
        _validate_campaign_contract()
        if args.check:
            differences = check_map_include(CAMPAIGN_MANIFEST)
            if differences:
                for difference in differences:
                    print(difference)
                raise SystemExit(1)
            print("Generated campaign include is deterministic and up to date.")
            return
        output = write_map_include(CAMPAIGN_MANIFEST)
    except MapCompileError as error:
        parser.exit(2, f"error: {error}\n")
    print(f"Generated {output.relative_to(PROJECT_ROOT)}")


if __name__ == "__main__":
    main()
