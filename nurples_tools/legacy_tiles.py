"""Prepare the passive artwork imported from the original Z80 Nurples."""

from __future__ import annotations

from io import BytesIO
from pathlib import Path
from tempfile import TemporaryDirectory

from PIL import Image

from nurples_tools.images import convert_to_agon_palette, img_to_rgba2


# Runtime tile IDs 1 through 16 are owned by modern active-tile behavior.
# These five IDs are the first passive slots and therefore cannot accidentally
# enter an active-tile callback.  Tiled GID N occupies atlas cell N - 1.
LEGACY_PASSIVE_TILES = (
    (17, "tile_blank.png"),
    (18, "tile_circle.png"),
    (19, "tile_conduit_cross.png"),
    (20, "tile_conduit_straight.png"),
    (21, "tile_conduit_straight_rot.png"),
)


def _png_bytes(image: Image.Image) -> bytes:
    output = BytesIO()
    image.save(output, format="PNG")
    return output.getvalue()


def generate_legacy_tile_outputs(
    project_root: Path,
    output_root: Path,
) -> dict[Path, bytes]:
    """Return the imported runtime tiles, payloads, and updated Tiled atlas."""

    source_root = project_root / "tiles/dg/legacy_z80"
    atlas_path = project_root / "tiles/dg/dg0.png"
    with Image.open(atlas_path) as source_atlas:
        atlas = source_atlas.convert("RGBA").copy()
    if atlas.size != (256, 256):
        raise ValueError(f"DG atlas is {atlas.size}; expected 256x256")

    outputs: dict[Path, bytes] = {}
    with TemporaryDirectory(prefix="nurples-legacy-tiles-") as temp_dir:
        temporary_root = Path(temp_dir)
        for tile_id, source_name in LEGACY_PASSIVE_TILES:
            source_path = source_root / source_name
            if not source_path.is_file():
                raise FileNotFoundError(f"missing original Z80 tile: {source_path}")
            with Image.open(source_path) as source:
                if source.size != (16, 16):
                    raise ValueError(
                        f"original Z80 tile {source_name} is {source.size}; "
                        "expected 16x16"
                    )
                converted = convert_to_agon_palette(
                    source.convert("RGBA"),
                    64,
                    "RGB",
                )

            png_data = _png_bytes(converted)
            tile_name = f"{tile_id:03d}"
            outputs[output_root / f"tiles/dg/0/{tile_name}.png"] = png_data

            rgba_path = temporary_root / f"{tile_name}.rgba2"
            img_to_rgba2(converted, rgba_path, num_colors=64, method="RGB")
            outputs[
                output_root / f"assets/rgba2/tiles/dg/0/{tile_name}.rgba2"
            ] = rgba_path.read_bytes()

            atlas_index = tile_id - 1
            atlas_x = (atlas_index % 16) * 16
            atlas_y = (atlas_index // 16) * 16
            atlas.paste(converted, (atlas_x, atlas_y), converted)

    outputs[output_root / "tiles/dg/dg0.png"] = _png_bytes(atlas)
    return outputs


def _write_if_changed(path: Path, data: bytes) -> None:
    if path.is_file() and path.read_bytes() == data:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)


def write_legacy_tiles(project_root: Path) -> list[Path]:
    """Prepare the checked-in runtime forms of the original passive artwork."""

    outputs = generate_legacy_tile_outputs(project_root, project_root)
    for path, data in outputs.items():
        _write_if_changed(path, data)
    return sorted(outputs)


def check_legacy_tiles(project_root: Path) -> list[str]:
    """Return deterministic-output drift for the imported passive artwork."""

    with TemporaryDirectory(prefix="nurples-legacy-tiles-check-") as temp_dir:
        temp_root = Path(temp_dir)
        generated = generate_legacy_tile_outputs(project_root, temp_root)
        differences: list[str] = []
        for generated_path, wanted in generated.items():
            relative = generated_path.relative_to(temp_root)
            actual_path = project_root / relative
            if not actual_path.is_file():
                differences.append(f"missing {relative}")
            elif actual_path.read_bytes() != wanted:
                differences.append(f"changed {relative}")
        return differences
