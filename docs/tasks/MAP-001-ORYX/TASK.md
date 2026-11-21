# MAP-001 — Level-design workspace on Oryx

Give Smith a small, editable checkout of the tracked Nurples maps and artwork
on Oryx, with Tiled available locally. The production repository, emulator
profiles, game binary and existing map format stay untouched by setup.

- MAP-001-01 [x] Freeze and push this plan on `dev` before installation or
  staging. Oryx Manager owns installing Tiled and verifying the editor can
  open the project's format; Nurples owns the checkout and content checks.
- MAP-001-02 [x] Create a separate sparse clone at
  `~/Agon/mystuff/nurples-level-design`, on local `dev` tracking
  `origin/dev`. Refuse a pre-existing destination rather than overwriting
  work. Include production and fixture level sources, shared tilesets,
  tile catalogs, and browsable source/reference art; omit runtime/build
  trees and unrelated project infrastructure.
- MAP-001-03 [x] Verify the checked-out commit, sparse paths, clean status,
  campaign inputs and actual map → tileset → artwork dependencies. Provide
  the editor command, main maps and a short guide to art worth browsing.
- MAP-001-04 [x] Record Oryx Manager's installation/version and editor smoke
  result separately from Git/data verification. Do not alter Smith's maps
  during the smoke test; no emulator or hardware reservation required.

Design option, not yet approved for implementation: use a distinct enemy
sprite as each spawner's Tiled brush while rendering the normal launch-pad
bitmap/animation in-game. Keep spawner identity separate from its runtime
appearance; review the art and choose the mapping before changing compiler,
tile IDs, placement rules or game actors.

## 2026-11-21 — Ready for level design

The plan was published at `9a1783c2dcd8379fcf179172bd5470486d6147b0`
before setup. The new independent clone tracks `origin/dev`. It includes
`maps/`, `tests/maps/`, `tiles/`, `assets/img/` and `assets/vector/`;
the build, assembly, runtime and emulator directories are absent. The old
tracked Tiled session is excluded so it does not reopen unrelated files.

Nurples verified 216 input hashes on Oryx, including all 14 tracked TMX
maps, their referenced atlas and the catalog artwork. Both campaign maps
rendered successfully from the sparse checkout. Oryx Manager installed
Tiled (Ubuntu package 1.11.90-1; executable reports 1.11.0) and checked
editor export and rasterization on a temporary copy: the modern map
retained its tile layer and tileset reference and rendered at 256×4096.
Input hashes were unchanged. No GUI session or emulator was started.

[Launch instructions, sample maps and art index](../../../maps/README.md).
No campaign, game code, tileset or spawner mapping was modified.
