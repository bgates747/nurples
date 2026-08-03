# Vendored runtime fonts

This directory preserves the project-local font collection and its conversion
metadata. Purple Nurples currently builds and loads only
`Lat38-VGA8_8x8.font`; the other files are inactive remnants of the historical
font-gallery experiment. `nurples_tools/fonts.py` is the authoritative runtime
selection.

The files were copied from the canonical `agon-utils` font-editor example on
2026-08-02. The historical absolute `original_font_path` values in the XML are
provenance only; the Purple Nurples build does not follow or require them.

Build or verify the deployable copies and assembly manifest from the repository
root:

```bash
.venv/bin/python build/scripts/build_fonts.py
.venv/bin/python build/scripts/build_fonts.py --check
```
