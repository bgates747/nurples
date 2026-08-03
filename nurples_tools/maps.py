"""Strict, map-only compiler for Purple Nurples Tiled levels."""

from __future__ import annotations

import csv
import re
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from io import StringIO
from pathlib import Path

import tomllib

MAP_WIDTH = 16
MIN_MAP_HEIGHT = 1
MAX_MAP_HEIGHT = 256
MAX_LEVELS = 255
MAX_TILE_ID = 255
DG_TILE_BUFFER_BASE = 512
TILED_FLIP_MASK = 0xF000_0000

_MANIFEST_KEYS = {
    "format_version",
    "output",
    "symbol_prefix",
    "tile_catalog",
    "tileset",
    "levels",
}
_LEVEL_KEYS = {"name", "source"}
_SYMBOL_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*\Z")


class MapCompileError(ValueError):
    """Raised when a manifest, TMX map, or supporting catalog is invalid."""


@dataclass(frozen=True)
class CompiledLevel:
    """One validated level in the runtime's bottom-to-top row order."""

    name: str
    source_path: Path
    height: int
    runtime_rows: tuple[tuple[int, ...], ...]


@dataclass(frozen=True)
class CompiledMapSet:
    """A complete deterministic assembly include and its destination."""

    manifest_path: Path
    output_path: Path
    symbol_prefix: str
    base_buffer_id: int
    levels: tuple[CompiledLevel, ...]
    text: str


def _parse_xml(path: Path, description: str) -> ET.Element:
    if not path.is_file():
        raise MapCompileError(f"missing {description}: {path}")
    try:
        return ET.parse(path).getroot()
    except (ET.ParseError, OSError) as error:
        raise MapCompileError(f"cannot parse {description} {path}: {error}") from error


def _decimal(text: str | None, description: str) -> int:
    if text is None or re.fullmatch(r"[0-9]+", text) is None:
        raise MapCompileError(f"{description} must be an unsigned decimal integer")
    return int(text, 10)


def _relative_path(value: object, field: str, manifest_dir: Path) -> Path:
    if not isinstance(value, str) or not value:
        raise MapCompileError(f"manifest {field!r} must be a non-empty path string")
    path = Path(value)
    if path.is_absolute():
        raise MapCompileError(f"manifest {field!r} must be relative for portability")
    return (manifest_dir / path).resolve()


def _validate_tileset(tileset_path: Path) -> None:
    if tileset_path.name != "dg0.tsx":
        raise MapCompileError("manifest tileset must name the external dg0.tsx")
    root = _parse_xml(tileset_path, "DG tileset")
    if root.tag != "tileset":
        raise MapCompileError(f"unexpected DG tileset root: {root.tag}")

    expected = {
        "name": "dg0",
        "tilewidth": "16",
        "tileheight": "16",
        "tilecount": "256",
        "columns": "16",
    }
    for attribute, wanted in expected.items():
        actual = root.attrib.get(attribute)
        if actual != wanted:
            raise MapCompileError(
                f"DG tileset {attribute} is {actual!r}; expected {wanted!r}"
            )
    images = root.findall("image")
    if len(images) != 1:
        raise MapCompileError("DG tileset must contain exactly one image")
    image = images[0]
    expected_image = {
        "source": "dg0.png",
        "width": "256",
        "height": "256",
    }
    for attribute, wanted in expected_image.items():
        actual = image.attrib.get(attribute)
        if actual != wanted:
            raise MapCompileError(
                f"DG tileset image {attribute} is {actual!r}; expected {wanted!r}"
            )


def _load_tile_catalog(catalog_path: Path) -> tuple[frozenset[int], int]:
    root = _parse_xml(catalog_path, "DG tile catalog")
    if root.tag != "TileSet":
        raise MapCompileError(f"unexpected DG tile catalog root: {root.tag}")
    if root.findtext("TileWidth") != "16" or root.findtext("TileHeight") != "16":
        raise MapCompileError("DG tile catalog must describe 16x16 tiles")

    base_buffer_id = _decimal(
        root.findtext("BaseBufferId"),
        "DG tile catalog BaseBufferId",
    )
    if base_buffer_id != DG_TILE_BUFFER_BASE:
        raise MapCompileError(
            f"DG tile catalog BaseBufferId is {base_buffer_id}; "
            f"expected {DG_TILE_BUFFER_BASE}"
        )

    collections = root.findall("Tiles")
    if len(collections) != 1:
        raise MapCompileError("DG tile catalog must contain exactly one Tiles collection")

    indices: set[int] = set()
    for tile in collections[0]:
        if tile.tag != "Tile":
            raise MapCompileError(
                f"unexpected element in DG tile catalog Tiles: {tile.tag}"
            )
        index = _decimal(tile.attrib.get("index"), "DG tile catalog index")
        if index > MAX_TILE_ID:
            raise MapCompileError(f"DG tile catalog index {index} exceeds 255")
        if index in indices:
            raise MapCompileError(f"duplicate DG tile catalog index {index}")
        indices.add(index)

    if not indices:
        raise MapCompileError("DG tile catalog has no tile records")
    if 0 not in indices:
        raise MapCompileError("DG tile catalog must load tile index 0")
    if base_buffer_id + max(indices) > 65_535:
        raise MapCompileError("DG tile buffers exceed the 16-bit buffer ID range")
    return frozenset(indices), base_buffer_id


