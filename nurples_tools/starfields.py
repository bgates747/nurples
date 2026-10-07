"""Slice one sparse atlas into the game's randomly selected background tiles."""

from io import BytesIO
from pathlib import Path
from tempfile import TemporaryDirectory

from PIL import Image

from nurples_tools.images import img_to_rgba2, rgba2_to_img


TILE_SIZE = 256
EDGE_MARGIN = 4
# Dominant background sampled from all four original tiles on main.
BACKGROUND = (0, 0, 85, 255)
# Atlas reading order; the initial screen uses 0 above 4.
TILE_NAMES = ("bg_stars_0", "bg_stars_3", "bg_stars_4", "bg_stars_6")
MASTER = Path("tiles/bg/starfield_cel_master.png")


def slice_starfield(source: Path) -> dict[str, Image.Image]:
    with Image.open(source) as original:
        if original.width != original.height:
            raise ValueError("starfield master must be square")
        atlas = original.convert("RGB").resize(
            (TILE_SIZE * 2, TILE_SIZE * 2), Image.Resampling.LANCZOS,
        )
    tiles = {}
    for index, name in enumerate(TILE_NAMES):
        x, y = (index % 2) * TILE_SIZE, (index // 2) * TILE_SIZE
        tile = atlas.crop((x, y, x + TILE_SIZE, y + TILE_SIZE)).convert("RGBA")
        # Runtime chooses the next tile randomly, not in atlas order. Reserve
        # identical dark-blue borders on every tile, including resampling fringes.
        for box in (
            (0, 0, TILE_SIZE, EDGE_MARGIN),
            (0, TILE_SIZE - EDGE_MARGIN, TILE_SIZE, TILE_SIZE),
            (0, 0, EDGE_MARGIN, TILE_SIZE),
            (TILE_SIZE - EDGE_MARGIN, 0, TILE_SIZE, TILE_SIZE),
        ):
            tile.paste(BACKGROUND, box)
        tiles[name] = tile
    return tiles


def _png(image: Image.Image) -> bytes:
    output = BytesIO()
    image.save(output, format="PNG")
    return output.getvalue()


def generate_starfields(project_root: Path) -> dict[Path, bytes]:
    outputs = {}
    with TemporaryDirectory(prefix="nurples-starfields-") as temporary:
        for name, image in slice_starfield(project_root / MASTER).items():
            outputs[Path(f"tiles/bg/orig/{name}.png")] = _png(image)
            raw = Path(temporary) / f"{name}.rgba2"
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
