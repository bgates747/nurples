# EARTH-001-POSE-12 — Where does the direct Earth update cross VBlank?

Build a diagnostic twin of the R03 comparison's mirror-off **direct** game. Record which eZ80 operation straddles a serviced VBlank, not which call merely discovers it later. This is an attribution fixture, not a new speed score.

- EARTH-001-POSE-23 [x] Add fixture-only fences around the 256-byte row upload, the background bitmap draw, SD refill, scrolling, actor work and the interleaved sprite pose sends. Keep tile/other work and waiting distinguishable. Reuse the MOS epoch, pending-claim boundary and EPS1 per-update workload. Do not add another interrupt, change the music, reorder work, batch sends or defer any pose.

- EARTH-001-POSE-24 [x] Write a separate bounded report on normal fixture completion. Decode both the first edge (one update opportunity) and additional edges (skips); distinguish reaching the wait on time from finding an already-pending edge. Report all updates and dense updates (at least eight eligible poses on that update). Flag unavailable or unresolvable attribution, never silently count it as actor work. Timing includes interrupts and observer overhead, not VDP completion time.

- EARTH-001-POSE-25 [x] Verify report reconciliation and boundary cases, assemble outside all existing fixture directories, and verify the uninstrumented input against the retained binary before adding probes. Preserve the direct control, R03 candidate, production code paths and existing reports. Report the new binary and decoder; hardware measurement is a separate run, not a result inferred from the build.

Smith approved this scope November 27. Baseline: `.build/earth-pose-r02/native-direct/nurples.bin`, SHA-256 `1e9c3aed753566a4f94fd2717266d079a1f36fa51d8b41c4b6c0216a0cb29177`; R03 used those exact bytes. Keep the local binary, symbols and private assets in place for rollback. Do not publish the executable.

### Direct deadline observer R01

Build with `.venv/bin/python build/scripts/build_00_all_the_things.py --earth-deadlines`.
The builder assembles and hashes the uninstrumented direct twin **before** installing the probes.
Private output: `.build/earth-deadlines-r01/nurples.bin`, 144,408 bytes, SHA-256
`67167f2a67adf43f1e9b9422bf67f1a00dfb061b1b4c2b719e051f5ce8efc090`.
Both generated AGNBs matched the direct fixture byte-for-byte. No new data need staging on the target.
The game still reads the shared control asset paths. Use a fresh report workspace;
the existing create-new writer intentionally won't overwrite an earlier `DEADLINE.BIN`.
Physical run completed on Olimex November 28; results and reset incident below.

Decode a collected report (the control argument and individual late-update listing are optional):

```sh
.venv/bin/python -m nurples_tools.earth_deadlines DEADLINE.BIN --list-dense-misses \
  --control .build/earth-pose-r03/bench-uart-01/run-native-direct/results/POSE.BIN
```

