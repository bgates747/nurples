"""Generate palette-native vector masters and previews for screen overlays.

The geometry is deliberately expressed as integer rectangles and polygons.  The
SVG masters and PNG previews therefore come from the same source without a font
renderer, antialiaser, palette reduction or image-transform step.
"""

from collections.abc import Iterable
from dataclasses import dataclass
from html import escape
from io import BytesIO
from pathlib import Path
from tempfile import TemporaryDirectory

from PIL import Image, ImageDraw

from nurples_tools.images import img_to_rgba2
from nurples_tools.lode_font import LODE_UPPERCASE_ROWS


RGBA = tuple[int, int, int, int]
Point = tuple[int, int]

TRANSPARENT: RGBA = (0, 0, 0, 0)
BLACK: RGBA = (0, 0, 0, 255)
NAVY: RGBA = (0, 0, 85, 255)
VIOLET: RGBA = (85, 0, 85, 255)
DARK_MAGENTA: RGBA = (170, 0, 170, 255)
MAGENTA: RGBA = (255, 0, 255, 255)
ORANGE: RGBA = (255, 170, 0, 255)
PALE_YELLOW: RGBA = (255, 255, 85, 255)
WHITE: RGBA = (255, 255, 255, 255)


@dataclass(frozen=True)
class Rectangle:
    x: int
    y: int
    width: int
    height: int
    color: RGBA


@dataclass(frozen=True)
class Polygon:
    points: tuple[Point, ...]
    color: RGBA


Shape = Rectangle | Polygon


@dataclass(frozen=True)
class Drawing:
    width: int
    height: int
    shapes: tuple[Shape, ...]


def _text_width(text: str, scale: int) -> int:
    return len(text) * 8 * scale


def _glyph_rectangles(
    text: str,
    x: int,
    y: int,
    scale: int,
    color: RGBA,
) -> list[Rectangle]:
    rectangles = []
    for character_index, character in enumerate(text):
        if character == " ":
            continue
        try:
            rows = LODE_UPPERCASE_ROWS[character]
        except KeyError as error:
            raise ValueError(f"no vector glyph for {character!r}") from error
        glyph_x = x + character_index * 8 * scale
        for row, pixels in enumerate(rows):
            for column in range(8):
                if pixels & (0x80 >> column):
                    rectangles.append(Rectangle(
                        glyph_x + column * scale,
                        y + row * scale,
                        scale,
                        scale,
                        color,
                    ))
    return rectangles


def _display_title(
    text: str, x: int, y: int, scale: int, face: RGBA, shadow: RGBA,
) -> list[Rectangle]:
    shapes = _glyph_rectangles(text, x + 3, y + 3, scale, BLACK)
    shapes.extend(_glyph_rectangles(text, x + 2, y + 2, scale, shadow))
    shapes.extend(_glyph_rectangles(text, x, y, scale, face))
    # A single-pixel palette highlight gives the large blocks cockpit-like trim.
    for rectangle in _glyph_rectangles(text, x, y, scale, WHITE):
        shapes.append(Rectangle(rectangle.x, rectangle.y, 1, 1, rectangle.color))
    return shapes


def game_over_drawing(frame: int) -> Drawing:
    """Return one of the two deliberately alternating title treatments."""
    if frame not in (0, 1):
        raise ValueError("GAME OVER frame must be 0 or 1")
    width, height = 240, 64
    text = "GAME OVER"
    scale = 3
    x = (width - _text_width(text, scale)) // 2
    y = 20
    face = PALE_YELLOW if frame == 0 else MAGENTA
    accent = MAGENTA if frame == 0 else ORANGE
    shadow = DARK_MAGENTA if frame == 0 else VIOLET
    shapes: list[Shape] = [
        Polygon(((3, 7), (37, 7), (45, 11), (37, 15), (3, 15), (11, 11)), accent),
        Polygon(((237, 7), (203, 7), (195, 11), (203, 15), (237, 15), (229, 11)), accent),
        Polygon(((3, 57), (37, 57), (45, 53), (37, 49), (3, 49), (11, 53)), accent),
        Polygon(((237, 57), (203, 57), (195, 53), (203, 49), (237, 49), (229, 53)), accent),
    ]
    shapes.extend(_display_title(text, x, y, scale, face, shadow))
    return Drawing(width, height, tuple(shapes))


def loading_title_drawing(frame: int) -> Drawing:
    """Two fixed-footprint color phases of the Mode 20 loading title."""
    if frame not in (0, 1):
        raise ValueError("Loading title frame must be 0 or 1")
    width, height, scale = 240, 76, 4
    face, shadow = (PALE_YELLOW, DARK_MAGENTA) if frame == 0 else (MAGENTA, VIOLET)
    shapes = []
    for word, y in (("PURPLE", 2), ("NURPLES", 39)):
        x = (width - _text_width(word, scale)) // 2
        shapes.extend(_display_title(word, x, y, scale, face, shadow))
    return Drawing(width, height, tuple(shapes))


