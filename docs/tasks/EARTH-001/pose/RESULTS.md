# Earth pose comparison — execution record

**November 27 — current fixtures (R02).** Smith accepted the Oryx transition
without the full-screen clear. All 20 interactive and unattended fixture
variants were rebuilt. Their linked playfield setup still verifies the mode,
resets the viewport and draws the background; none calls the clear routine.
The unattended builds keep the old splash-buffer lifetime for comparability.
The eleven timing, pose and cost executables are each four bytes smaller than
their R01 counterpart. Run `sha256sum -c ../docs/tasks/EARTH-001/pose/no-clear-r02.sha256`
from `.build/` to verify their [pins](no-clear-r02.sha256).

R01 manifests, binaries and hardware evidence below are historical. Do not
use the R01 runners to qualify R02. No new bench run or deployment took place;
the sync test remains on hold.

Contract frozen on dev at `7d9cc146814c316c73cee2ba2e96f3ba577bb16a`.
Do not use the earlier async-terrain trace as the control for these arms.
All arms read the same staged unaligned Earth file, do ordinary 256-byte
terrain uploads, retain pending VBlank dispatch and PT3 playback, and use the
pre-splash-cleanup asset lifetime. Only poses for already moving Earth
formation actors in the first traversal are candidates. Flush before other
actors' output, visibility transitions, scoring, shield changes and the first
show of new actors. Measure actual batch lengths, not only the table capacity.

| Full-game build | Private executable SHA-256 | 12 consecutive poses |
| --- | --- | --- |
| Direct | `b9300266b38ee67fb0eae68b9c892e97d21651645bfa3f7161b594fd02dc534a` | 180 bytes, 36 calls |
| Synchronous | `cdb709ae53cadf25da197fc6076cf5fe3e30a4f2a3f1e1e4c8acce1150e20d11` | 180 bytes, one ordinary MOS send |
| Async | `498139d81cd7fb812a3b128ed55aa2139041abef3030767192dcadb66d0fb135` | 180 bytes, one EMOS/Eirene send |

Linked eZ80 pose routines for these three arms produced identical ordered
command bytes in the CPU oracle for zero, one, sparse, twelve and thirty-two
poses, including fractional and negative coordinates. A delayed simulated
callback also exercised the linked async barrier: the read source stayed
unchanged until the callback and the loop waited separately for wire drain.
MOS/UART are stubbed
in that check; this is not target timing. The physical job for those exact
binaries passed offline manifest preflight. EPS1 reports add actual eligible
poses and send count to each existing timing row (including the SD refill
duration). Compare 4,544 completed updates, skip/VBlank counts and refill
min/mean/sample SD/median/p95/p99/max on the same verified firmware.

The stock state-only arm is **actually compiled with Golem**. A general
loop required 5,964 command-model operations for twelve poses. The specialized
suffix implementation required 464 in the same model and emits only the
requested records after shrinking a group; neither figure is a wall-clock
measurement. It sends 26 + 6N bytes per nonempty group (98 for twelve):
an adjust-data header, state, an expansion call, and a call into the populated
output suffix. Compiled install is 22,424 bytes; the game sends 22,417 after
omitting the backend's unrelated seven-byte global affine feature toggle.
The install occurs after stopping title music and before game timing starts.
Linked state packet/entry/offset checks passed. Golem executable:
`089e4e5578981a3b7573db9afd609121c80c41a25f97d611ff6ad99de929f0ee`.
Independent decoding of the install counted **431 live buffers and 16,383
bytes of payload** (including the 886-byte shared state). This does not
include ESP32 allocator/map/block metadata. All are released by the normal
game-exit clear-all. There is no VDP heap high-water measurement here.

