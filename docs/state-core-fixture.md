# State coordinator core fixture

`ASM-001B` is qualified by a self-contained eZ80 program at
`tests/asm/state_core_fixture.asm`. It executes the same generic dispatcher and
legal-edge graph that the replacement game scaffold includes; the fixture does
not copy either implementation.

The fixture runs in ADL mode from `$040000`, establishes SPL at `$05FF00` before
its first call or push, uses only CPU and zero-filled RAM, and reports success
by writing guest status `$2A` to debug port `$00`. It has no MOS, VDP, SD-card,
timer, or keyboard dependency. Generous instruction and cycle ceilings are
watchdogs, not timing assertions.

Run the durable qualification from the repository root:

```bash
.venv/bin/python tests/run_state_core_fixture.py --repeat 20
```

The runner assembles to a temporary directory and invokes the committed
`agon-headless-test run-direct` interface. `AGON_HEADLESS_TEST` or
`--headless-repo` may select another Headless checkout; `--runner` may select a
prebuilt executable. No output is deployed to an emulator profile or hardware.

## Shared implementation boundary

- `src/asm/state_core.inc` owns validated numeric dispatch, transition commit,
  event snapshots, policy lookup, reentrancy guards, and actor-controller
  primitives.
- `src/asm/state_graph.inc` owns the canonical Purple Nurples global legal-edge
  graph.
- `src/asm/state_new.inc` supplies the currently inert game lifecycle
  callbacks, immutable descriptor/policy tables, and reference player class.
- The fixture supplies trace-only callbacks and synthetic actor classes, then
  includes the same core and graph files.

Both the game scaffold and fixture use ez80asm fail-build span guards for every
global descriptor table, policy table, actor registry, actor record, and actor
descriptor table. The core also checks the global immutable table spans before
the first callback lookup.

`state_new_process_init` is a fresh-process bootstrap, not a callback-safe
public operation. It deliberately clears retained global and actor dispatch
guards before rebuilding the coordinator after MOS `RUN`; ordinary code must
use the guarded init/tick/render interfaces and must never invoke the process
bootstrap from a callback.

## Deterministic coverage

The fixture independently classifies all 196 `(current, requested)` global
state pairs. The frozen request semantics contain 47 committed non-self edges,
14 accepted self no-ops, and 135 illegal requests. EXIT is terminal; FAULT may
advance only to EXIT. A same-state request is accepted before consulting the
edge graph and does not claim the first-request slot.

Additional cases cover:

- poisoned process memory followed by deterministic BOOT initialization;
- invalid current, requested target, and corrupted pending target recovery;
- first-request-wins conflicts and duplicate requests;
- event snapshotting when UPDATE posts an event for the following tick;
- QUIT arbitration without running the obsolete state's UPDATE callback;
- nested global, actor, and cross-domain dispatch rejection;
- actor reconciliation before initial ENTER, recovery ENTER, or UPDATE;
- reconcile-driven transitions skipping UPDATE until the next actor tick;
- local request conflict/duplicate behavior and transition scratch cleanup;
- invalid local IDs, valid cross-class corruption, invalid classes, and
  zero-count class quarantine without an unchecked indirect callback.

Callbacks deliberately poison general registers where useful. Persistent
expectations live in explicitly sized bytes or 24-bit `dl` fields so the tests
also exercise ADL register-width and stack-balance assumptions.
