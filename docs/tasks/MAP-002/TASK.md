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

- MAP-002-001 [ ] Freeze contract and unchanged input state on origin/dev.

- MAP-002-002 [ ] Implement planner, guarded application and build-side
  mapping. With no rearrangement, normal generation must be unchanged.

- MAP-002-003 [ ] Exercise move, swap, rejected destinations and preservation
  of compiled map bytes on disposable data. Record results and usage.

- MAP-002-004 [ ] Smith selects moves for a subsequent joint test. No edits
  to live art/maps, sparse checkout sync, emulator, hardware or new enemy
  definitions in this implementation tranche. Commit the tool separately
  so tests can start from a clean rollback point.

Recovery: tracked inputs are pinned; no machine-external state is modified.
Keep future accepted rearrangements in individual commits, reversible with
`git revert` without rewriting development history.
