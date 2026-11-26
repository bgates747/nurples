# EARTH-001 research note: let VBlank release the next game update

**November 2026 review; subsequent bounded fixture implementation recorded below.**
The actual VBlank interrupt already publishes a frame clock. Nurples currently
forgets an edge that arrives while it is doing a frame and waits for a fresh
one. First try remembering the last edge consumed, with exactly one game
update at a time in foreground. Do not move the game, music decoder, disk
reads or VDU writes into the interrupt. No new sprite architecture, private
VDP operation, or MOS firmware change is necessary to test the idea.

## Handoff: which run and what we learned

Based on dev `af46123` **plus the uncommitted** Earth 256-byte async and wait
instrumentation. Build selector `--earth-jit-256`; private output
`.build/earth-jit-256/nurples.bin`. Wait-timed binary SHA-256
`37b4a19d71a2d39c5c95eb2822ee83c09209db7156b1ce369a36942abbc9d221`.
The async fixture requires the shared Eirene RC3 / EMOS UART0 ABI; the
proposed scheduler itself need not depend on that ABI.

| Hardware observation | Skipped / delivered VBlanks | Interpretation |
| --- | ---: | --- |
| 1 KiB push, terrain scrolling on | 915 / 5,239 | 17.5% |
| 256-byte push | 768 / 5,120 | 15.0%; Smith saw most skips with the full formation firing |
| 256-byte push, timed, full first level, music and all drawing on | 824 / 5,233 | 4,409 updates, zero zero-tick waits; 35,847 ms in outer VBlank waits |

At nominal 60 Hz the timed interval was ~87.22 s: 8.13 ms/update in
the wait and ~11.65 ms/update elsewhere. The wait includes music service
and interrupt handling, not just idle CPU. No per-frame distribution or
pose timing was measured. Background off looked smooth to Smith. The runs
were not identical input traces; neither CPU saturation nor a UART stall is
proven. The report-file write failed; the counters above are from the screen.

## Findings in the code actually exercised

- **Video clock:** Stock MOS and Eirene bind GPIO PB1 (vector 0x32) to the
  VBlank ISR. It acknowledges the GPIO and increments the MOS clock by two.
  EMOS additionally calls its keyboard service there. Sources: stock
  `agon-mos/src/interrupts.asm:_vblank_handler`, pinned Eirene RC3 package
  `source/src/main.c` and `source/src/interrupts.asm`, and
  `agon-emos-uart0/src/interrupts.asm:_vblank_handler`. The MOS sysvar is an
  interrupt-published counter, not a VDP command round trip. It measures
  *delivered* edges, not scanout timing. The RC3 UART driver can delay IRQs
  while handling CTS with interrupts masked (RC3 package README).
- **Current pacing:** [main loop](../../../src/asm/nurples.asm) runs
  pause/input, timestamp, state and music before calling
  [vdu_vblank](../../../src/asm/vdu.inc). The wait samples the clock at
  **entry**, then spins for a change or fails open on a poll limit. Example
  at 60 Hz: enter the wait at 17 ms after a frame began and the 16.7 ms tick
  is already history; it normally waits until 33.3 ms. That is distinct
  from an update whose work really takes two full periods.
- **Game ordering is observable:** ordinary play scrolls and admits a tile
  row before [game_update_live_world](../../../src/asm/state_game_playing.inc).
  It runs player, enemies, explosions and active tiles, applies pending death
  *after* the tile pass, and restores origin for HUD drawing. Warp calls the same
  world update. Splitting or re-entering this sequence risks repeating row
  events, a partial electrical fence, or drawing with the HUD viewport.
- **Pose work is not one end-of-frame flush.** [move_sprite_group](../../../src/asm/sprites.inc)
  runs each behavior then directly selects the sprite and frame and moves it;
  new sprites have a separate show/publication pass. For a normal visible
  actor that is three synchronous submissions, 4 + 4 + 7 bytes. Twelve such
  actors alone are 180 bytes, ~1.60 ms on the wire at 1.125 Mbaud 8N1,
  before missiles, terrain, HUD or music. That is a wire floor, not measured
  CPU time. [VDU helpers](../../../src/asm/vdu_sprites.inc) patch shared command
  templates in RAM. Do not re-enter them or hand their mutable storage to
  asynchronous TX. Later, if measuring warrants it, separate pose construction
  from submission and test immutable command batches. See the isolated
  [UART-001 trace fixture](../../../tests/asm/uart001_full_frame.inc) for
  precedent, not a drop-in renderer.
