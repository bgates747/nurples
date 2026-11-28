# EARTH-001 — Formation poses: direct UART versus a C VDPlet on unchanged firmware

Does Nurples miss fewer frame deadlines if the eZ80 sends compact poses and a C VDPlet submits the sprite commands locally? Compare that path against the current direct sender on the same production VDPlet firmware. Either result is useful; less UART traffic is not itself a win.

**Execution contract approved by Smith, November 27.** Nurples owns the fixture and comparison. No firmware or MOS changes, new public functions, Golem, or music work. Smith's rationale: keep the firmware small and avoid exposing sprite functions before there is evidence that an application needs them.

## The two arms

| Arm | What crosses UART for eligible formation poses | What applies the pose |
| --- | --- | --- |
| A — current direct control | Ordinary select-sprite, set-frame and move-absolute VDU commands | The VDP's normal incoming command path |
| B — C VDPlet | A count and six bytes per ship (ID, frame, X, Y), plus the buffer-write and RUN commands | The module expands the records into the same ordered VDU bytes and calls the public `vdu_submit` once per nonempty batch |

B is native C **decoding**, not direct sprite-table access. Its input buffer contains data, not a buffered-command program. Local submission still goes through VDU validation and dispatch. Correct the [existing C fixture](native.c) using the [RUN argument investigation](native-run-investigation.md); do not build a second version of the same module. The rejected R02 RUNs are not a timing baseline.

## Work and acceptance

- EARTH-001-POSE-19 [x] **Firmware service proposal set aside.** Use the existing public interface. No bulk sprite hook, private sprite-table access or firmware request.

- EARTH-001-POSE-20 [ ] **Qualify the corrected client/module pair.** Keep the 193-byte pose data buffer separate from the two-byte RUN descriptor that names it. Check the production capability, validate the whole batch, and release its read lease before submission. Apply the same descriptor correction to the final count check. Reuse evidence from POSE-15–18 rather than duplicating runs.

- EARTH-001-POSE-21 [ ] **Show equivalence before timing.** Require one acknowledged RUN that visibly moves and changes the frame of one ship. Compare B's generated commands with A's for sparse, full and shrinking batches, including the ordering of ordinary sprite operations at batch boundaries. Reconcile batch and pose counts. Stop on the first unexpected rejection or mismatch; no silent fallback to A.

- EARTH-001-POSE-22 [ ] **Score A/B on matched hardware.** Pin both binaries, assets and one unchanged production firmware image. Reset before each arm. Use the mirror-off direct twin and the corrected mirror-off candidate; hold Earth input, eligible actors, refresh points, 256-byte terrain uploads, VBlank policy, tracing and existing eZ80 PT3 playback fixed. Require the same completed updates and pose sequence. Compare skipped / total VBlanks and updates/s for the level and dense formation interval. Report actual pose traffic and sends in total, per update and per second (including B's protocol overhead); record visible judder and scanline faults separately. No production promotion on timing alone.

## What the result can establish

This A/B measures the **whole-game tradeoff**: UART traffic and eZ80 sends change, and B adds RUN dispatch, pose expansion and local submission. It cannot establish that reading commands from a buffer is slower than reading them from the wire. In particular, `vdu_submit` is not a call to execute a stored program via the stock buffered-command API.

If the command-source question remains important, a separate follow-up would replay an identical, prebuilt select/frame/move sequence from the same initial state: send the bytes directly over UART versus execute those exact bytes from a preloaded stock VDP command buffer. Exclude preload from steady-state timing, account for it separately, and measure to the same observable completion point, not just sender return. That follow-up is not a third arm of this task.

**Recovery and stop rules:** Preserve the R02 artifacts under `.build/earth-pose-r02/`, their [binary pins](no-clear-r02.sha256), reports, private assets and existing SD files. Use new output/attempt paths. The frozen source checkpoint plus those retained files is the rollback point. Do not take a timing score without the functional gate above; an unusable bench or a firmware mismatch is not permission to flash or bypass its procedure.
