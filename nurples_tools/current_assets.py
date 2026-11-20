"""Build exactly the bitmap assets consumed by the current Purple Nurples game."""

from collections.abc import Iterable
from pathlib import Path
from tempfile import TemporaryDirectory
import xml.etree.ElementTree as ET

from PIL import Image

from nurples_tools.agnb import ImageRecord, build_container, validate_records
from nurples_tools.agnb_verify import verify_container
from nurples_tools.legacy_tiles import check_legacy_tiles, write_legacy_tiles
from nurples_tools.overlay_art import check_overlay_art, write_overlay_art
from nurples_tools.ui_art import UI_ASSET_COUNT, check_ui_art, ui_manifest, write_ui_art
from nurples_tools.starfields import check_starfields, write_starfields


UI_BUFFER_BASE = 0
SPRITE_BUFFER_BASE = 256
TILE_BUFFER_BASE = 512
BACKGROUND_BUFFER_BASE = 1024
OVERLAY_BUFFER_BASE = 63_990
TITLE_PHASE_MAP_BUFFER = 63_984
TITLE_PHASES = ("splash_title", "game_over")
DERIVED_TITLE_NAMES = frozenset(f"{phase}_1" for phase in TITLE_PHASES)

EXPECTED_COUNTS = {
    "ui": UI_ASSET_COUNT,
    "bg": 4,
    "sprites": 159,
    "tiles_dg": 192,
    "overlays": 4,
}

ROTATING_SPRITES = frozenset({"seeker", "turret", "fireball_2", "fireball_3"})


def _rotate_image(image: Image.Image, degrees_256: int) -> Image.Image:
    """Reproduce the game's established 256-degree sprite rotation."""

    degrees = 360 - degrees_256 * 360 / 256
    width, height = image.size
    rotated = image.rotate(degrees, resample=Image.Resampling.NEAREST, expand=True)
    center_x, center_y = rotated.size[0] // 2, rotated.size[1] // 2
    left = center_x - width // 2
    top = center_y - height // 2
    return rotated.crop((left, top, left + width, top + height))


def _ordinary_family(
    family: str,
    source_dir: Path,
    payload_dir: Path,
    buffer_base: int,
) -> list[ImageRecord]:
    records: list[ImageRecord] = []
    sources = sorted(source_dir.glob("*.png"), key=lambda path: path.name)
    for index, source_path in enumerate(sources):
        name = source_path.stem
        payload_path = payload_dir / f"{name}.rgba2"
        with Image.open(source_path) as source:
            width, height = source.size
        records.append(
            ImageRecord(
                family=family,
                name=name,
                buffer_id=buffer_base + index,
                width=width,
                height=height,
                payload_path=payload_path,
            )
        )
    return records


def _sprites(
    source_dir: Path,
    payload_dir: Path,
) -> list[ImageRecord]:
    images: list[tuple[str, int, int]] = []
    for source_path in sorted(source_dir.glob("*.png"), key=lambda path: path.name):
        with Image.open(source_path) as source:
            width, height = source.size
        if source_path.stem not in ROTATING_SPRITES:
            images.append((source_path.stem, width, height))
            continue
        for degrees_256 in range(0, 256, 8):
            images.append((f"{source_path.stem}_{degrees_256:03d}", width, height))

    records: list[ImageRecord] = []
    for index, (name, width, height) in enumerate(
        sorted(images, key=lambda item: item[0])
    ):
        payload_path = payload_dir / f"{name}.rgba2"
        records.append(
            ImageRecord(
                family="sprites",
                name=name,
                buffer_id=SPRITE_BUFFER_BASE + index,
                width=width,
                height=height,
                payload_path=payload_path,
            )
        )
    return records


def _tiles(project_root: Path, payload_dir: Path) -> list[ImageRecord]:
    manifest = project_root / "tiles/dg/dg_0_files.xml"
    root = ET.parse(manifest).getroot()
    if root.tag != "TileSet":
        raise ValueError(f"unexpected tile manifest root: {root.tag}")
    width = int(root.findtext("TileWidth", "0"))
    height = int(root.findtext("TileHeight", "0"))
    declared_base = int(root.findtext("BaseBufferId", "-1"))
    if declared_base != TILE_BUFFER_BASE:
        raise ValueError(
            f"DG tile manifest buffer base is {declared_base}; "
            f"expected {TILE_BUFFER_BASE}"
        )

    tiles = root.find("Tiles")
    if tiles is None:
        raise ValueError("DG tile manifest has no Tiles collection")
    records: list[ImageRecord] = []
    for tile in tiles.findall("Tile"):
        tile_index = int(tile.attrib["index"])
        if tile.text is None:
            raise ValueError(f"DG tile {tile_index} has no source path")
        source_path = project_root / tile.text
        name = f"dg_{tile_index:03d}"
        payload_path = payload_dir / f"{tile_index:03d}.rgba2"
        with Image.open(source_path) as source:
            actual_width, actual_height = source.size
        if (actual_width, actual_height) != (width, height):
            raise ValueError(
                f"DG tile {tile_index} is {actual_width}x{actual_height}; "
                f"expected {width}x{height}"
            )
        records.append(
            ImageRecord(
                family="tiles_dg",
                name=name,
                buffer_id=TILE_BUFFER_BASE + tile_index,
                width=width,
                height=height,
                payload_path=payload_path,
            )
        )
    return records


