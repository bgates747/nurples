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

## Result

Frozen at `aa62f1fdd1fc86592d2fa1d66032467e056a08fd` before restoring assets.
The exact source was `0c741d5e2021927adfa440f45b8e04b950114889`
(`a2fd5d7^`). All four original source and preview PNGs are 256×256.
The four 65,536-byte RGBA2222 payloads match that revision's AGNB records
byte for byte, with their original IDs (1024–1027) unchanged.

The local `tgt/game.agnb` now contains those originals. It is 406,256
bytes, SHA-256
`3701ecee9f67d13056471d513731f6402a30e8ce0f1ff71d44a8be62d3d7c4f6`.
The independent parser verified all 359 records: only the four starfield
payloads changed; the other 355 records, all metadata, the executable,
its symbols and `ui.agnb` were unchanged. No code, deployment, or emulator
changes were made. Visual confirmation is pending.

### Initial build limitation (superseded below)

Use the restored `tgt/game.agnb` as-is. An assembly-only build keeps it.
To verify or repack from existing RGBA2222, use the existing
`nurples_tools.current_assets.generate_outputs(project_root, output_root)`
API and select its `tgt/game.agnb` result. It does not generate artwork;
read back and verify the result before replacing the deployable container.

Do not run `--assets`, `--all`, `build_current_assets.py` in write mode,
or `build_starfields.py` in write mode for this rollback. They regenerate
the replacement tiles from the sparse atlas. There is no existing config
switch to opt out. The read-only full asset check reports exactly twelve
expected differences (four source PNGs, four preview PNGs, four payloads).
The AGNB catalog/payload comparison passed; its atlas determinism test
and the sparse-art density test fail because they still require the
replacement artwork. Re-enabling full regeneration for the original tiles
requires a separate build-tool change; it was not made in this tranche.

## Full-rebuild amendment — 2026-11-19

Smith clarified that the rollback includes the normal full rebuild, not
just a repack. Update only the necessary asset tooling and tests so
`build_00_all_the_things.py --all` regenerates the original starfields
and its normal checks pass. Preserve exact original runtime pixels, IDs,
and dimensions. Keep the sparse atlas as archived art and leave all
gameplay and assembly source unchanged. Compare the four rebuilt records
against the historical source of truth and the remaining records and
executable against the pre-rebuild baseline. No deployment or push.

### Full-rebuild result

Amendment frozen at `3c97457762aba475772c80170fb1a0d4c79c382c`.
The build now reads the four original PNGs in `tiles/bg/orig`, rejects
dimensions other than 256×256, and writes only encoded payloads and
decoded previews. The archived sparse atlas is no longer a build input.
A clean-generation regression uses the four source PNGs alone and checks
their historical runtime hashes and that the sources were not edited.

`build_00_all_the_things.py --all` completed asset generation, the normal
asset checks, and native assembly under Pi compiler checkout 1054 (released).
The rebuilt game container matched the verified rollback candidate exactly:
the four original payloads and all other records were identical to that
candidate. The UI container, executable, symbols, original PNGs, and all
assembly source were unchanged. Ruff and diff checks passed; 216 tests ran
with one headless-input test skipped because its runner was not installed.
No deployment or visual test was performed.

### Oryx comparison — Author requested

Stage the rebuilt original-starfield bundle in separate stopped FAE and
Alexe profiles, using one ordinary Oryx checkout. Reuse each engine's last
qualified runtime/firmware (stock VDP for FAE; pinned VDPlet for Alexe),
verify pinned Eirene RC3 and use the same raw card contents in both.
Preserve the donors, verify the complete guest payload and load-only
startup, then hand over launch commands. Smith runs the two profiles one
at a time and can use `P` to pause for seam screenshots. No commit or push
of emulator changes ahead of visual review.

Staged stopped on Oryx under checkout 520 (request `a47fe97bf577`).
The [readback receipt](../../../.build/art-001/original-starfield-rollback/oryx-r01/staging-receipt.txt)
verifies both raw cards are byte-identical (SHA-256 `fda305f5…0005c`)
and match the rebuilt game binary, assets, font, and load-only startup.
FAE uses stock FAE 1.2.6 VDP; Alexe uses the same VDPlet firmware image as
the earlier Alexe R03 review (`dc43ecb8…33ab`). Both use pinned Eirene RC3.
No emulator process was running at handoff. Smith launches
`./fab-agon-emulator` in `original-starfield-fae-r01` and
`./flexe-agon-emulator` in `original-starfield-alexe-r01` under the
Oryx project’s `.emulator` directory, then types `RUN` at each MOS prompt.
Close the first before opening the second. Hold Oryx until review and
emulator shutdown are confirmed.
