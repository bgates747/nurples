"""Strict importer for the data-only main level from original Z80 Nurples."""

from __future__ import annotations

import re
from pathlib import Path
import xml.etree.ElementTree as ET

from nurples_tools.tile_bindings import IDENTITY, bindings


LEGACY_ROWS = 255
MAP_WIDTH = 16
LEGACY_TO_MODERN_TILE = {
    0: 0,   # empty
    1: 17,  # original passive blank panel
    2: 18,  # original passive circle
    4: 19,  # original passive conduit cross
    5: 20,  # original passive horizontal conduit
    7: 1,   # modern animated launch pad
    8: 5,   # modern animated laser turret
    11: 21, # original passive vertical conduit
}

_LEVEL_HEADER = re.compile(
    r"^tiles_level_1:\s*defb\s+255\s*;\s*number of rows\s*$",
    re.IGNORECASE | re.MULTILINE,
)
_ROW = re.compile(
    r"^\s*defb\s+(.+?)\s*;\s*row\s+([0-9]+)\s*$",
    re.IGNORECASE,
)


class LegacyLevelError(ValueError):
    """Raised when the recovered level definition violates its data contract."""


def _hex_byte(token: str, row_number: int) -> int:
    token = token.strip()
    if re.fullmatch(r"#[0-9A-Fa-f]{2}", token) is None:
        raise LegacyLevelError(
            f"legacy row {row_number} has invalid byte token {token!r}"
        )
    return int(token[1:], 16)


def parse_main_level(source_text: str) -> tuple[tuple[int, ...], ...]:
    """Decode `tiles_level_1` into modern runtime bottom-to-top rows."""

    header = _LEVEL_HEADER.search(source_text)
    if header is None:
        raise LegacyLevelError("missing exact 255-row tiles_level_1 definition")

    rows: list[tuple[int, ...]] = []
    for line in source_text[header.end():].splitlines():
        match = _ROW.fullmatch(line)
        if match is None:
            if rows and len(rows) < LEGACY_ROWS and line.strip().endswith(":"):
                break
            continue
        row_number = int(match.group(2), 10)
        if row_number != len(rows) + 1:
            raise LegacyLevelError(
                f"legacy row sequence reached {row_number}; expected {len(rows) + 1}"
            )
        values = [
            _hex_byte(token, row_number)
            for token in match.group(1).split(",")
        ]
        mask = values[0]
        packed_pairs = iter(values[1:])
        legacy_row: list[int] = []
        for mask_bit in range(7, -1, -1):
            if mask & (1 << mask_bit):
                try:
                    packed = next(packed_pairs)
                except StopIteration as error:
                    raise LegacyLevelError(
                        f"legacy row {row_number} is shorter than mask {mask:02X}"
                    ) from error
                legacy_row.extend((packed >> 4, packed & 0x0F))
            else:
                legacy_row.extend((0, 0))
        try:
            next(packed_pairs)
        except StopIteration:
            pass
        else:
            raise LegacyLevelError(
                f"legacy row {row_number} is longer than mask {mask:02X}"
            )

        unknown = sorted(set(legacy_row) - LEGACY_TO_MODERN_TILE.keys())
        if unknown:
            raise LegacyLevelError(
                f"legacy row {row_number} uses unmapped tile IDs {unknown}"
            )
        modern_row = tuple(LEGACY_TO_MODERN_TILE[tile] for tile in legacy_row)
        if len(modern_row) != MAP_WIDTH:
            raise LegacyLevelError(
                f"legacy row {row_number} decoded to {len(modern_row)} columns"
            )
        rows.append(modern_row)
        if len(rows) == LEGACY_ROWS:
            break

    if len(rows) != LEGACY_ROWS:
        raise LegacyLevelError(
            f"tiles_level_1 contains {len(rows)} rows; expected {LEGACY_ROWS}"
        )
    return tuple(rows)


def render_tmx(runtime_rows: tuple[tuple[int, ...], ...], runtime_ids=IDENTITY) -> str:
    """Render runtime rows as a portable Tiled top-to-bottom CSV map."""

    if len(runtime_rows) != LEGACY_ROWS:
        raise LegacyLevelError(
            f"legacy map contains {len(runtime_rows)} rows; expected {LEGACY_ROWS}"
        )
    editor_gids = {runtime: gid for gid, runtime in enumerate(runtime_ids)}
    csv_rows = []
    for row in reversed(runtime_rows):
        if len(row) != MAP_WIDTH:
            raise LegacyLevelError(f"legacy map row contains {len(row)} columns")
        csv_rows.append(",".join(str(editor_gids[tile]) for tile in row))
    csv_text = ",\n".join(csv_rows)
    return (
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        '<map version="1.10" tiledversion="1.11.0" orientation="orthogonal" '
        'renderorder="right-down" width="16" height="255" tilewidth="16" '
        'tileheight="16" infinite="0" nextlayerid="2" nextobjectid="1">\n'
        ' <tileset firstgid="1" source="dg0.tsx"/>\n'
        ' <layer id="1" name="Original Z80 main level" width="16" height="255">\n'
        '  <data encoding="csv">\n'
        f'{csv_text}\n'
        '  </data>\n'
        ' </layer>\n'
        '</map>\n'
    )


def import_main_level(source_path: Path, output_path: Path) -> None:
    """Read the recovered source and write only its decoded main-level data."""

    runtime_rows = parse_main_level(source_path.read_text(encoding="utf-8"))
    output_path.parent.mkdir(parents=True, exist_ok=True)
    tsx = output_path.with_name("dg0.tsx")
    if not tsx.exists():
        # Standalone exports to /tmp still use the project's current bindings.
        tsx = Path(__file__).resolve().parents[1] / "tiles/dg/dg0.tsx"
    runtime_ids = bindings(ET.parse(tsx).getroot())
    output_path.write_text(render_tmx(runtime_rows, runtime_ids), encoding="utf-8")