def build_catalogs(
    project_root: Path,
) -> dict[str, list[ImageRecord]]:
    """Return catalogs over the checked-in, accepted RGBA2222 payloads."""

    payload_root = project_root / "assets/rgba2"
    catalogs = {
        "ui": [
            ImageRecord(
                family="ui", name=entry["name"], buffer_id=entry["buffer_id"],
                width=entry["width"], height=entry["height"],
                payload_path=payload_root / "ui" / f"{entry['name']}.rgba2",
            )
            for entry in ui_manifest(project_root)
        ],
        "bg": _ordinary_family(
            "bg",
            project_root / "tiles/bg/orig",
            payload_root / "bg",
            BACKGROUND_BUFFER_BASE,
        ),
        "sprites": _sprites(
            project_root / "assets/img/orig/sprites",
            payload_root / "sprites",
        ),
        "tiles_dg": _tiles(
            project_root,
            payload_root / "tiles/dg/0",
        ),
        "overlays": _ordinary_family(
            "overlays",
            project_root / "assets/img/proc/overlays",
            payload_root / "overlays",
            OVERLAY_BUFFER_BASE,
        ),
    }
    for family, expected_count in EXPECTED_COUNTS.items():
        actual_count = len(catalogs[family])
        if actual_count != expected_count:
            raise ValueError(
                f"{family} catalog contains {actual_count} records; "
                f"expected {expected_count}"
            )

    all_records = [record for records in catalogs.values() for record in records]
    validate_records(all_records)
    if any(record.buffer_id == TITLE_PHASE_MAP_BUFFER for record in all_records):
        raise ValueError("title lookup scratch buffer collides with an image ID")
    return catalogs


def _asm_name(record: ImageRecord) -> str:
    return "BUF_" + record.name.upper().replace("-", "_")


def render_family_include(family: str, records: list[ImageRecord]) -> str:
    """Render constants-only assembly metadata; runtime filenames are omitted."""

    count_name = {
        "ui": "ui_num_images",
        "bg": "bg_num_images",
        "sprites": "sprites_num_images",
        "tiles_dg": "tiles_dg_num_images",
        "overlays": "overlays_num_images",
    }[family]
    lines = [
        "; Generated by build_current_assets.py; do not edit.\n\n",
        f"{count_name}: equ {len(records)}\n\n",
        "; Explicit resident VDP buffer IDs.\n"
        if any(record.name in DERIVED_TITLE_NAMES for record in records)
        else "; Explicit VDP buffer IDs carried by the matching AGNB records.\n",
    ]
    for record in records:
        lines.append(f"{_asm_name(record)}: equ {record.buffer_id}\n")
    return "".join(lines)


def _write_if_changed(path: Path, data: bytes) -> None:
    if path.is_file() and path.read_bytes() == data:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)


def container_records(
    catalogs: dict[str, list[ImageRecord]],
) -> tuple[list[ImageRecord], list[ImageRecord]]:
    ui_records = [record for record in catalogs["ui"]
                  if record.name not in DERIVED_TITLE_NAMES]
    game_resident = (
        catalogs["bg"]
        + catalogs["sprites"]
        + catalogs["tiles_dg"]
        + catalogs["overlays"]
    )
    game_records = [record for record in game_resident
                    if record.name not in DERIVED_TITLE_NAMES]
    if len(ui_records) != UI_ASSET_COUNT - 1 or len(game_records) != 358:
        raise ValueError("UI/game AGNB record-count contract changed")
    return ui_records, game_records