- **Music is already interrupt-scheduled but foreground-serviced.** PRT1's
  [handler](../../../src/asm/prt-loader.inc) only posts a 50 Hz deadline.
  Foreground sends at most one prepared packet per service, tracks lateness
  and missed periods, then prepares the next. [Build hooks](../../../nurples_tools/loading_music.py)
  call `lp_poll` during the wait and at safe game boundaries. Removing the
  wait without replacing that service would tie idle-time music to frames.
  `mus_frame` is not an idle callback: it also advances frame-specific
  audio/transition behavior. P freezes playback; M stops PRT1. Do not treat
  `lp_irqs != lp_seen` alone as runnable work during silent/paused states.
- **Earth transfer:** [earth_batch.inc](../../../src/asm/earth_batch.inc)
  sends 11 header + 256 data bytes per scrolling frame. The first row of
  each four-row group overlaps its transfer with the next 1 KiB SD read
  when there is one. Before drawing, foreground joins the source-release
  callback and waits for TX-empty. Neither event proves VDP execution. Keep
  this path identical for the scheduler comparison. Scheduling does not make
  the serial link faster.

## Suggested structure; not an implementation contract

1. Cache the MOS sysvar pointer and keep a foreground-owned
   `last_claimed_clock`. At a safe boundary read the low 24 bits with a
   single ADL load. Compute unsigned modular delta from last claimed; two
   units are one delivered edge. With pending edges, claim the *latest*
   snapshot **before** starting work. Perform one ordinary update. If two
   or more edges accumulated, count all but one as coalesced, do not run a
   burst of historical collision/scroll/pose passes. An edge arriving during
   the update stays visible to the next dispatch; do not reset from the
   clock at frame end. This targets the unnecessary extra wait, not time
   already spent on an overloaded frame.
