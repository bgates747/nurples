"""Prepare the four cel UI replacements without changing their runtime ABI."""

from io import BytesIO
import json
from pathlib import Path
from tempfile import TemporaryDirectory

from PIL import Image, ImageChops

from nurples_tools.images import img_to_rgba2, rgba2_to_img


def ui_manifest(project_root: Path) -> list[dict]:
    """Explicit IDs prevent source-art variants from entering the runtime catalog."""
    entries = json.loads((project_root / "build/ui_assets.json").read_text())
    if len(entries) != 46 or sorted(e["buffer_id"] for e in entries) != list(range(46)):
        raise ValueError("UI manifest must retain exactly buffer IDs 0 through 45")
    if len({e["name"] for e in entries}) != 46:
        raise ValueError("duplicate UI asset name")
    return sorted(entries, key=lambda e: e["buffer_id"])


def remove_logo_checkerboard(image: Image.Image) -> Image.Image:
    """Key only the neutral gray backdrop of this specific magenta logo draft.

    Both red and blue exceed green throughout its pink/purple ink, including
    the pale highlights. The generator's neutral checkerboard does not.
    This is a source-specific alpha mask, not an Agon palette algorithm.
    """
    image = image.convert("RGBA")
    red, green, blue, alpha = image.split()
    chroma = ImageChops.subtract(ImageChops.darker(red, blue), green)
    mask = chroma.point(lambda value: 255 if value > 20 else 0)
    image.putalpha(ImageChops.multiply(alpha, mask))
    return image


def prepare_image(source: Path, size: tuple[int, int], *, logo: bool) -> Image.Image:
    with Image.open(source) as original:
        image = original.convert("RGBA")
    if logo:
        image = remove_logo_checkerboard(image)
    # Pillow resamples RGBA using premultiplied alpha, avoiding gray edge bleed.
    image = image.resize(size, Image.Resampling.LANCZOS)
    if logo:
        image.putalpha(image.getchannel("A").point(lambda a: 255 if a >= 128 else 0))
        clean = Image.new("RGBA", size)
        clean.paste(image, mask=image.getchannel("A"))
        image = clean
    else:
        image.putalpha(255)
    return image


def generate_ui_art(project_root: Path) -> dict[Path, bytes]:
    """Return payloads and decoded palette previews; write nothing to the project."""
    outputs = {}
    with TemporaryDirectory(prefix="nurples-ui-art-") as temporary:
        for entry in ui_manifest(project_root):
            if not entry.get("prepare", False):
                continue
            name = entry["name"]
            size = (entry["width"], entry["height"])
            image = prepare_image(
                project_root / entry["source"], size,
                logo=entry.get("remove_gray_background", False),
            )
            raw = Path(temporary) / f"{name}.rgba2"
            # Canonical native conversion: nearest Agon64 RGB, no dithering.
            img_to_rgba2(image, raw, num_colors=64, method="RGB")
            payload = raw.read_bytes()
            if len(payload) != size[0] * size[1]:
                raise ValueError(f"incorrect RGBA2222 size for {name}")
            outputs[Path(f"assets/rgba2/ui/{name}.rgba2")] = payload
            preview = rgba2_to_img(raw, *size)
            png = BytesIO()
            preview.save(png, format="PNG")
            outputs[Path(f"assets/img/proc/ui/{name}.png")] = png.getvalue()
    return outputs


def write_ui_art(project_root: Path) -> list[Path]:
    outputs = generate_ui_art(project_root)
    for relative, data in outputs.items():
        destination = project_root / relative
        if not destination.is_file() or destination.read_bytes() != data:
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(data)
    return [project_root / path for path in outputs]


def check_ui_art(project_root: Path) -> list[str]:
    differences = []
    for relative, expected in generate_ui_art(project_root).items():
        actual = project_root / relative
        if not actual.is_file() or actual.read_bytes() != expected:
            differences.append(f"stale or missing prepared UI asset: {relative}")
    return differences