def _parse_csv_rows(
    data: ET.Element,
    *,
    source_path: Path,
    height: int,
    available_tiles: frozenset[int],
) -> tuple[tuple[int, ...], ...]:
    if data.attrib.get("encoding") != "csv":
        raise MapCompileError(f"{source_path}: tile layer must use CSV encoding")
    if "compression" in data.attrib:
        raise MapCompileError(f"{source_path}: CSV tile layer cannot be compressed")
    if list(data):
        raise MapCompileError(f"{source_path}: chunked or nested layer data is unsupported")
    if data.text is None or not data.text.strip():
        raise MapCompileError(f"{source_path}: CSV tile layer is empty")

    source_lines = data.text.strip().splitlines()
    if len(source_lines) != height:
        raise MapCompileError(
            f"{source_path}: CSV has {len(source_lines)} rows; expected {height}"
        )

    rows: list[tuple[int, ...]] = []
    for row_number, line in enumerate(source_lines, start=1):
        if '"' in line:
            raise MapCompileError(
                f"{source_path}: CSV row {row_number} contains quoted fields"
            )
        try:
            fields = next(csv.reader(StringIO(line), strict=True))
        except csv.Error as error:
            raise MapCompileError(
                f"{source_path}: invalid CSV row {row_number}: {error}"
            ) from error

        # Tiled conventionally emits one trailing comma on all but the last row.
        if len(fields) == MAP_WIDTH + 1 and not fields[-1].strip():
            fields.pop()
        if len(fields) != MAP_WIDTH:
            raise MapCompileError(
                f"{source_path}: CSV row {row_number} has {len(fields)} cells; "
                f"expected {MAP_WIDTH}"
            )

        parsed_row: list[int] = []
        for column, field in enumerate(fields, start=1):
            token = field.strip()
            if re.fullmatch(r"[0-9]+", token) is None:
                raise MapCompileError(
                    f"{source_path}: row {row_number}, column {column} "
                    "is not an unsigned decimal GID"
                )
            gid = int(token, 10)
            if gid & TILED_FLIP_MASK:
                raise MapCompileError(
                    f"{source_path}: row {row_number}, column {column} "
                    f"uses unsupported Tiled transform bits in GID {gid}"
                )
            if gid > MAX_TILE_ID:
                raise MapCompileError(
                    f"{source_path}: row {row_number}, column {column} "
                    f"GID {gid} exceeds 255"
                )
            if gid and gid not in available_tiles:
                raise MapCompileError(
                    f"{source_path}: row {row_number}, column {column} "
                    f"uses GID {gid}, absent from dg_0_files.xml"
                )
            parsed_row.append(gid)
        rows.append(tuple(parsed_row))
    return tuple(rows)


