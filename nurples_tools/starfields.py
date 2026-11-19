"""Encode the four original 256x256 background tiles without editing their art."""

from io import BytesIO
from pathlib import Path
from tempfile import TemporaryDirectory

from PIL import Image

from nurples_tools.images import img_to_rgba2, rgba2_to_img


TILE_SIZE = 256
TILE_NAMES = ("bg_stars_0", "bg_stars_3", "bg_stars_4", "bg_stars_6")


def _png(image: Image.Image) -> bytes:
    output = BytesIO()
    image.save(output, format="PNG")
    return output.getvalue()


def generate_starfields(project_root: Path) -> dict[Path, bytes]:
    outputs = {}
    with TemporaryDirectory(prefix="nurples-starfields-") as temporary:
        for name in TILE_NAMES:
            source = project_root / "tiles/bg/orig" / f"{name}.png"
            raw = Path(temporary) / f"{name}.rgba2"
            with Image.open(source) as image:
                if image.size != (TILE_SIZE, TILE_SIZE):
                    raise ValueError(f"{source} must be {TILE_SIZE}x{TILE_SIZE}")
                img_to_rgba2(image, raw, num_colors=64, method="RGB")
            payload = raw.read_bytes()
            if len(payload) != TILE_SIZE * TILE_SIZE:
                raise ValueError(f"incorrect starfield payload size: {name}")
            outputs[Path(f"assets/rgba2/bg/{name}.rgba2")] = payload
            outputs[Path(f"tiles/bg/proc/{name}.png")] = _png(
                rgba2_to_img(raw, TILE_SIZE, TILE_SIZE)
            )
    return outputs


def write_starfields(project_root: Path) -> list[Path]:
    outputs = generate_starfields(project_root)
    for relative, data in outputs.items():
        path = project_root / relative
        if not path.is_file() or path.read_bytes() != data:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
    return [project_root / path for path in outputs]


def check_starfields(project_root: Path) -> list[str]:
    differences = []
    for relative, data in generate_starfields(project_root).items():
        path = project_root / relative
        if not path.is_file() or path.read_bytes() != data:
            differences.append(f"stale or missing starfield: {relative}")
    return differences