Native feasibility: publish to the existing VDPlet host once per group;
do not mutate firmware. Query the installed runtime's sprite support, use its
returned handle, and compare against a direct build on that same firmware.
The native VDU submitter refuses `printerOn`, but the stock bench observer
turns the MOS printer mirror on. The separate native-direct and native arms
switch it off during play, then restore it for the final report in both arms.
Do **not** compare native against the mirror-on direct timing as if firmware
were the only variable. Measure all 4,544 updates. On capable firmware, the
native module also reconciles all successful submissions and poses against
the eZ80 totals at shutdown, so refusal to draw cannot earn a speedup.
With twelve consecutive poses its expected wire size is 101 bytes
(11-byte update + count + 72 bytes of state + 17-byte silent RUN). The
runtime validates sprite tables and interprets ordinary VDU on every submit;
the module's expansion loop alone is not the relevant timing.

Audio is a separate design decision. The existing per-channel queue carries
channel events, not an arbitrary timed PT3 command list, and production's
lean build omits that richer queue. The PCM sink is not a drop-in scheduler
for AY oscillator register changes. Native RUN is foreground work, not an
application-owned periodic ESP32 callback. A stock-compatible alternative
is still the current prepared PT3 packet + PRT; a future timestamped queue
would need defined clock ownership, watermark/underrun behavior and a way
to pause or switch songs without delaying game SFX. Leave music identical
across pose tests.

### Can the VDP clock our existing PT3 music?

**Not exactly on the installed interfaces.** The existing music does not
start a fresh tone each tick. It changes parameters on running voices. To
measure what matters, we ran the linked control executable's own
`ay_init`, game amplitude table, `lp_prime_track`, `lp_prepare` and
`lp_send` for the first 4,500 ticks of each pinned tune (90 seconds at
50 Hz). A bounded CPU test captured each nonempty ordinary MOS send, checked
its length against the game's prepared packet, decoded every operation and
retained only aggregate results. No game or firmware changes. The measurement
excludes setup, warp silence, private tune contents and unrelated SFX.
It describes the first 90 seconds, not the worst case for an entire tune.

| Track | Raw B/s | Empty ticks | Multi-voice ticks | Sample-rate changes | Largest 1 / 4 / 8 successive ticks (B) |
| --- | ---: | ---: | ---: | ---: | --- |
| Earth: BREAK2 | 916.4 | 144 | 2,990 | 216 | 79 / 151 / 278 |
| ACIDFLSH | 1,423.9 | 47 | 3,903 | 847 | 93 / 194 / 358 |
| Terminator | 1,441.6 | 0 | 3,867 | 481 | 73 / 195 / 358 |
| Bugs2 2 | 1,292.7 | 93 | 3,905 | 917 | 78 / 215 / 421 |
| LOADTUNE | 1,106.3 | 94 | 3,446 | 3 | 73 / 156 / 262 |

Earth issued 6,265 volume changes, 6,195 frequency changes, one waveform
change and 216 sample-rate changes on channels 0–4. Starting at zero-based
volume/frequency event 200 in that decode, **10,923 consecutive events**
matched the direct stock FAE log in channel, operation, value and order;
10,925 matched the sync log. These logs are independent observations, not
the test's own expected bytes. This is command corroboration, not audible
output/timing parity. The completed Golem log filters out audio, so it
cannot corroborate sound. Reproduce with the linked test executable:
`--music-shape DIR_WITH_DIRECT_BIN_AND_SYMBOLS TRACK_INDEX 4500 [UNFILTERED_STOCK_LOG]`.

Three distinct options:

- **Shared audio queue:** group START and SILENCE can synchronize channels
  and represent rests. TONE cannot apply an in-place parameter update:
  its playback starts with fresh oscillator phase and noise state for every
  entry ([queue renderer](../../../../../agon-vdplet/src/vdp_audio_queue_core.h)).
  It has no per-event equivalent of the stock channel sample-rate command.
  Splitting a sustaining voice into 20 ms TONE entries is not equivalent,
  even if pitch, volume and total duration look right. The lean production
  VDPlet does not ship the shared queue in the first place.
