# MAP-004 — Level design migration — approved

Rollback point: `7c0f65ed362dbe8565a02a6caf4f3eb2e0773d43` on dev.
Move files first, normalize authoring second. Neither step should change
the campaign order, artwork, spawn timing, tile IDs, music, or gameplay.

Owner: Nurples. Approved and frozen at `2cd93861eda19b48a3650ed7e3125cd660d567f7`.

- MAP-004-001 [x] **Inventory and baseline.** With the Oryx editor closed, compare its
   sparse checkout to the checkpoint again; save any new edits before sync.
   Record TMX → TSX → image dependencies, project paths, art provenance,
   map compilers, music mappings, fixture scripts and sparse patterns.
   Capture each campaign level's compiled rows, tile payloads, both Earth
   spawn layouts, and reference Tiled renders. Keep private build outputs
   local. Record existing test failures separately.

- MAP-004-002 [x] **Move without converting.** Place Earth City in `levels/01/` with
   `level.tmx`, `staggered.tmx`, and one project. Move its shared art to
   `assets/` as specified in the sibling README. Move the DG maps into
   named, unassigned directories until their story slots are confirmed;
   keep the current three campaign entries playable and in order. Share
   one DG tileset and atlas. Preserve working source art, licenses and
   catalog metadata; do not deduplicate on filenames alone. Leave unrelated
   art studies and historical fixtures in place unless the inventory
   identifies a dependency. Update relative references and build consumers
   together, including path-keyed music choices and staggered staging.

- MAP-004-003 [x] **Prove the move.** From a clean checkout build the ordinary campaign
   and both aligned and staggered fixtures. Compare compiled level rows,
   packed image payloads, music selections and Tiled renders to the baseline.
   Exercise tile-move tooling on a disposable copy. Checkpoint this pass
   separately before changing any layer semantics.

- MAP-004-004 [ ] **Normalize authoring.** Supply a copyable per-level template and use
   the conventions in the README. Separate invisible spawners from visible
   structures at compile time. Preserve visible pads, destructible terrain,
   electrode links and activation rows; do not mechanically move every
   active DG tile into Spawns. Test an overlapping marker/structure cell.
   If any case needs new runtime semantics, stop and review it separately.

- MAP-004-005 [ ] **Validate and hand off.** Check that references exist on a case-sensitive
   filesystem, are relative, and stay inside the owning level or shared art.
   Reject cross-level links and dependencies on the template. Refresh Oryx
   sparse patterns without discarding local edits. Open Earth and a DG
   project independently in system Tiled; compare renders, then test Earth
   into DG in FAE. No physical-card deployment as part of the migration.

**Proposed for review:** assign only Earth a story number now; keep the proving
ground, modern DG map and recovered Z80 map named until their slots are
decided. This preserves their existing runtime indices without treating
test order as story design.
