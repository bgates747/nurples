# BENCH-001 — Canonical Olimex bench documentation

Nurples bench guidance now delegates exclusively to the maintained Extender procedure. Conflicting local instructions are removed; historical firmware and application compatibility evidence is retained.

## Contract and plan

Smith requested this cleanup on 2026-11-17. Owner: Nurples. Scope: Markdown documentation in canonical nurples-repair on dev. Deliverables: this task silo, canonical links, removal of contrary procedures, and verification. No hardware access or game changes. No commit or push requested.

Sole authority: [/Agon/mystuff/agon-extender/docs/operations/olimex-bench/README.md](/Agon/mystuff/agon-extender/docs/operations/olimex-bench/README.md), including its linked supporting guides. Local documents do not own bench scheduling, preparation, flashing, recovery, restoration, or check-in procedures.

- BENCH-001-001 [x] Inventory local documentation and read the canonical procedure. Reviewed 32 Markdown files through targeted searches and pertinent sections.

- BENCH-001-002 [x] Remove conflicting instructions in AGENTS.md, TODO.md, and the November 8 firmware audit; replace with canonical links.

- BENCH-001-003 [x] Verify final diff and repeat obsolete-procedure searches. Targeted Markdown search returned no old scheduling path, exact-standard-selector instruction, physical-lease instruction, scheduler standard-preparation instruction, or local bench-safety ownership. Git diff whitespace checks passed for all three edited existing files.

## Record

Starting state: dev, ahead 12, with substantial unrelated uncommitted game work preserved. Plan recorded before procedural edits. Removed local ownership of bench safety guidance, selector/lease directions, and scheduler-owned preparation. Historical hashes, measurements and application ABI restrictions remain evidence. Recovery: revert only this task's documentation diff, preserving existing overlapping edits.
