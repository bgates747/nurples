"""Build or check an ordered Purple Nurples TMX map suite."""

import argparse
from pathlib import Path

from nurples_tools.maps import (
    MapCompileError,
    check_map_include,
    compile_map_manifest,
    write_map_include,
)

PROJECT_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_MANIFEST = PROJECT_ROOT / "tests/maps/dg/suite.toml"
CAMPAIGN_INCLUDE = (PROJECT_ROOT / "src/asm/levels_tileset_0.inc").resolve()
TEST_INCLUDE = (PROJECT_ROOT / "src/asm/levels_test_maps.inc").resolve()


def _validate_project_output(manifest: Path) -> None:
    output = compile_map_manifest(manifest).output_path
    try:
        output.relative_to(PROJECT_ROOT)
    except ValueError as error:
        raise MapCompileError(
            f"generated map output escapes the project: {output}"
        ) from error
    if output == CAMPAIGN_INCLUDE:
        raise MapCompileError("refusing to overwrite the campaign level include")
    if output != TEST_INCLUDE:
        raise MapCompileError(
            f"map output must be the dedicated test include: {TEST_INCLUDE}"
        )


def _display_path(path: Path) -> str:
    try:
        return path.relative_to(PROJECT_ROOT).as_posix()
    except ValueError:
        return str(path)


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Compile only the ordered TMX maps in a manifest; never regenerate "
            "tiles or image assets."
        )
    )
    parser.add_argument(
        "manifest",
        nargs="?",
        type=Path,
        default=DEFAULT_MANIFEST,
        help="ordered TOML map manifest (default: tests/maps/dg/suite.toml)",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="validate inputs and report generated-include drift without writing",
    )
    args = parser.parse_args()

    try:
        _validate_project_output(args.manifest)
        if args.check:
            differences = check_map_include(args.manifest)
            if differences:
                for difference in differences:
                    print(difference)
                raise SystemExit(1)
            print("Generated map include is deterministic and up to date.")
            return

        output_path = write_map_include(args.manifest)
    except MapCompileError as error:
        parser.exit(2, f"error: {error}\n")
    print(f"Generated {_display_path(output_path)}")


if __name__ == "__main__":
    main()
