# ART-001-023 — Restore the original starfield artwork

Restore the four starfield tiles from immediately before the sparse atlas
replacement. The paused FAE frame matched a blank band baked into the
replacement art, so this task rolls back only the starfield assets, not the
scrolling code or any other game changes.

## Frozen scope — 2026-11-19

1. Use the last pre-replacement assets (`a2fd5d7^`) as the source of truth.
   Verify that the original PNGs and runtime payloads are 256×256, the
   dimensions expected today. Preserve the exact original pixels and the
   current buffer IDs and asset paths. Stop if restoration needs a change
   beyond the dimensions exception Smith granted.
2. Restore the original sources, previews and packed pixels. Refresh
   `game.agnb` with those four bitmap records. Verify exact equality to the
   historical payloads and that every other record, the game executable,
   and all code stay unchanged.
3. Determine whether normal regeneration honors the originals under the
   existing code. If the atlas generator would replace them again and has
   no data-only opt-out, document the existing pack-only route and the
   limitation; do not edit the generator or its callers in this tranche.
4. Record the verified result here. This freeze is documentation only.
   Do not launch an emulator, deploy, or push as part of this task.
