# Vendored runtime fonts

This directory is the authoritative, project-local source for the 29 font
binaries loaded by Purple Nurples. Each `.font.xml` file records the dimensions
and historical conversion settings for its corresponding `.font` file.

The files were copied from the canonical `agon-utils` font-editor example on
2026-08-02. They were byte-identical to the files already tracked under
`tgt/fonts`. The historical absolute `original_font_path` values in the XML are
provenance only; the Purple Nurples build does not follow or require them.

Build or verify the deployable copies and assembly manifest from the repository
root:

```bash
.venv/bin/python build/scripts/build_fonts.py
.venv/bin/python build/scripts/build_fonts.py --check
```
