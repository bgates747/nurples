"""Build the exact 8x8 Lode Runner/Nurples hybrid display font.

The uppercase rows were cropped without resampling from the verified 10x11
Lode Runner status font: the original three blank rows are omitted and its
remaining eight rows fit in one byte.  Nurples' Lat38 font supplies every
other glyph so existing digits, punctuation, and symbols remain usable.
"""

from __future__ import annotations

from hashlib import sha256
from io import BytesIO
from pathlib import Path

from PIL import Image


BASE_FONT_NAME = "Lat38-VGA8_8x8.font"
BASE_FONT_SHA256 = "23321abb10437db6b8bc8f21907268b46615d409b38975f6a195eb9643065a69"
OUTPUT_FONT_NAME = "LodeRunner-Hybrid-8x8.font"
LODE_SOURCE_FONT_SHA256 = (
    "1b46afdc483af1a704348884c51bddaaffb67f72d3e94a1abb912945fe9b3fc2"
)
LODE_SOURCE_RAM_SHA256 = (
    "58628155177584fbd1b9f15f3ffa46b064b783efd6781d1704aa653f6f4f1dfa"
)

# Exact first-byte rows 3..10 from LodeRunner-10x11.font.  The verified
# uppercase glyphs have no set pixels in their second storage byte, so this is
# a crop from 10x11 to 8x8, not a resize or an artistic reconstruction.
LODE_UPPERCASE_ROWS: dict[str, tuple[int, ...]] = {
    "A": (0x1F, 0x11, 0x11, 0x7F, 0x41, 0x41, 0x47, 0x47),
    "B": (0x7C, 0x44, 0x44, 0x7F, 0x41, 0x41, 0x41, 0x7F),
    "C": (0x7F, 0x41, 0x40, 0x40, 0x70, 0x70, 0x71, 0x7F),
    "D": (0x7C, 0x41, 0x41, 0x41, 0x71, 0x71, 0x71, 0x7C),
    "E": (0x7F, 0x70, 0x70, 0x7C, 0x40, 0x40, 0x40, 0x7F),
    "F": (0x7F, 0x70, 0x70, 0x7C, 0x40, 0x40, 0x40, 0x40),
    "G": (0x7F, 0x41, 0x40, 0x40, 0x47, 0x47, 0x41, 0x7F),
    "H": (0x41, 0x41, 0x41, 0x7F, 0x71, 0x71, 0x71, 0x71),
    "I": (0x10, 0x10, 0x10, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C),
    "J": (0x04, 0x04, 0x04, 0x07, 0x07, 0x07, 0x47, 0x7F),
    "K": (0x41, 0x47, 0x44, 0x7C, 0x7F, 0x71, 0x71, 0x71),
    "L": (0x40, 0x40, 0x40, 0x40, 0x70, 0x70, 0x70, 0x7F),
    "M": (0x41, 0x71, 0x7F, 0x7F, 0x41, 0x41, 0x41, 0x41),
    "N": (0x41, 0x41, 0x71, 0x7F, 0x7F, 0x47, 0x41, 0x41),
    "O": (0x7F, 0x47, 0x47, 0x41, 0x41, 0x41, 0x41, 0x7F),
    "P": (0x7F, 0x41, 0x41, 0x7F, 0x70, 0x70, 0x70, 0x70),
    "Q": (0x7F, 0x47, 0x47, 0x41, 0x41, 0x41, 0x44, 0x71),
    "R": (0x7F, 0x41, 0x41, 0x7F, 0x7C, 0x7C, 0x71, 0x71),
    "S": (0x7F, 0x41, 0x40, 0x7F, 0x07, 0x07, 0x47, 0x7F),
    "T": (0x7F, 0x10, 0x10, 0x10, 0x1C, 0x1C, 0x1C, 0x1C),
    "U": (0x41, 0x41, 0x41, 0x41, 0x71, 0x71, 0x71, 0x7F),
    "V": (0x71, 0x71, 0x71, 0x71, 0x71, 0x7F, 0x1C, 0x10),
    "W": (0x41, 0x41, 0x41, 0x41, 0x7F, 0x7F, 0x71, 0x41),
    "X": (0x41, 0x41, 0x41, 0x1C, 0x1C, 0x41, 0x41, 0x41),
    "Y": (0x47, 0x47, 0x47, 0x7F, 0x1C, 0x1C, 0x1C, 0x1C),
    "Z": (0x7F, 0x41, 0x41, 0x04, 0x1C, 0x40, 0x47, 0x7F),
}


