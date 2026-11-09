# Lode Runner display font

Purple Nurples now has a reproducible 8x8 adaptation of Lode Runner's original
status lettering.  The A-Z shapes are exact cropped game pixels rather than a
lookalike font or a resampled screenshot.  The generated font keeps the proven
Lat38 digits, punctuation, symbols, and non-Latin cells, so it remains useful
for ordinary game diagnostics and loading text.

## Provenance

The source is the original running-game RAM sprite table at `$AD00`, extracted
by the `agon-6502` project from its preserved Lode Runner disk and verified
against the annotated reverse engineering at
<https://github.com/XekriRedmane/lode_runner_reveng/blob/main/main.nw>.

1. Original RAM image SHA256:
   `58628155177584fbd1b9f15f3ffa46b064b783efd6781d1704aa653f6f4f1dfa`.
2. Verified 10x11 Agon source font SHA256:
   `1b46afdc483af1a704348884c51bddaaffb67f72d3e94a1abb912945fe9b3fc2`.
3. Nurples fallback font SHA256:
   `23321abb10437db6b8bc8f21907268b46615d409b38975f6a195eb9643065a69`.

The 10x11 source stores three blank rows above eight rows of artwork.  Every
uppercase letter has zero pixels in the source's second storage byte.  Removing
the blank rows therefore produces an exact 8x8 glyph: no interpolation,
thresholding, palette conversion, or hand redrawing occurs.

## Runtime adaptation

`nurples_tools.lode_font` writes the exact A-Z rows into both the uppercase and
lowercase ASCII slots of a copy of `Lat38-VGA8_8x8.font`.  Existing mixed-case
strings consequently display in the desired all-caps face without adding a
runtime case-conversion loop.  All other 204 glyphs stay byte-for-byte Lat38.

Generate or verify the artifact with the project environment:

```bash
.venv/bin/python build/scripts/build_lode_font.py
.venv/bin/python build/scripts/build_lode_font.py --check
```

The overlay generator consumes the same eight source rows.  Its SVG masters
use one rectangle per source pixel and its PNG previews use exact Agon64
colours with binary alpha.  Enlarged text uses integer rectangles equivalent
to nearest-neighbour scaling; bilinear, bicubic and quadratic filters are not
part of this path.

The generated font is intentionally not the selected runtime font yet.  That
selection and the hardware-sprite overlays remain separately gated on visual
review, resource-lifecycle qualification, and the current gameplay candidate.