def _compile_tmx(
    source_path: Path,
    *,
    name: str,
    tileset_path: Path,
    available_tiles: frozenset[int],
) -> CompiledLevel:
    root = _parse_xml(source_path, "TMX map")
    if root.tag != "map":
        raise MapCompileError(f"{source_path}: unexpected TMX root {root.tag}")
    if root.attrib.get("orientation") != "orthogonal":
        raise MapCompileError(f"{source_path}: map orientation must be orthogonal")
    if root.attrib.get("infinite", "0") != "0":
        raise MapCompileError(f"{source_path}: infinite maps are unsupported")

    width = _decimal(root.attrib.get("width"), f"{source_path}: map width")
    height = _decimal(root.attrib.get("height"), f"{source_path}: map height")
    if width != MAP_WIDTH:
        raise MapCompileError(f"{source_path}: map width is {width}; expected 16")
    if not MIN_MAP_HEIGHT <= height <= MAX_MAP_HEIGHT:
        raise MapCompileError(
            f"{source_path}: map height {height} is outside 1..256"
        )
    tile_width = _decimal(
        root.attrib.get("tilewidth"),
        f"{source_path}: tile width",
    )
    tile_height = _decimal(
        root.attrib.get("tileheight"),
        f"{source_path}: tile height",
    )
    if (tile_width, tile_height) != (16, 16):
        raise MapCompileError(
            f"{source_path}: tile dimensions are {tile_width}x{tile_height}; "
            "expected 16x16"
        )

    tilesets = root.findall("tileset")
    if len(tilesets) != 1:
        raise MapCompileError(f"{source_path}: map must contain exactly one tileset")
    tileset = tilesets[0]
    first_gid = _decimal(
        tileset.attrib.get("firstgid"),
        f"{source_path}: tileset firstgid",
    )
    if first_gid != 1:
        raise MapCompileError(f"{source_path}: dg0.tsx firstgid must be 1")
    tileset_source = tileset.attrib.get("source")
    if not tileset_source:
        raise MapCompileError(f"{source_path}: tileset must be an external dg0.tsx")
    if list(tileset):
        raise MapCompileError(f"{source_path}: embedded tileset data is unsupported")
    referenced_tileset = (source_path.parent / tileset_source).resolve()
    if referenced_tileset != tileset_path:
        raise MapCompileError(
            f"{source_path}: map must reference the manifest's external dg0.tsx"
        )

    content_tags = {"layer", "objectgroup", "imagelayer", "group"}
    content = [child for child in root if child.tag in content_tags]
    if len(content) != 1 or content[0].tag != "layer":
        raise MapCompileError(
            f"{source_path}: map must contain exactly one tile layer and no other layers"
        )
    layer = content[0]
    layer_width = _decimal(
        layer.attrib.get("width"),
        f"{source_path}: layer width",
    )
    layer_height = _decimal(
        layer.attrib.get("height"),
        f"{source_path}: layer height",
    )
    if (layer_width, layer_height) != (width, height):
        raise MapCompileError(
            f"{source_path}: layer dimensions {layer_width}x{layer_height} "
            f"do not match map dimensions {width}x{height}"
        )
    for attribute in ("x", "y", "offsetx", "offsety"):
        value = layer.attrib.get(attribute, "0")
        if value not in {"0", "0.0"}:
            raise MapCompileError(
                f"{source_path}: nonzero layer {attribute} is unsupported"
            )

    data_elements = layer.findall("data")
    if len(data_elements) != 1:
        raise MapCompileError(f"{source_path}: tile layer must contain exactly one data element")
    tiled_rows = _parse_csv_rows(
        data_elements[0],
        source_path=source_path,
        height=height,
        available_tiles=available_tiles,
    )
    return CompiledLevel(
        name=name,
        source_path=source_path,
        height=height,
        runtime_rows=tuple(reversed(tiled_rows)),
    )


def _render_include(
    symbol_prefix: str,
    base_buffer_id: int,
    levels: tuple[CompiledLevel, ...],
) -> str:
    lines = [
        "; Generated by build_maps.py; do not edit.\n\n",
        f"{symbol_prefix}_num_levels: db {len(levels)}\n\n",
        f"{symbol_prefix}_levels:\n",
    ]
    for index in range(len(levels)):
        lines.append(f"\tdl {symbol_prefix}_level_{index:02X}\n")
    lines.append("\tdl 0 ; list terminator\n\n")

    for index, level in enumerate(levels):
        lines.extend(
            (
                f"{symbol_prefix}_level_{index:02X}: ; {level.name}\n",
                f"\tdb {MAP_WIDTH}          ; num cols\n",
                f"\tdl {level.height}          ; num rows\n",
                f"\tdl {base_buffer_id}        ; base bufferId\n",
            )
        )
        for row in level.runtime_rows:
            lines.append("\tdb " + ",".join(f"{gid:03d}" for gid in row) + "\n")
        if index + 1 != len(levels):
            lines.append("\n")
    return "".join(lines)


