"""Pillow adapters around the canonical native :mod:`agonutils` API.

Palette conversion and RGBA2222 encoding deliberately stay in the canonical
extension.  This module only bridges its path-based API to build stages that
already hold Pillow images and supplies the raw RGBA8888 writer that agonutils
1.1 does not expose.
"""

from pathlib import Path
from tempfile import TemporaryDirectory

import agonutils
from PIL import Image


PROJECT_ROOT = Path(__file__).resolve().parents[1]
PALETTE_DIR = PROJECT_ROOT / "build" / "palettes"


def _palette_path(num_colors: int) -> Path:
    if num_colors not in {16, 64}:
        raise ValueError("num_colors must be 16 or 64")

    palette_path = PALETTE_DIR / f"Agon{num_colors}.gpl"
    if not palette_path.is_file():
        raise FileNotFoundError(f"Agon palette not found: {palette_path}")
    return palette_path


def _transparent_color(color: tuple[int, ...] | None) -> tuple[int, ...] | None:
    if color is None:
        return None
    if len(color) == 3:
        return (*color, 255)
    if len(color) == 4:
        return color
    raise ValueError("transparent_color must contain three or four channels")


def convert_to_agon_palette(
    image: Image.Image,
    num_colors: int,
    method: str,
    transparent_color: tuple[int, ...] | None = None,
) -> Image.Image:
    """Return a Pillow copy converted by canonical ``agonutils``."""

    with TemporaryDirectory(prefix="nurples-palette-") as temp_dir:
        source_path = Path(temp_dir) / "source.png"
        target_path = Path(temp_dir) / "target.png"
        image.convert("RGBA").save(source_path)
        agonutils.convert_to_palette(
            str(source_path),
            str(target_path),
            str(_palette_path(num_colors)),
            method,
            _transparent_color(transparent_color),
        )
        with Image.open(target_path) as converted:
            return converted.copy()


def img_to_rgba2(
    image: Image.Image,
    filepath: str | Path,
    *,
    num_colors: int = 64,
    method: str = "RGB",
    transparent_color: tuple[int, ...] | None = None,
) -> None:
    """Encode a Pillow image as RGBA2222 using canonical ``agonutils``."""

    with TemporaryDirectory(prefix="nurples-rgba2-") as temp_dir:
        source_path = Path(temp_dir) / "source.png"
        image.convert("RGBA").save(source_path)
        agonutils.img_to_rgba2(
            str(source_path),
            str(filepath),
            str(_palette_path(num_colors)),
            method,
            _transparent_color(transparent_color),
        )


def img_to_rgba8(image: Image.Image, filepath: str | Path) -> None:
    """Write one RGBA8888 byte quartet per pixel in row-major order."""

    Path(filepath).write_bytes(image.convert("RGBA").tobytes())


def rgba8_to_img(filepath: str | Path, width: int, height: int) -> Image.Image:
    """Decode RGBA8888 through canonical ``agonutils`` into Pillow."""

    source_path = Path(filepath)
    payload_size = source_path.stat().st_size
    expected_size = width * height * 4
    if payload_size != expected_size:
        raise ValueError(
            f"RGBA8888 payload is {payload_size} bytes; expected {expected_size}"
        )

    with TemporaryDirectory(prefix="nurples-rgba8-read-") as temp_dir:
        target_path = Path(temp_dir) / "target.png"
        agonutils.rgba8_to_img(
            str(source_path),
            str(target_path),
            width,
            height,
        )
        with Image.open(target_path) as decoded:
            return decoded.copy()


def rgba2_to_img(filepath: str | Path, width: int, height: int) -> Image.Image:
    """Decode RGBA2222 through canonical ``agonutils`` and return Pillow data."""

    with TemporaryDirectory(prefix="nurples-rgba2-read-") as temp_dir:
        target_path = Path(temp_dir) / "target.png"
        agonutils.rgba2_to_img(str(filepath), str(target_path), width, height)
        with Image.open(target_path) as decoded:
            return decoded.copy()
