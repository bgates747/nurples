# Why the Earth native pose run failed

The November 27 Olimex run did not benchmark successful native rendering.
Nurples passed a 193-byte buffer as the RUN argument. Production VDPlet
v0.1.4-alpha rejects any argument buffer larger than 32 bytes **before calling
the module**. Even a batch containing one ship uses the same 193-byte backing
buffer. Investigate this client/host mismatch first; only measure speed once
the intended sprites are actually being drawn.

## What we know

- The R02 guest loaded module build `4e505331` as handle 1. Its serial trace
  then recorded 1,930 `op=2 status=1` errors and hit the runner's 64 KiB
  capture limit. The game subsequently returned to MOS according to Smith.
  No validated native score; the three cost fixtures never ran.
- In [the release source](https://github.com/bgates747/agon-vdplet/blob/77ad7a1d28c0d44fd2cd2cf4aa4cded10f770485/src/vdplet_runtime.h#L470),
  dispatch checks the size of the **selected buffer**, not the byte count in
  the latest update. The host test called the module entry point directly
  with `argument_bytes=193`, so it tested the code *after* this gate.
  This identifies a sufficient cause of rejection, not the cause of every
  visible scanout error.
- Smith saw frequent top-line faults on the mirror-off direct control and
  full-screen judder plus top-line faults on native. Direct validated at
  551 skipped / 5,095 VBlanks (4,544 updates, 53.5 updates/s). Keep the
  visual observations separate from the game clock. Error logging is extra
  workload; neither its cost nor scanout timing was instrumented here.
- Evidence on Pi5: `.build/earth-pose-r02/bench-uart-01/run-native/results/vdp-usb.bin`
  (SHA-256 `5f599dcfb10107092064228acee1c6f2f4e1604c0357b33385f5754774d1d138`),
  adjacent phase log and failure receipt, and `run-native-direct/validated.json`.
  Exact image pins: [R02 packet](olimex-suite-r02.json), production identity in
  `bench-uart-01/r02-production.json`. Module SHA-256:
  `de1f52eb22639b47c8bd705e228e63a2d5cf842b241045d524c09de8e3406962`.

## Investigation plan (proposed; no new run or firmware change)

- EARTH-001-POSE-15 [ ] **Confirm the ABI with VDPlet.** Trace the linked
  guest's first batch through wire decode, handle, entry and argument
  allocation. Ask the runtime owner for a known-good game using small RUN
  arguments to reference resident bulk data, and confirm buffer ownership,
  read leases, release rules, and the sprite-submit capabilities of this
  exact production build. Document what that game offloads and how often it
  invokes the module; do not compare FPS across unrelated games.

  **Runtime owner confirmation (November 27):** Both Nurples entries pass
  `0x2B01` directly as the RUN argument. The runtime checks its full
  193-byte allocation before entry. The published ABI supports a separate
  descriptor; no firmware change is needed. Rally uses a two-byte buffer ID
  to reach 310 bytes of state ([client](../../../../../AgonArcade/docs/tasks/RALLY-28/custom-vdp/prepared-bitmap-set/runtime/source/native.c),
  [emulator results](../../../../../AgonArcade/docs/tasks/RALLY-28/custom-vdp/prepared-bitmap-set/runtime/RESULTS.md)).
  The SDK's [vdu_plot example](../../../../../agon-vdplet/examples/vdu_plot/vdu_plot.c)
  shows the same indirection with command and result IDs.

- EARTH-001-POSE-16 [ ] **Prove one successful submission offline.** Keep
  the 193-byte pose table in its existing data buffer. Pass a separate,
  single-block two-byte RUN argument containing its little-endian ID to both
  entries. Require exactly two argument bytes; read and release the descriptor,
  then acquire the payload read-only for that entry. Release it before
  submitting sprite commands; retain no borrowed pointer between calls.
  Preserve the live profile-8 capability probe. Check both normal
  submission and final reconciliation. Test the actual host dispatch limit
  (32 accepted for dispatch, 33 and 193 rejected before entry), not just
  a mock that calls the module directly. Compare emitted ID, frame, position
  and order against direct for one ship, sparse ranks, maximum batch and
  shrinking batches. Exercise malformed counts and stale handles.

- EARTH-001-POSE-17 [ ] **Fail visibly on the first bad request.** For a
  separately approved smoke test, use an acknowledged first pose with a
  bounded wait and retain the first status, handle, buffer IDs and lengths.
  Require a known visible move, an incremented module submission count and
  the expected pose count. Stop on rejection. Do not raise the serial
  cap, silence the evidence or issue thousands of RUNs to diagnose one.

- EARTH-001-POSE-18 [ ] **Only then measure performance.** On unchanged
  production firmware compare a fresh-reset native build with its mirror-off
  direct twin. Keep terrain, music, input, drawing and game state matched.
  Reconcile per-update workload, game/serial totals and module counts;
  require zero rejected submissions. Compare full-level and dense skips /
  VBlanks, updates/s, pose bytes and sends per update and per second. Log
  visible top-line faults and whole-screen judder separately. If still slow,
  isolate invocation overhead from state expansion and sprite submission
  before proposing a firmware optimization.