def prompt_drawing(text: str) -> Drawing:
    width, height = 208, 16
    scale = 1
    text_width = _text_width(text, scale)
    if text_width > width:
        raise ValueError(f"prompt is {text_width} pixels wide; maximum is {width}")
    x = (width - text_width) // 2
    shapes: list[Shape] = []
    shapes.extend(_glyph_rectangles(text, x + 1, 5, scale, NAVY))
    shapes.extend(_glyph_rectangles(text, x, 4, scale, WHITE))
    shapes.extend((
        Rectangle(x - 4, 7, 2, 1, ORANGE),
        Rectangle(x + text_width + 2, 7, 2, 1, ORANGE),
    ))
    return Drawing(width, height, tuple(shapes))


def overlay_drawings() -> dict[str, Drawing]:
    return {
        "game_over_0": game_over_drawing(0),
        "game_over_1": game_over_drawing(1),
        "press_any_key_to_continue": prompt_drawing("PRESS ANY KEY TO CONTINUE"),
        "press_any_key_to_start": prompt_drawing("PRESS ANY KEY TO START"),
    }


def _svg_color(color: RGBA) -> str:
    return f"#{color[0]:02x}{color[1]:02x}{color[2]:02x}"


def drawing_to_svg(drawing: Drawing, title: str) -> bytes:
    lines = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        (f'<svg xmlns="http://www.w3.org/2000/svg" width="{drawing.width}" '
         f'height="{drawing.height}" viewBox="0 0 {drawing.width} {drawing.height}" '
         'shape-rendering="crispEdges">'),
        f"  <title>{escape(title)}</title>",
    ]
    for shape in drawing.shapes:
        if isinstance(shape, Rectangle):
            lines.append(
                f'  <rect x="{shape.x}" y="{shape.y}" width="{shape.width}" '
                f'height="{shape.height}" fill="{_svg_color(shape.color)}"/>'
            )
        else:
            points = " ".join(f"{x},{y}" for x, y in shape.points)
            lines.append(
                f'  <polygon points="{points}" fill="{_svg_color(shape.color)}"/>'
            )
    lines.append("</svg>")
    return ("\n".join(lines) + "\n").encode()


def drawing_to_image(drawing: Drawing) -> Image.Image:
    image = Image.new("RGBA", (drawing.width, drawing.height), TRANSPARENT)
    painter = ImageDraw.Draw(image)
    for shape in drawing.shapes:
        if isinstance(shape, Rectangle):
            painter.rectangle(
                (shape.x, shape.y, shape.x + shape.width - 1, shape.y + shape.height - 1),
                fill=shape.color,
            )
        else:
            painter.polygon(shape.points, fill=shape.color)
    return image


def drawing_to_png(drawing: Drawing) -> bytes:
    image = drawing_to_image(drawing)
    output = BytesIO()
    image.save(output, format="PNG", optimize=False)
    return output.getvalue()


def drawing_to_rgba2(drawing: Drawing) -> bytes:
    """Encode exact source pixels through the canonical agonutils adapter."""
    with TemporaryDirectory(prefix="nurples-overlay-rgba2-") as temp_dir:
        output = Path(temp_dir) / "overlay.rgba2"
        img_to_rgba2(
            drawing_to_image(drawing),
            output,
            num_colors=64,
            method="RGB",
        )
        return output.read_bytes()


def generate_overlay_art() -> dict[Path, bytes]:
    outputs = {}
    for name, drawing in overlay_drawings().items():
        outputs[Path(f"assets/vector/overlays/{name}.svg")] = drawing_to_svg(
            drawing, name.replace("_", " ").upper()
        )
        outputs[Path(f"assets/img/proc/overlays/{name}.png")] = drawing_to_png(drawing)
        outputs[Path(f"assets/rgba2/overlays/{name}.rgba2")] = drawing_to_rgba2(
            drawing
        )
    return outputs


def write_overlay_art(project_root: Path) -> list[Path]:
    outputs = generate_overlay_art()
    for relative, data in outputs.items():
        destination = project_root / relative
        if not destination.is_file() or destination.read_bytes() != data:
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(data)
    return [project_root / path for path in outputs]


def check_overlay_art(project_root: Path) -> list[str]:
    differences = []
    for relative, expected in generate_overlay_art().items():
        actual = project_root / relative
        if not actual.is_file() or actual.read_bytes() != expected:
            differences.append(f"stale or missing overlay asset: {relative}")
    return differences


def palette_colors(drawings: Iterable[Drawing]) -> set[RGBA]:
    """Return all non-transparent source colors for validation and reporting."""
    return {shape.color for drawing in drawings for shape in drawing.shapes}
