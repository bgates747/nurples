# EARTH-001-POSE-12 — Where does the direct Earth update cross VBlank?

Build a diagnostic twin of the R03 comparison's mirror-off **direct** game. Record which eZ80 operation straddles a serviced VBlank, not which call merely discovers it later. This is an attribution fixture, not a new speed score.

- EARTH-001-POSE-23 [ ] Add fixture-only fences around the 256-byte row upload, the background bitmap draw, SD refill, scrolling, actor work and the interleaved sprite pose sends. Keep tile/other work and waiting distinguishable. Reuse the MOS epoch, pending-claim boundary and EPS1 per-update workload. Do not add another interrupt, change the music, reorder work, batch sends or defer any pose.

- EARTH-001-POSE-24 [ ] Write a separate bounded report on normal fixture completion. Decode both the first edge (one update opportunity) and additional edges (skips); distinguish reaching the wait on time from finding an already-pending edge. Report all updates and dense updates (at least eight eligible poses on that update). Flag unavailable or unresolvable attribution, never silently count it as actor work. Timing includes interrupts and observer overhead, not VDP completion time.

- EARTH-001-POSE-25 [ ] Verify report reconciliation and boundary cases, assemble outside all existing fixture directories, and verify the uninstrumented input against the retained binary before adding probes. Preserve the direct control, R03 candidate, production code paths and existing reports. Report the new binary and decoder; hardware measurement is a separate run, not a result inferred from the build.

Smith approved this scope November 27. Baseline: `.build/earth-pose-r02/native-direct/nurples.bin`, SHA-256 `1e9c3aed753566a4f94fd2717266d079a1f36fa51d8b41c4b6c0216a0cb29177`; R03 used those exact bytes. Keep the local binary, symbols and private assets in place for rollback. Do not publish the executable.