def compile_map_manifest(manifest_path: Path) -> CompiledMapSet:
    """Validate every input and return the deterministic include without writing."""

    manifest_path = manifest_path.resolve()
    if not manifest_path.is_file():
        raise MapCompileError(f"missing map manifest: {manifest_path}")
    try:
        with manifest_path.open("rb") as manifest_file:
            manifest = tomllib.load(manifest_file)
    except (OSError, tomllib.TOMLDecodeError) as error:
        raise MapCompileError(f"cannot parse map manifest {manifest_path}: {error}") from error
    if not isinstance(manifest, dict):
        raise MapCompileError("map manifest must be a TOML table")

    actual_keys = set(manifest)
    missing = sorted(_MANIFEST_KEYS - actual_keys)
    unknown = sorted(actual_keys - _MANIFEST_KEYS)
    if missing:
        raise MapCompileError(f"map manifest missing keys: {', '.join(missing)}")
    if unknown:
        raise MapCompileError(f"map manifest has unknown keys: {', '.join(unknown)}")
    if type(manifest["format_version"]) is not int or manifest["format_version"] != 1:
        raise MapCompileError("map manifest format_version must be integer 1")

    symbol_prefix = manifest["symbol_prefix"]
    if not isinstance(symbol_prefix, str) or _SYMBOL_RE.fullmatch(symbol_prefix) is None:
        raise MapCompileError("map manifest symbol_prefix is not an assembly identifier")

    manifest_dir = manifest_path.parent
    output_path = _relative_path(manifest["output"], "output", manifest_dir)
    tileset_path = _relative_path(manifest["tileset"], "tileset", manifest_dir)
    catalog_path = _relative_path(
        manifest["tile_catalog"],
        "tile_catalog",
        manifest_dir,
    )
    _validate_tileset(tileset_path)
    available_tiles, base_buffer_id = _load_tile_catalog(catalog_path)

    level_entries = manifest["levels"]
    if not isinstance(level_entries, list):
        raise MapCompileError("map manifest levels must be an ordered array of tables")
    if not 1 <= len(level_entries) <= MAX_LEVELS:
        raise MapCompileError("map manifest must contain 1..255 levels")

    levels: list[CompiledLevel] = []
    seen_names: set[str] = set()
    seen_sources: set[Path] = set()
    for index, entry in enumerate(level_entries):
        description = f"map manifest level {index}"
        if not isinstance(entry, dict):
            raise MapCompileError(f"{description} must be a table")
        keys = set(entry)
        if keys != _LEVEL_KEYS:
            missing_level = sorted(_LEVEL_KEYS - keys)
            unknown_level = sorted(keys - _LEVEL_KEYS)
            details = []
            if missing_level:
                details.append(f"missing {', '.join(missing_level)}")
            if unknown_level:
                details.append(f"unknown {', '.join(unknown_level)}")
            raise MapCompileError(f"{description}: {'; '.join(details)}")

        name = entry["name"]
        if (
            not isinstance(name, str)
            or not name
            or not name.isascii()
            or not name.isprintable()
        ):
            raise MapCompileError(f"{description} name must be printable, one-line ASCII")
        if name in seen_names:
            raise MapCompileError(f"duplicate map manifest level name: {name}")
        seen_names.add(name)

        source_path = _relative_path(entry["source"], "level source", manifest_dir)
        if source_path in seen_sources:
            raise MapCompileError(f"duplicate map manifest level source: {source_path}")
        seen_sources.add(source_path)
        levels.append(
            _compile_tmx(
                source_path,
                name=name,
                tileset_path=tileset_path,
                available_tiles=available_tiles,
            )
        )

    frozen_levels = tuple(levels)
    text = _render_include(symbol_prefix, base_buffer_id, frozen_levels)
    return CompiledMapSet(
        manifest_path=manifest_path,
        output_path=output_path,
        symbol_prefix=symbol_prefix,
        base_buffer_id=base_buffer_id,
        levels=frozen_levels,
        text=text,
    )


def write_map_include(manifest_path: Path) -> Path:
    """Compile and write only the declared assembly include, if it changed."""

    compiled = compile_map_manifest(manifest_path)
    wanted = compiled.text.encode("utf-8")
    if compiled.output_path.is_file() and compiled.output_path.read_bytes() == wanted:
        return compiled.output_path
    compiled.output_path.parent.mkdir(parents=True, exist_ok=True)
    compiled.output_path.write_bytes(wanted)
    return compiled.output_path


def check_map_include(manifest_path: Path) -> list[str]:
    """Return output drift without writing to the filesystem."""

    compiled = compile_map_manifest(manifest_path)
    if not compiled.output_path.is_file():
        return [f"missing generated map include: {compiled.output_path}"]
    if compiled.output_path.read_bytes() != compiled.text.encode("utf-8"):
        return [f"changed generated map include: {compiled.output_path}"]
    return []