def title_phase_lut(master: bytes, alternate: bytes) -> bytes:
    """Compile an exact byte remap for VDP buffered Expand Bitmap."""
    if not master or len(master) != len(alternate):
        raise ValueError("title phases must be equally sized and nonempty")
    used: dict[int, int] = {}
    for before, after in zip(master, alternate, strict=True):
        alpha = before & 0xC0
        if alpha not in (0, 0xC0) or alpha != (after & 0xC0):
            raise ValueError("title phases must share a binary opacity mask")
        if alpha == 0 and before != after:
            raise ValueError("title lookup must leave transparent bytes unchanged")
        if used.setdefault(before, after) != after:
            raise ValueError(f"title phase has conflicting mappings for {before:#04x}")
    lookup = bytearray(range(256))
    for before, after in used.items():
        lookup[before] = after
    return bytes(lookup)


def render_title_phase_luts(catalogs: dict[str, list[ImageRecord]]) -> str:
    records = {record.name: record for family in catalogs.values() for record in family}
    lines = ["; Generated by build_current_assets.py; do not edit.",
             f"TITLE_PHASE_MAP_BUFFER: equ {TITLE_PHASE_MAP_BUFFER}"]
    for name in TITLE_PHASES:
        master, alternate = (records[f"{name}_{i}"] for i in (0, 1))
        if (master.width, master.height) != (alternate.width, alternate.height):
            raise ValueError(f"title phases disagree on geometry: {name}")
        lookup = title_phase_lut(master.payload_path.read_bytes(),
                                 alternate.payload_path.read_bytes())
        lines.extend(("", f"{name.upper()}_WIDTH: equ {master.width}",
                      f"{name.upper()}_HEIGHT: equ {master.height}", f"{name}_lut:"))
        for offset in range(0, len(lookup), 16):
            lines.append("    db " + ",".join(f"${entry:02x}" for entry in lookup[offset:offset+16]))
    return "\n".join(lines) + "\n"


def render_container_include(
    ui_records: list[ImageRecord],
    game_records: list[ImageRecord],
) -> str:
    """Publish the same record counts consumed by both container writers."""

    return (
        "; Generated by build_current_assets.py; do not edit.\n\n"
        f"ui_agnb_num_images: equ {len(ui_records)}\n"
        f"game_num_images: equ {len(game_records)}\n"
    )


def generate_outputs(project_root: Path, output_root: Path) -> dict[Path, bytes]:
    """Regenerate all bitmap containers and constants in an isolated directory."""

    catalogs = build_catalogs(project_root)
    ui_records, game_records = container_records(catalogs)
    ui_container = build_container(ui_records)
    game_container = build_container(game_records)
    verify_container(ui_container, ui_records)
    verify_container(game_container, game_records)

    outputs = {
        output_root / "tgt/ui.agnb": ui_container,
        output_root / "tgt/game.agnb": game_container,
        output_root / "src/asm/title_phase_luts.inc": render_title_phase_luts(catalogs).encode(),
        output_root / "src/asm/images_agnb.inc": render_container_include(
            ui_records,
            game_records,
        ).encode(),
    }
    for family, records in catalogs.items():
        outputs[
            output_root / f"src/asm/images_{family}.inc"
        ] = render_family_include(family, records).encode()
    return outputs


def write_current_assets(project_root: Path) -> list[Path]:
    """Generate verified containers and assembly metadata in the repository."""

    prepared = write_ui_art(project_root)
    prepared.extend(write_starfields(project_root))
    prepared.extend(write_legacy_tiles(project_root))
    prepared.extend(write_overlay_art(project_root))
    outputs = generate_outputs(project_root, project_root)
    for path, data in outputs.items():
        _write_if_changed(path, data)
    return sorted([*prepared, *outputs], key=str)


def check_current_assets(project_root: Path) -> list[str]:
    """Return deterministic-output differences without changing the repository."""

    with TemporaryDirectory(prefix="nurples-assets-check-") as temp_dir:
        temp_root = Path(temp_dir)
        generated = generate_outputs(project_root, temp_root)
        differences = check_ui_art(project_root)
        differences.extend(check_starfields(project_root))
        differences.extend(check_legacy_tiles(project_root))
        differences.extend(check_overlay_art(project_root))
        for generated_path, wanted in generated.items():
            relative = generated_path.relative_to(temp_root)
            actual_path = project_root / relative
            if not actual_path.is_file():
                differences.append(f"missing {relative}")
            elif actual_path.read_bytes() != wanted:
                differences.append(f"changed {relative}")
        return differences


def legacy_bitmap_directories(project_root: Path) -> Iterable[Path]:
    """Return the exact former runtime bitmap roots, never a broad target."""

    target = (project_root / "tgt").resolve()
    for relative in ("ui", "bg", "sprites", "tiles"):
        candidate = (target / relative).resolve()
        if candidate.parent != target:
            raise RuntimeError(f"unsafe legacy bitmap target: {candidate}")
        yield candidate