- **Stock buffered VSYNC callback:** there *is* a firmware-free VDP-side
  wakeup. Bistromathics [polls the frame counter](https://github.com/AgonPlatform/agon-vdp/blob/c7ac293d2aa81ddfa693390549bcd909069c8fc3/video/context.h#L1312)
  and [calls the buffer from its command loop](https://github.com/AgonPlatform/agon-vdp/blob/c7ac293d2aa81ddfa693390549bcd909069c8fc3/video/vdu_stream_processor.h#L597),
  not the audio interrupt. At nominal 60 Hz, five steps per six frames
  preserve average tempo, but four gaps are 16.7 ms and one is 33.3 ms per
  five steps instead of five 20 ms gaps. A busy interpreter can observe more
  than one elapsed frame as one callback. The exposed frame counter can
  tell us *how late* we are, not play a missed tick in the past. Mode
  changes remove this callback. Useful as a separate jitter audition if ever
  wanted, not a silent fallback for the 50 Hz soundtrack.
- **Native VDPlet:** the foreground VDU validator admits the four stock
  audio command shapes (subject to its normal resource and callback guards).
  RUN must return; it is not a periodic callback.
  The production async worker's COMPUTE service table supplies only job/stop,
  not VDU/audio submission. Moving the sequencer into a module alone buys no
  playback clock. A faithful timed host service would queue *complete tick
  deltas*, including zero-length ticks, on the mixer's clock. On Earth,
  the largest observed four-tick payload was 151 B (159 B with four 16-bit
  lengths, before transport header). Across these excerpts it was 215 B raw.
  Feasibility is about ownership of the playback deadline, not large buffers.
  Retain queued unplayed deltas and a played sequence number across pause;
  flush by song generation on replacement, stop music immediately for M,
  and never reserve SFX/speech channels. Keep PRT1 as the shipping path.

Used VDPlet source `290919d8d4bb47ef34927abb35cbddeae7de6294` for the
queue/ABI audit. No hardware queue, audio audition or timing change claimed.

**Pi functional run authorization and bounds.** In the November 26 dinner
goal in Nurples chat `019fc0a7-f33a-7160-a663-427af9394e01`, Smith wrote
“oryx will be down, but the pi5 is available for emulator runs.” Scope:
serial stock FAE 1.2.5 / Arthur / Bistromathics, one isolated full Earth
run each for direct, sync and Golem. No async on stock MOS. At most 220 s
per launch, stop on first failure, 80 C/512 MiB limits, retain SD and
logs, reap only owned children. Not an emulator performance benchmark.
Use manager admission and start/end notifications independently of Olimex.

Pi run R01: direct and sync each completed 4,544 updates and 22,754 eligible
poses. The stock VDP independently logged 45,652 equal air frame/move actions
(includes non-batched publications), in exactly the same order, SHA-256
`fdd1b042e6a711dd30571faa75e9d9dc0571070a4757f3aa74ff3e31b1b30f14`.
Golem reached gameplay but hit the host's 96 MiB verbose-log guard. Preserve
R01. R02 is a single fresh Golem profile using the same executable, game,
firmware and 220 s/80 C/512 MiB guards; filter interpreter debugging at the
host pipe while retaining every published frame/movement line for comparison
against the completed direct R01. Gen 1299 admitted R02; it completed and
the private emulator was stopped and the Pi lease released.

| Stock emulator, full Earth | Eligible poses | Host sends for those poses | Recurring pose bytes |
| --- | ---: | ---: | ---: |
| Direct (R01) | 22,754 | 68,262 | 341,310 |
| Synchronous (R01) | 22,754 | 3,769 | 341,310 |
| Golem (R02) | 22,754 | 3,769 | 234,518 |

All three completed 4,544 updates including 636 with at least eight poses,
895 successful 1 KiB refills, and identical **45,652 ordered frame/move
events** observed on the VDP side (digest above). Counts of poses and sends
come from EPS1; bytes follow from the checked packet format and exclude
setup and all non-pose traffic. Golem saved 106,792 recurring bytes (31.3%).
In both batched arms 821 updates sent nothing, 3,677 made one send and 46
made two. No update contained more than ten eligible poses; the largest
ordinary batch in this trace was 150 bytes, the average 90.6. Even the peak
was smaller than the earlier proposed 256-byte crossover. That motivates
checking async, not presuming it wins. The per-update pose counts and SD
refill positions matched across all three files. For the collected hardware
reports use `.venv/bin/python -m nurples_tools.pose_compare ARM=POSE.BIN ...`
after checking the run/firmware receipts; the tool separates the mirror-off
pair and reports refill distributions and dense-update skips.
Raw EPS1 files, under `.build/earth-pose-r01/`, in each profile's
`sdcard/test/pose/POSE.BIN`:

| Profile | Report SHA-256 |
| --- | --- |
| pi-stock-r01/direct | `9a12e0bc0d4bdd706f9d2bf72cc856c1ddac52e3d7ed0ef679aba57aed21b8e6` |
| pi-stock-r01/sync | `d614497d331887d9959f95ba8aa450a89f3ca44372d289271c17cc20a530950d` |
| pi-stock-golem-r02/golem | `8b5e82d7c27047b03453685b49e035ae79f15e2732295751e2b8c2eb2bef9b2a` |

Native builds are pinned in [the native manifest](olimex-native-r01.json);
the 1,272-byte module SHA-256 is
`de1f52eb22639b47c8bd705e228e63a2d5cf842b241045d524c09de8e3406962`.
The actual module passed the host C test, and the linked guest passed the
eZ80 test for successful and rejected submissions and final reconciliation.
The [native-only packet](/Agon/mystuff/agon-extender/agents/earth-001/pose-uart/NATIVE.md)
also passed host preflight; nothing in these checks establishes target execution.

Additional async functional cell under the same human Pi5 emulator authority:
one private Flexe profile with the verified ARM64 Jukebox runtime, Eirene RC3
(`97311c437093e416342a6c836cf9914f2ced027d`), its previously qualified
receiver-test VDP (`201ad8ab…`), a fresh FAT image and the exact async game
binary above. This is a UART compatibility check, **not** an ESP32 performance
comparison. Bound to 10 minutes, stop on first failure, 80 C/512 MiB guards,
request separate admission, send START/END, join only our own children
and retain the raw wire, SD image and any diagnostic on failure.

Async R01 (Pi gen 1301): **partial functional evidence, not a pass**.
RC3 loaded 422 images and ran Earth. The eZ80 UART diagnostic recorded eight
RX overruns during gameplay. Stopped the emulator on that observation
(~102 s); final counts were 2,619 transmit IRQs, zero TX drops, eight RX
overruns. The CPU receives traffic in the opposite direction from pose
uploads. Of 81,630 bytes sent by the VDP, 13,072 packets were audio replies
and 2,915 were mode reports; the capture does not identify which eight bytes
were lost at the CPU. The 2,631 complete select/frame/move records on the
outbound wire matched the start of the stock direct run in order, including
IDs, signed coordinates and frames. No EPS1 file, full-game timing or claim
of a hardware fault. Raw wire, final framebuffer, both UART endpoints, fresh
FAT and cleanup proof retained in `.build/earth-pose-r01/pi-flexe-async-r01/`.
All Pi workers joined and the lease was released. No repeat. The primary
Olimex binaries and their pins are unchanged.

Hardware status: queued behind Sprite for exclusive Olimex access. Smith's
temporary UART-only policy supersedes the existing schema-3 physical runner;
the named-job runner hard-requires PARLIO both ways. Its physical path is
disabled. Extender supplied the [commissioned UART packet](/Agon/mystuff/agon-extender/agents/earth-001/pose-uart/NURPLES.md).
The [matched Golem continuation](/Agon/mystuff/agon-extender/agents/earth-001/pose-uart/GOLEM-MATCHED.md)
binds its run to the completed direct firmware and EPS1 receipts. Both host
preflights pass. Scheduler subsequently reported the preceding operator's
UART listener and MOS input unavailable after reset; VDPlet retains custody.
No Nurples access before actual slot-open. Use the verified firmware pin
from the actual handoff. Compare only arms
on identical installed firmware. No fixture was staged and no firmware
was touched at the time of this entry. Do not infer a speedup from byte
or call counts.

**21:36 UTC handoff check (November 26):** the Olimex scheduler directly
confirmed the previous owner's workers have exited; no unattended recovery
is reported running. The commissioned reset did not restore MOS input or
bring up the UART listener. The FIFO still has VDPlet in custody, then
Sprite, then Nurples. This is a stopped bench, not a long-running test to
poll or replay. No Nurples physical arm has started. Resume on the
scheduler's actual slot-open notice after usable handback and Sprite;
take the firmware identity from that new handoff, not the old receipts.

### Sparse attribution observer (separate from the scored games)

POSE-05 needs eZ80 time inside the ordinary MOS send, not a wire-speed
estimate. The three optional `--earth-pose-cost direct|sync|golem` builds
live under `.build/earth-pose-cost-r01/`; none replaces a scored binary.
Beginning at counted play/warp update 17, sample every 61st update and
rotate three scopes across those frames. The scopes do not nest:

| Scope | Measurement |
| --- | --- |
| Publish | One real eligible actor, including the three sends on direct; CPU record preparation only on sync/Golem. |
| Nonempty submit | After testing the batch for empty, through packet finalization and the send. Group by actual input payload bytes. Direct has no such operation. |
| MOS send | From just before the ordinary RST to return, grouped by real byte length. Only formation pose sends count. Not the time to scan out or render. |

The probe uses the existing 72 counts/ms PRT1 clock, records both stamps in
RAM, and does a matched empty call/return on each sampled frame to expose
the measurement floor. Raw times include interrupt service and observer
overhead; show the calibration beside them, not a claimed sub-tick cost.
Different scopes sample different frames: do not subtract unmatched percentiles.
The separate output `POSECOST.BIN` is 47,782 bytes: header, bounded time
events, and all 4,544 ordinary per-frame records for cross-checking.
`python -m nurples_tools.pose_cost ARM FILE` validates frame IDs, event order,
packet lengths, counts, refills, and clock rollover. Unusable durations are
counted separately; overflow, missing timer ownership, and incomplete games
fail validation. Do **not** promote the observer's skipped-frame total to the
primary comparison, or use synchronous return times as an async measurement.

Linked eZ80 checks exercised all three observers with the 4,544 recorded
per-frame workload counts and a synthetic PRT. Against the uninstrumented
twins they emitted identical pose bytes on every replayed frame. Direct
produced 1,268 stamps; sync and Golem produced 486 each, all decoded. These
are test counts, **not hardware durations**. Six scored executable hashes
rechecked unchanged. [Optional hardware manifest](olimex-cost-r01.json);
reuse the staged assets, same firmware as the first four scored arms, one
reset per run, and collect the generated files by ordinary UART. Do not
interrupt or reorder the primary comparisons to run this observer.

Pi compiler lease 1302 released. The offline manifest preflight lists stage,
direct, sync, Golem and handback; actual transfer still requires the UART
adapter. Extender has the supplemental commissioning request. All six
original linked transport tests and the new parser tests passed. Full Python
suite: 258 tests, 20 skipped, five failing assertions. The same five
reproduced on a private export of frozen `7d9cc14`: the older defaults
inventory does not list `bomb_defaults`, and starfield retouch hashes for
0/3/4/6 differ. Neither baseline issue was changed for this experiment.

Extender delivered the [supplemental UART packet](/Agon/mystuff/agon-extender/agents/earth-001/pose-uart/COST.md).
Rechecked the adapter SHA and its host-only preflight from the commissioned
checkout. It is ready **after** the scored comparisons on matching firmware;
no physical phase or file transfer has started.