def build_hybrid_font(base_font: bytes) -> bytes:
    """Return a 256-glyph font with exact Lode letter rows in both cases."""
    if len(base_font) != 256 * 8:
        raise ValueError(f"expected a 2048-byte 8x8 base font, got {len(base_font)}")
    actual_hash = sha256(base_font).hexdigest()
    if actual_hash != BASE_FONT_SHA256:
        raise ValueError(f"unexpected {BASE_FONT_NAME} SHA256: {actual_hash}")

    output = bytearray(base_font)
    for uppercase, rows in LODE_UPPERCASE_ROWS.items():
        for character in (uppercase, uppercase.lower()):
            offset = ord(character) * 8
            output[offset:offset + 8] = bytes(rows)
    return bytes(output)


def render_font_atlas(font: bytes) -> bytes:
    """Render a lossless 16x16 glyph atlas for inspection."""
    if len(font) != 256 * 8:
        raise ValueError("an 8x8 font must contain exactly 2048 bytes")
    image = Image.new("RGBA", (128, 128), (0, 0, 0, 255))
    for code in range(256):
        glyph_x = (code % 16) * 8
        glyph_y = (code // 16) * 8
        for row, bits in enumerate(font[code * 8:code * 8 + 8]):
            for column in range(8):
                if bits & (0x80 >> column):
                    image.putpixel(
                        (glyph_x + column, glyph_y + row),
                        (255, 255, 255, 255),
                    )
    output = BytesIO()
    image.save(output, format="PNG", optimize=False)
    return output.getvalue()


def font_metadata() -> bytes:
    """Return deterministic font-editor metadata for the generated binary."""
    return b"""<?xml version="1.0" ?>
<settings>
  <setting name="original_font_path" value="LodeRunner-Hybrid-8x8.font"/>
  <setting name="font_name" value="Lode Runner Hybrid"/>
  <setting name="font_variant" value="Exact uppercase; Lat38 fallback"/>
  <setting name="font_width" value="8"/>
  <setting name="font_height" value="8"/>
  <setting name="font_width_mod" value="8"/>
  <setting name="font_height_mod" value="8"/>
  <setting name="point_size" value="8"/>
  <setting name="offset_left" value="0"/>
  <setting name="offset_top" value="0"/>
  <setting name="offset_width" value="0"/>
  <setting name="offset_height" value="0"/>
  <setting name="scale_width" value="0"/>
  <setting name="scale_height" value="0"/>
  <setting name="raster_type" value="threshold"/>
  <setting name="threshold" value="128"/>
  <setting name="palette" value="Agon64"/>
  <setting name="fg_color" value="255, 255, 255, 255"/>
  <setting name="bg_color" value="0, 0, 0, 255"/>
  <setting name="ascii_start" value="0"/>
  <setting name="ascii_end" value="255"/>
  <setting name="chars_per_row" value="16"/>
</settings>
"""


def generate_lode_font(project_root: Path) -> dict[Path, bytes]:
    base = (project_root / "assets/fonts" / BASE_FONT_NAME).read_bytes()
    font = build_hybrid_font(base)
    return {
        Path("assets/fonts") / OUTPUT_FONT_NAME: font,
        Path("assets/fonts") / f"{OUTPUT_FONT_NAME}.xml": font_metadata(),
        Path("assets/img/proc/fonts/lode_runner_hybrid_8x8.png"):
            render_font_atlas(font),
    }


def write_lode_font(project_root: Path) -> list[Path]:
    outputs = generate_lode_font(project_root)
    for relative, payload in outputs.items():
        destination = project_root / relative
        if not destination.is_file() or destination.read_bytes() != payload:
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(payload)
    return [project_root / relative for relative in outputs]


def check_lode_font(project_root: Path) -> list[str]:
    differences = []
    for relative, expected in generate_lode_font(project_root).items():
        actual = project_root / relative
        if not actual.is_file() or actual.read_bytes() != expected:
            differences.append(f"stale or missing Lode font asset: {relative}")
    return differences
