# MAP-002 — Rearrange the DG editor tilesheet without changing the game

Owner: Nurples. Smith selected one operation at a time and authorized freezing
this contract, then implementing the tool. Starting art/design checkpoint:
`4859e4205cbef9668b4bfa67dd1c38f039eadadc` on dev.

## Contract

Use the local IDs shown in Tiled: upper left is 0. In today's shared DG
tileset, local 0 is TMX GID 1 and initially corresponds to game byte 1.
TMX empty and the game's fast blank check stay zero. For this tranche use
local IDs 0–254; leave local 255 outside the byte-sized game mapping.

Provide a dry-run-first command for **move A B** (occupied to free) and
**swap A B** (both occupied). Work from the full clean checkout, not the
sparse editor checkout. Inspect the pixels, runtime catalog and references
before declaring a destination free. Read all tracked maps using this TSX,
including design maps and standalone tests. Reject unsupported representations
instead of changing only the references we understand. Rewrite from the
original cells simultaneously, not by two global replacements.

Move the editor artwork and update maps together. Keep each tile's runtime
byte, art buffer, animation and active behavior stable through a persistent
editor-to-runtime binding; teach the current map compiler and legacy
regenerators about that binding. Do not renumber game assets, touch the
assembly, reassign blank, or change the contents of a level. Report the two
IDs, runtime identities and affected map cell counts. Provide a reviewable
all-or-nothing change with a stale-input check and a Git rollback path.

- MAP-002-001 [x] Freeze contract and unchanged input state on origin/dev:
  `dfe71f669557fc97f2b6e1de2eb3a351f23340fb`.

- MAP-002-002 [x] Implement planner, guarded application and build-side
  mapping. With no rearrangement, normal generation must be unchanged.

- MAP-002-003 [x] Exercise move, swap, rejected destinations and preservation
  of compiled map bytes on disposable data. Record results and usage.

- MAP-002-004 [ ] Smith selects moves for a subsequent joint test. No edits
  to live art/maps, sparse checkout sync, emulator, hardware or new enemy
  definitions in this implementation tranche. Commit the tool separately
  so tests can start from a clean rollback point.

Recovery: tracked inputs are pinned; no machine-external state is modified.
Keep future accepted rearrangements in individual commits, reversible with
`git revert` without rewriting development history.

## Usage

From the full checkout on dev, for example:

```bash
.venv/bin/python -m nurples_tools.tile_moves move 0 24
.venv/bin/python -m nurples_tools.tile_moves swap 0 4
```

Each command above is an independent **preview**. The report gives counts in
source/destination order per map. To apply one, rerun that same command with
`--apply TOKEN`, using its printed token. The tool stages the PNG, TSX, changed
TMXs and historical cross-reference CSV together. Inspect `git diff --cached`;
commit an accepted move separately before previewing the next. No auto-commit,
push or Oryx sync. Adding new artwork or assigning new gameplay behavior is
a different operation.

## Implementation checks — 2026-11-22

- In a disposable Git checkout with all 14 tracked maps, moved local 0
  (launchpad) to 24, then swapped 24 with 16 (imported passive tile).
  All 14 maps retained every runtime byte; both campaign and standalone
  assembly includes compiled identically before and after each operation.
  Verified the three pixel blocks, legacy TMX export and regenerated atlas.
- Verified dry runs left no changes; rejected occupied, reserved, empty-source,
  same-ID and out-of-range requests, wrong tokens, dirty input and unsupported
  layer encoding. No actual level, atlas, game byte or Oryx file was edited.
- Baseline caveat: the frozen compiler and updated compiler both produce the
  same campaign include SHA-256
  `ce6766a9e859a5c60028c3ef8e9d1cceb5304d7b339e9c20748b8f0bdeb5f793`.
  The already-tracked include differs in four cells from the campaign TMX,
  so the existing campaign freshness assertion fails even at baseline. Do not
  regenerate it as a side effect of moving tiles.
- Next: Smith selects a local-ID pair for preview and visual review. No
  emulator, hardware or remote design session is needed or held by this task.