- **First vs. additional edge:** one edge between claims is one normal update opportunity. The decoder reports the first edge on all updates, the first edge on late updates, and separately the additional edges that reconcile to skipped/VBlanks. A pending claim is not inherently a skip. `outer_wait` includes the pending probe or forced-fresh path, not exclusively polling. `carry_in` belongs to work between the previous exact claim and the new frame classification.
- **Exclusive scopes:** background upload is the 267-byte synchronous MOS output (11-byte header plus one 256-byte row); draw is the select/plot calls for streamed rows, pinned Earth strips and starfields; scroll is the viewport scroll call. The SD scope is the 1 KiB read. Ordinary terrain plotting, row admission and viewport setup in the tile pipeline stay distinct. Actor work includes player, enemy, explosion and active-tile routines, including their non-pose I/O. Sprite select, frame and position sends inside the ordinary helpers are separately tagged at MOS, regardless of actor class; pose preparation stays with its caller. HUD and unscoped work are not billed to actors.
- **Readout:** all, dense (at least eight eligible formation poses) and light cohorts; counts, skip rate and skips per elapsed second at 60 Hz. The optional list identifies the one-based update, phase sequence, pending flag, eligible poses/sends and SD refill timing. The MOS phase indicates where the eZ80 was when the **serviced** tick changed, not which earlier operation exhausted the budget. IRQ time and probe overhead remain included. Always compare the aggregate to uninstrumented direct; the optional control check requires the same refill and pose/send sequence on every update.
- **Format and bounds:** EDL1 keeps the 33-byte EPS1 header and its six workload bytes per update, then three edge tags and one flags byte. For a full 4,544-update run the file is 45,473 bytes; storage is capped at 5,000 updates and nothing is written to SD during play. More than three edges still reconcile; additional unplaceable edges are reported as `unresolved`. An odd intermediate clock delta invalidates attribution for that row; invalid aggregate clocks, truncated reports or non-reconciling totals are rejected.
- **Verified offline:** linked code with a synthetic MOS epoch exercised all phase types, nested actor/send, real pose packet bytes and counts through 32 poses, carry-in, 24-bit wrap, pending and fresh waits, a tick after claim, and excess/invalid ticks. The tests also verify preserved MOS return flags and trace writes. None of those synthetic ticks is hardware timing evidence.
- **Regression checks:** all 13 Earth-specific Python tests passed, as did the linked eZ80 check and lint. Full suite: 261 tests, 20 skipped, same five unrelated assertion failures as the [previous control](RESULTS.md) (one actor-default inventory assertion and four starfield hashes).

### November 28 — reset incident and completed run

The UART stage succeeded. Before the game was invoked, the mainboard reset advanced the boot ID from 3560887823 to 3560887824, but the 60-second keyboard readiness check expired with `ready:false`. Smith saw an autoexec.txt line 2 error followed by a volume timeout. He suspects line 2 selects the Extender keyboard; neither the directive nor the root cause was verified by the runner. Another mainboard-only reset showed the same symptom. Smith then reset P4, waited for it to come up, reset the mainboard, and confirmed a remote DIR worked. We resumed from the staged file without another reset or transfer.

- EARTH-001-POSE-26 [ ] Diagnose the post-reset remote keyboard failure separately from game timing. With the reservation retained and MOS responsive, arm timestamped passive CPU/VDP boot output and Extender keyboard/link status capture before one mainboard-only reset. Record boot/session IDs, first ready transition or timeout and exact startup error/order. Check what line 2 actually commands against the preserved startup file. If reproduced, preserve the failing state and logs before any P4 reset; if not, record non-reproduction. Do not change startup or firmware to induce it.

Evidence root: `.build/earth-deadlines-r01/bench-01/`. Initial failure: `reset-direct/before.json`, `suite.log` and its terminal receipt. Successful continuation: `postreset.log`, `run-direct/results/DEADLINE.BIN`, `run-direct/validated.json`, and `handback/handback.json`. CLI and keyboard responsive afterward; scheduler reservation deliberately retained.

The workload matched R03 on every update. Instrumented: 549/5,093 skipped/VBlanks overall; 122/758 across 636 dense updates (control: 555/5,099 and 127/763 respectively). Of the 122 additional dense edges, 71 occurred in pose sends, 28 in actor work, 21 in other work and two in the tile pipeline. First edges on those updates: 81 upload, 30 tile pipeline, six carry-in, two scroll, two refill, one background draw. The most common pair was upload → pose send (39). This attributes serviced edges, not exclusive blame or time spent per operation.

- EARTH-001-POSE-27 [ ] Smith requested a matched Olimex arm: 256-byte sequential SD reads instead of four rows per 1 KiB read. Prime one row at open, then send the current row before fetching the next; no read beyond the last streamed row or during pinned terrain/warp. Preserve the exact EDL1 observer, 267-byte VDP packet, game, music and control assets. Keep the instrumented 1 KiB run as reference. Require identical per-update eligible poses and sends, 3,583 measured refills at the expected positions (plus the initial prime), and reconciled report/serial counts. Compare overall/dense misses, phases and SD latency, retaining the bench reservation.