2. With no frame due, service due music in foreground at a complete-command
   boundary. The eventual idle path can sleep until an interrupt rather
   than busy-wait. Use a **lost-wakeup-safe** check: briefly mask interrupts,
   recheck frame and *serviceable* music conditions; if both clear,
   adjacent `EI; HALT`, otherwise re-enable and dispatch. UART/PRT1 wakes
   are not frames; recheck on every wake. Never hold DI for VDU, MOS calls,
   disk, decode or actor work. See the [eZ80 CPU manual](https://www.zilog.com/docs/UM0077.pdf)
   on EI deferral and HALT. No naked HALT as a replacement for the old
   bounded wait: when music is off and VBlank disappears it may never wake.
   For a clean first A/B retain bounded, music-servicing idle polling with
   the new claim semantics; qualify interrupt sleep and an independent timeout
   separately. Alternatively prove the sleep/watchdog pair before including
   it. Do not use the stopped music timer or the VBlank clock to watchdog itself.
3. Keep input, game logic, music phase transitions and telemetry single-owner.
   A *frame opportunity* still samples controls when paused; it does not
   advance the world. Preserve ESC while paused, test D's edge, P/M behavior,
   `timestamp_rebase` on resume, and the terminal/no-fictitious-frame
   [telemetry contract](../../runtime-telemetry.md). Keep wall-clock timers and
   per-update motion distinct. Seed/rebase the scheduling epoch explicitly
   after long loading, level setup, or in-process restart: don't inherit credit
   from an inactive phase. Never rebase every ordinary game/warp frame.
4. Prefer the stock ISR publishing the counter over an app-owned vector.
   If precise **IRQ-entry timestamps** are later needed, a separate fixture
   could install a minimal register-preserving marker that tail-jumps to the
   original handler and restores the exact vector on every exit. Do not omit
   the MOS GPIO acknowledgment/clock or EMOS keyboard service. Do not invoke
   the whole frame in interrupt context: even an ISR waiting for its own UART
   completion can deadlock, and permitting recursive frames would corrupt
   game state and command buffers.

**Next review decision for Smith:** test only the claim policy on the existing
256-byte workload first, or qualify interrupt sleep in that same experiment?
Suggested first scope: normal play and its shared warp, with explicit entry/
exit handoffs; keep blocking asset loading and display initialization out.
Update the *staged* pace and PRT2 probes at the scheduler boundary (the build
patches the original call site). Define comparable measured phases and first/
last samples. Record delivered edges, claimed updates, coalesced edges, time
spent waiting and music misses. Validate late-by-one versus multiple-edge
overrun, P/M/ESC and restart, no repeated spawns, and no change to the Earth
assets. Use FAE for behavior and Olimex for performance; keep the timed
baseline binary separately before rebuilding. No decision yet to buffer
poses, move any Earth upload to idle, or change production pacing.

### Follow-up: play/warp claim audition built

Smith selected the first option: no interrupt sleep and no upload, pose, or
music changes. The 256-byte fixture claims the newest pending edge at the end
of a play/warp step without waiting for another. A bypass increments
`Immediate play/warp starts` only when the successor update actually begins,
not when ESC exits or P pauses before dispatch. Nonplay, pause and level setup
retain a fresh wait. The prior timed binary and symbols are preserved under
`.build/earth-jit-256/rollback-37b4a19d/`. Replacement binary in the usual
directory: SHA-256 `81e3e721f8fd6bf887bc7dd125e4e8e0084d7e40a266e8cfab59f91e153c778a`.
Asset hashes unchanged. Linked CPU tests covered immediate and waited paths,
coalescing, wrap, timeout/music polling and excluded states. Await hardware
result; compare immediate starts, skipped/VBlanks and wait milliseconds to
the timed control above. No production change or card transfer by Nurples.

Hardware result: full first level, music and all drawing on. 392 skipped /
4,919 VBlanks; 4,527 updates, zero zero-tick waits; 2,008 immediate starts;
23,617 ms waiting. The skip rate fell from 15.75% to 7.97% across the two
playthroughs, not identical input traces. Maximum SD read 1,177 counts
(16.35 ms); this peak does not identify the cause of the remaining misses.
Report-file write still failed; values transcribed from the exit screen.

### SD comparison with Jukebox (research only)

Compared the spectrum-enabled AgonJukebox skins worktree at `685dfdc`
(`src/asm/play.inc:184`, `audio_service.inc:46`, `docs/pcm-spectrum.md`).
It too keeps an open FIL and uses blocking `ffs_fread`. At 65,535 Hz mono
8-bit PCM it requests 1,092 or 1,093 bytes per 60 Hz service tick. The VDP
plays one approximately one-second sample while the other buffer fills.
Frequency analysis is optional, sliced and droppable. This demonstrates
sustained throughput, not a worst-case 16.67 ms display deadline.

Examined the actual 256-byte fixture: pixel DATA starts at byte 72 of
`earth.agnb`, so **every** 1,024-byte read starts 72 bytes into a 512-byte
sector. EMOS reference (`agon-emos-uart0/src_fatfs/ff.c:f_read`, tiny FatFS)
copies a 440-byte head and 72-byte tail via its window, with just the middle
512 bytes going directly to the destination. The tail sector can be reused:
three sectors touched does not imply three fresh SD reads. Aligned data can
avoid both partial copies, but this driver still issues one CMD17 per sector.
Separate Extender storage-only benchmark: 178.08 KiB/s for 1,020-byte calls;
it was not a measurement of this card/game. The Earth SD timer includes any
UART/other interrupts that preempt the read. Turning background off skips
upload/plot, not the read, so SD alone does not explain that visual A/B.

Proposed first test: sector-align the pixel payload in a private stream,
keeping 1 KiB reads, 256-byte sends, pending-VBlank claim, music and same
art. The existing AGNB reader expects a fixed chunk order; do not insert
unrecognized padding. Compare missed edges on refill vs non-refill updates
and the distribution of refill times. Later independent test: use aligned
512-byte reads on alternate scrolling updates or a deeper prepared-row
reserve; extra buffers do not make a blocking SD call asynchronous.

### Olimex A/B: change only the offset of the Earth pixel data

Smith approved the AGNB 0.4 reader for the existing `--earth-jit-256`
fixture. The 0.1 control starts DATA at byte 72; the 0.4 file starts it at
512. Both have the same 917,504 pixel bytes. Use one executable for both
runs: keep the 1 KiB reads, 256-byte async writes, pending-VBlank policy,
music, formation and timing counters unchanged. No production API change.

- EARTH-001-ALIGN-01 [x] Verify the AGNB handoff against the current control.
  Control SHA-256: `abf3e8355be3e90eca77c8dd22606d26626e8f64c6e0204743325c9a2cae35ee`;
  aligned: `13be5a719c649e0852e7148a57cb653da7748752a549e652f07bfe679217292b`.
  Reader source SHA-256: `c41c3db717c533a28faf1616ddf293dcd1a303cbfbbff45bc4f07068a38ec6b5`.
  This is **local AGNB source**, not a published 0.4 commit.

- EARTH-001-ALIGN-02 [x] Built in `.build/earth-jit-256/` using the fixture's
  native Pi assembler in `.build/f40-tools/`. New `nurples.bin` SHA-256:
  `cf4e31e5320d8d829e802e7187db0f3cdca83f99725aa95424820a661c7346f5`.
  Control: `earth.agnb`; candidate: `aligned/earth.agnb`. Previous binary
  and symbols preserved in `rollback-81e3e721/`; resident bank unchanged.
  The linked reader, with FatFS calls stubbed, reached offsets 72 and 512
  and fetched the correct first 1 KiB from each. The complete pixel payloads
  compare equal. Linked pending-VBlank regression and Earth pack tests pass.
  No hardware run or transfer by Nurples.

- EARTH-001-ALIGN-03 [ ] Smith/Sprite: run on Olimex with the control and
  aligned containers in turn, each deployed as `earth.agnb`. Compare skipped /
  VBlanks, immediate starts, wait time and max SD counts with music and all
  drawing on. Capture counters from the screen if report writing fails.

- EARTH-001-ALIGN-04 [ ] **After Smith reports a successful hardware run,
  remind him to tell AGNB to commit and push the API.** Then verify the
  published source matches the tested bytes and pin this build to that commit.

### Unattended playable Earth comparison (Smith approved)

- EARTH-001-ALIGN-05 [x] Built one fixture for both offsets. Start the normal
  Earth session with keyboard selected, no steering, damage disabled but live
  collisions, and the normal laser trigger held. Keep music and all drawing
  on, 1 KiB refills, 256-byte uploads and pending-edge scheduling unchanged.
  End after the last ordinary Earth update is sampled, before executing warp.
  No music-record write in this fixture. Instead write and close a dedicated
  timing report outside the measured interval and identify its DATA offset.
- EARTH-001-ALIGN-06 [ ] Run control then aligned in separate fresh SD
  directories on the installed VDPlet v0.1.3-alpha / EMOS Olimex; preserve firmware. Collect both
  reports, check completed update counts and offsets, compare skipped edges
  relative to elapsed VBlanks, and distinguish a paired result from a repeatability claim.
- EARTH-001-ALIGN-07 [ ] Repeat the same pair under stock Bistromathics on
  EMOS after the VDPlet pair. Coordinate qualified install/restore with
  Extender. Compare like offsets and keep all four results separately.

The unattended binary SHA-256 is `b25b23494252276302f2152bfa0a1ed6a9dc669df583e36090ae77eaf7b0182b`;
linked reader and pending-edge checks passed. Job: `olimex-align-vdplet.json`.
Bench slot granted; the detached stage service `nurples-align-vl1-stage`
owns the transfer. Evidence: `.build/earth-jit-256/auto-pair/bench-vl1/`.
Verify stage receipts on wake; **send Smith the requested spoken cue immediately
before launching the first gameplay arm**, not during staging. Stop on first failure.

The PARLIO staging batch timed out on aligned/game.agnb after eight completed
file receipts. After Escape stopped the listener, a single PARLIO retry returned
remote status 8 (recovery required). Inspection found state 0x06: partial file
and valid journal only. Authorized abandonment returned state 0; the UART-only
retry transferred and activated all 541,408 bytes (ID 2840383623, worker exit
0; no readback). Evidence: `bench-vl1/uart-file-08/uart.receipt.json`. Nine
files staged; aligned UI, voice and font remain. No game run or handback yet.

Final UART pass completed (worker exit 0): aligned UI 371,052 bytes, voice
79,588, font 2,048, all activated without whole-file readback. All twelve
inputs now have transfer receipts; the previous nine were untouched.
Evidence: `bench-vl1/uart-last-three/*.receipt.json`. Await Extender's
commissioned mixed-stage continuation before the audio cue and paired runs.

VDPlet/EMOS hardware: the control report (offset 72) was collected intact:
531 skipped / 5,075 VBlanks, 4,544 updates, zero zero-tick waits. The aligned
attempt's VDP USB capture records core 0 StoreProhibited at PC 0x4017cccd,
EXCVADDR 0, followed by a VDP reboot. Smith observed the crash at exit.
Buffer pressure during mode restoration is a hypothesis, not established by
the panic. Preserve both original phase directories. Reset the Agon once
the aligned worker joins, inspect the SD result, and rerun only aligned if
the report is absent or invalid. No restaging or firmware change.

Aligned worker joined with exit 1, no live group members and custody retained.
One ordinary Agon-only USB reset under the bench lock succeeded (verified fresh
EMOS input boot 147996885). No P4 reset. Await the commissioned aligned-only
report collection/retry continuation; input admission does not prove that the
report exists on SD.

Post-reset collection recovered the valid original aligned report: 544 skipped
/ 5,088 VBlanks versus control 531 / 5,075. Both ran 4,544 updates with zero
empty updates. No replay, firmware switch or restaging. See
`bench-vl1/recover-aligned-r01/pair.json` and its joined success receipt.
VDPlet received the crash evidence via chat delivery
`nurples-aligned-exit-panic-evidence` for offline investigation with Smith.

Stock Bistromathics comparison, preparation only: keep the Nurples slot,
approved EMOS, Extender P4, staged game and assets unchanged. The prior stock
app1 readback (1,077,680 bytes, SHA-256
`02c59b882d5d4289dd373e85020e6554dbe7193a222e60861338d8064a36910f`)
identifies the comparison firmware; it does not establish the current boot
selection. The last verified selection was production VDPlet app0. Requested
the stock-selection packet from Extender (`earth-vl1-prepare-stock`).
Readiness requires verified stock selection and a working MOS CLI. Keep both
VDPlet reports; do not launch the stock game during preparation.

Smith subsequently approved the flash and both runs together. Nurples started
the supervised `nurples-earth-stock-pair-r01` service using the existing SD
stock updater image and isolated evidence in
`.build/earth-jit-256/auto-pair/bench-stock-r01/`. Require selected-image
readback and a working MOS before the spoken cue and control/aligned gameplay;
use the unchanged staged files and archive the verified VDPlet reports.

Stock pair completed, selected app1 readback matched the pinned Bistromathics
image. Control: 604 skipped / 5,148 VBlanks; aligned: 635 / 5,179. Both
completed 4,544 updates with zero empty updates. Both USB traces show
`EARTH AB SAVED` and MOS accepting `CD /` afterward; no exit crash in this
pair. Original reports and captures are under `bench-stock-r01/run-control`
and `run-aligned`; the supervisor joined successfully. Alignment did not
reduce skips in either firmware pair. Control ran first both times, so the
small differences cannot be attributed to alignment alone. No handback or
firmware restoration in this phase.

Stock reverse order, one verified mainboard reset before aligned, none
between games: aligned 612 / 5,156, then control 634 / 5,178. Each completed
4,544 updates; both saved and accepted MOS commands after exit. Evidence:
`bench-stock-reverse-r01/pair.json`, reset receipt and both serial captures.
The second run skipped more in both orders (+31 forward, +22 reverse).
Across both pairs aligned skipped 1,247 versus control 1,238 over the same
9,088 updates each. No clear alignment benefit or consistent penalty.

VDPlet reverse pair: production app0 independently verified, mainboard
reset (boot 1571749887), then aligned 533 / 5,077 and control 510 / 5,054
with no intervening reset. Both completed 4,544 updates, saved the report,
and returned to MOS, which accepted CD / on each exit. Evidence:
`bench-vl-reverse-r01/pair.json` and both arms' serial captures. The prior
VDPlet second-exit panic did not recur; neither being second nor using the
aligned file was sufficient by itself to reproduce it. No handback.

Follow-up: release the loading-screen background immediately after its single
framebuffer plot, before game image uploads. Keep both flashing title frames
until the one-time transition to attract, once the playfield is drawn.
Do not change gameplay, Game Over, or the pinned timing comparison binaries.

Sprites SET01 hardware A/B: job `nurples-earth-set01-r01`, evidence under
`.build/earth-jit-256/auto-pair/bench-set01-r01/`. Both arms use the staged
pre-splash-cleanup binary (SHA-256 b25b23494252276302f2152bfa0a1ed6a9dc669df583e36090ae77eaf7b0182b)
and the same offset-72 control Earth file. Install and verify each firmware,
reset the mainboard, then measure one complete Earth run, R30 control first
and direct-Set candidate second. Record skipped/VBlanks, update count and exit
status separately; these are whole-game timings, not isolated pixel-loop timings.

### Per-update SD trace (approved)

- EARTH-001-TRACE-01 [x] Build one isolated unattended executable for both
  DATA offsets from the pre-splash-release comparison baseline. Do not
  modify the running SET01 job, the old executable, or either pixel stream.
- EARTH-001-TRACE-02 [x] Record up to 5,000 measured updates in eZ80 RAM:
  16-bit refill duration in 72 counts/ms, 8-bit elapsed VBlanks, flags for
  occurrence, invalid time and clipping. Clear pending data at frame entry;
  match the original pace counter's inclusion rules. No measured-path file I/O
  beyond the existing background reads. Check overflow rather than wrapping.
- EARTH-001-TRACE-03 [x] Save the trace after measurement, validate totals
  against the original counters, summarize refills and skips, and run both
  offsets on each of the two identical-to-SET01 R30 firmware pins after SET01
  completes. Smith specified one verified Agon reset before every run; no
  reversed-order repeats. Start with the already-installed candidate.

Original comparison (no frame trace, same offset-72 input, 4,544 updates):
R30 control 456 skipped / 5,000 VBlanks; direct Set 356 / 4,900. Both
returned to MOS. Test results and firmware readbacks in `bench-set01-r01/`.

Build: `--earth-frame-trace`; proves the pre-instrumentation binary matches
the existing hardware pin before assembling the traced variant, SHA-256
`3f0f26d84d804fab21ff17ed94d03a0c5864bbd1ac9f6d3ca34589b91ce84be4`.
Linked refill/claim/trace-boundary and host parser checks passed. EARTHFR.BIN
is EFT1 plus the old 28 bytes of aggregate counters, then a truncation byte,
then four bytes per captured update: little-endian SD counts, elapsed VBlanks,
flags (bits 0=refill, 1=valid time, 2=clipped SD, 3=odd MOS delta,
4=multiple refills, 5=clipped VBlanks). First prefill outside a measured
update is deliberately not attributed to a gameplay frame. Decode using
`nurples_tools.earth_frame_trace.summarize`. Timing includes interrupts
during a blocking read; coincidence is not proof of causation.

Trace suite: `run-r30-frame-traces.py`, service
`nurples-earth-frame-traces-direct`, evidence `bench-frame-r01/`. Earlier
host-only wait helper failed before accessing hardware; no arm replayed.

Four R30 traces collected; separate reset receipts, 4,544 updates and 895
valid measured refills each. The supervisor joined with exit 0. Results from
`bench-frame-r01/matrix.json` (read times in ms):

| Firmware | Earth file | Skipped / VBlanks | Refill median / p95 | Skips on refill / other updates |
| --- | --- | ---: | ---: | ---: |
| R30 matched | Control (72) | 451 / 4,995 | 8.500 / 12.208 | 121 / 330 |
| R30 matched | Aligned (512) | 453 / 4,997 | 8.000 / 11.847 | 117 / 336 |
| R30 direct Set | Control (72) | 365 / 4,909 | 8.097 / 11.889 | 115 / 250 |
| R30 direct Set | Aligned (512) | 363 / 4,907 | 7.361 / 11.083 | 105 / 258 |

The direct-Set runs missed 86 and 90 fewer VBlanks than their same-file
matched controls; 80 and 78 of those differences were on updates without an
SD refill. The aligned copy had faster median reads on both firmwares, but
the two-skip file differences went in opposite directions. No repeatability
claim from one run per combination. TRACE-04 started under the retained lease.

- EARTH-001-TRACE-04 [x] After the four R30 measurements join successfully,
  run exactly four more: stock Bistromathics control then aligned, production
  VDPlet control then aligned. Verified mainboard reset before each. Reuse
  the two staged 3f0f26d8 binaries, including the original splash lifetimes;
  no rebuild, asset transfer or order repeats. Compare refill distributions,
  misses with/without refill, total skipped/VBlanks and exit outcome.

  Continuation: `run-stock-vdpl-frame-traces.py`, service
  `nurples-earth-frame-stock-vdpl-r01`, evidence `bench-frame-stock-vdpl-r01/`.
  The service waits for success and joined results from the R30 suite before
  accessing hardware.

  Joined exit 0; production app0 verification and all four raw traces retained.
  Each run completed 4,544 updates and 895 valid timed refills:

  | Firmware | File | Skipped / VBlanks | Refill median / p95 (ms) | Skips on refill / other updates |
  | --- | --- | ---: | ---: | ---: |
  | Stock Bistromathics | Control | 605 / 5,149 | 9.222 / 13.333 | 183 / 422 |
  | Stock Bistromathics | Aligned | 606 / 5,150 | 8.972 / 12.681 | 159 / 447 |
  | Production VDPlet | Control | 522 / 5,066 | 9.111 / 13.056 | 156 / 366 |
  | Production VDPlet | Aligned | 535 / 5,079 | 8.528 / 12.514 | 144 / 391 |

- EARTH-001-TRACE-05 [x] After comparing all eight valid cold-start traces,
  select the firmware/file combination with fewest skipped VBlanks over the
  same 4,544 updates. Report a tie rather than calling a unique winner.
  Make five additional independent cold-start runs of the selected combination,
  preserving each raw trace/exit result. Report their individual counts, mean,
  sample SD and range without including the screening run in those statistics.

  All eight raw traces checked. Screening minimum: R30 direct Set, offset
  512 (363 skips); same firmware, offset 72 had 365. This is a selection
  rule, not evidence of a meaningful two-skip advantage. Five additional
  trials dispatched via `nurples-earth-frame-repeat-r01`, receipts in
  `.build/earth-jit-256/auto-pair/bench-frame-repeat-r01/`. Same traced
  executable, same SD allocation, one verified mainboard reset per trial.

  Five distinct boot receipts and raw EFT1 traces verified; each contained
  4,544 updates and 895 valid refill measurements. Results exclude screening:

  | Repeat | Skipped / VBlanks | Mean read (ms) | Median | p95 | p99 | Max |
  | --- | ---: | ---: | ---: | ---: | ---: | ---: |
  | 1 | 359 / 4,903 | 7.489 | 7.333 | 11.083 | 12.403 | 13.500 |
  | 2 | 362 / 4,906 | 7.705 | 7.722 | 11.292 | 12.722 | 13.514 |
  | 3 | 365 / 4,909 | 7.549 | 7.569 | 11.042 | 12.333 | 13.958 |
  | 4 | 368 / 4,912 | 7.705 | 7.639 | 11.472 | 12.611 | 13.861 |
  | 5 | 364 / 4,908 | 7.670 | 7.750 | 11.361 | 12.792 | 14.028 |

  Across runs: 363.6 mean skips, sample SD 3.36, range 359–368. Across
  4,475 reads: mean 7.624 ms, sample SD 2.152, min 4.847, median 7.597,
  p95 11.264, p99 12.444, max 14.028. Supervisor joined with no live workers.
  Smith reported the reservation already returned; no Nurples restoration
  or handback performed after the repeats. Last installed image: R30 direct Set.

### Read-time distributions from the eight screening traces

Elapsed milliseconds per measured 1 KiB refill, including interruptions
during the read. Each row is one run with 895 valid refills; non-refill
updates and the unmeasured initial prefill are excluded. SD below is the
sample standard deviation within that run; quantiles use nearest rank.

| Firmware | File | Min | Mean | SD | Median | p95 | p99 | Max |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Stock Bistromathics | Control | 5.22 | 9.28 | 2.47 | 9.22 | 13.33 | 14.79 | 16.75 |
| Stock Bistromathics | Aligned | 4.92 | 8.92 | 2.24 | 8.97 | 12.68 | 13.89 | 16.21 |
| Production VDPlet | Control | 5.11 | 9.07 | 2.46 | 9.11 | 13.06 | 14.49 | 16.58 |
| Production VDPlet | Aligned | 4.94 | 8.64 | 2.44 | 8.53 | 12.51 | 14.26 | 16.53 |
| R30 matched | Control | 5.21 | 8.50 | 2.26 | 8.50 | 12.21 | 13.61 | 16.10 |
| R30 matched | Aligned | 4.94 | 8.00 | 2.22 | 8.00 | 11.85 | 13.01 | 15.24 |
| R30 direct Set | Control | 5.21 | 8.14 | 2.30 | 8.10 | 11.89 | 13.64 | 15.64 |
| R30 direct Set | Aligned | 4.93 | 7.51 | 2.10 | 7.36 | 11.08 | 11.88 | 12.72 |

### Olimex SD card fragmentation check

Did a slow 1 KiB refill coincide with a jump between nonconsecutive FAT
clusters? The receipts for `/test/earth-ab-vl1/{control,aligned}/earth.agnb`
(`stage-file-01` and `stage-file-07`) identify the staged copies but do not
record their cluster chains. DATA offsets 72 and 512 describe positions
*inside* the files, not where the card's filesystem allocated them.

- EARTH-001-TRACE-06 [x] Preserve both original files in place through the
  eight screening runs and five reset-before-launch repeats in TRACE-05.
  No restaging, rewriting or defragmenting either file. Compare firmware on
  the same staged file; report control-versus-aligned results as observations
  about these two copies, not as the effect of alignment alone.

- EARTH-001-TRACE-07 [x] After the repeats, inspect the original Olimex card
  read-only. Record the volume identifier, sector and cluster sizes, and for
  each file its path, byte size and ordered runs of consecutive clusters
  (starting file offset, starting cluster, run length). Verify the runs
  cover the file through EOF. Report the run count and every discontinuity's
  file offset, or explicitly report one contiguous run. If the original
  allocation is unavailable, mark layout unknown; a copied file is not evidence.

- EARTH-001-TRACE-08 [x] Match each measured refill to its file byte range
  using that arm's DATA offset and accounting for the unmeasured initial
  prefill. Distinguish refills that start at a discontinuity, span one, or
  encounter none. For each group report sample count, read times and skipped
  VBlanks on those updates; check whether spikes recur at the same offsets
  across traces of the same file. If neither file has a break, rule out FAT
  fragmentation for these copies. If a group is empty or too small, say so;
  coincidence alone does not establish cause.

Oryx read-only inspection of the original AGONEX FAT32 volume, serial
`746D-53DB`: 512-byte sectors, 32 sectors per cluster. From file offset
zero, `mshowfat` reported control clusters 1176262–1176318 (917,576
file bytes) and aligned clusters 1176394–1176450 (918,016 file bytes).
Each occupies one consecutive run of 57 clusters (933,888 bytes capacity);
no refill starts on or crosses a discontinuity. This rules out FAT
fragmentation of the two inputs, not SD-controller or other latency.
No files moved. Both directories' game/ui/voice AGNBs also had one run each.
