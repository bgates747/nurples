# MUS-001 — Audition tracker music in the Nurples game loop

**Current decision:** R03 passed on FAE and Console8. Smith approved its
campaign audio for the ordinary game on `dev`, with private tunes and
executables kept out of Git. The original experiment and subsequent review
records below are chronological; the initial fixture-only limit is historical.

Find out whether a single-chip PT3 tune sounds right alongside game effects
without making play less responsive. The first experiment uses the existing
frame clock, not a new interrupt. Smith approved one private FAE audition
on Oryx. Build in isolation, leave the ordinary game and existing profiles
untouched, and stop at a verified, stopped profile for Smith to launch.
Recover by using the ordinary game; recreate the ignored song from the
manifest URL and hash if necessary. No hardware run or production promotion.

## Selected experiment

After `do_game` and its orderly-exit check, immediately before `@wait_frame`
in [the main loop](../../../src/asm/nurples.asm), advance one tracker tick
per completed music-active frame and submit only changed VDP audio settings.
Input, simulation and graphics submission stay first. The pause branch skips
the hook; Escape is still polled after VBlank. No extra wait or catch-up:
a late frame holds the previous sound.

At a steady 60 frames/s, 60 active frames must produce 60 ticks. A tune
written for 50 Hz plays 20% faster, including its tick-driven effects.
Preserve the original for comparison and identify any retimed copy
separately. The five-ticks-per-six-frames alternative is not this experiment.

## Playback decisions — Smith, 2026-11-21

- Start the tune from the beginning when a new game first hands control to
  the player, not on the splash or during the opening swoop. Start once per
  game: death and respawn carry the tune forward without muting or restarting.
- Stop playback and silence the music channels on entry to Game Over. Leave
  them silent until play begins in a new game.
- On the first **P**, silence the music channels, including any sustaining
  note, and freeze tracker position and effect state. On the next **P**,
  restore the current voices and continue at that position; do not restart,
  skip ahead for paused time, or mute separate game-effect channels.

## Selected original

Smith selected [Terminator by Spawn (1999)](https://zxart.ee/tune/79449)
instead of LOADTUNE. ZXArt lists PT3; the downloaded module identifies itself
as a ProTracker 3.4 compilation of “TERMINATOR II remix.” The unmodified
5,219-byte original is at `.build/mus001/audition/terminator.pt3` (ignored
by Git). [samples.json](samples.json) records the native download URL,
byte count, SHA-256 and the other Jukebox sample fields. No game reuse
terms were stated; keep the module out of published builds unless cleared.

## Scope and checks for the approved fixture

- MUS-001-01 [x] Record the clock, hook and scope from
  [Rally's handoff](/Agon/mystuff/agon-dev-env/cross-agent/agentcoms/messages/2026-11-21_14-46-54_rally_arcade-sfx-pt3-vblank-handoff-one.md)
  and [AgonArcade's measurements](/Agon/mystuff/AgonArcade/docs/tasks/ARCADE-SFX-001/README.md).

- MUS-001-02 [x] Smith approved the state and pause decisions and requested
  this contract be frozen on `dev` before fixture implementation.

- MUS-001-03 [x] Select the private audition tune, verify the native
  download and record provenance and reuse status in the sample manifest.
  For the eventual fixture, budget the 6,237-byte timer-free engine
  plus the tune and game glue. Leave existing loader scratch allocation
  untouched. Music uses VDP channels 0–4; assign game effects separately.

- MUS-001-04 [ ] In an isolated fixture, count frames and ticks; compare play
  with music on/off, including while firing and during an explosion. Verify
  effects are not cut off by music. Pause mid-note: verify silence, no ticks
  during pause, and continuation at the saved position. Trace a death and
  respawn without a tracker reset, silence at Game Over and a fresh start
  in the next game. Verify exit leaves no hanging voices. Report cadence and listening
  results separately. Hardware testing is deferred.

A separate 50 Hz timer is a fallback for discussion, not a parallel task.
Before adopting it, establish timer ownership and audit live alternate
registers; if used, the interrupt should count pending ticks and leave VDP
writes to the foreground so audio cannot split a graphics command.

## Approved audition amendment — two tracks, one channel map

Original contract freeze: `c52aad513e7c9185f6d9d4dffd4b4b89d249ee0b`.
Smith added [Spawn's Bugs2 2](https://zxart.ee/tune/79447) for comparison.
Keep Terminator as the default load. Stage the second private audition in a
separate guest directory under the same stopped FAE profile, with the same
`nurples.bin` filename, engine and game settings. Keep agonAMP unchanged
on its default channels 0–4. Map the fixture laser to 5 and the two explosion
voices to 6 and 7. Neither track is cleared for publication.

## R01 — fixture and staging evidence

Amendment freeze: `5950089106feb9b9c1540d7864ab0ebd4459b0b3`.
The [private fixture builder](../../../build/scripts/build_pt3_review.py)
uses unmodified agonAMP modules at `91ef401bcb7f636e7593ad1ba176508344deebb9`
and the existing AgonArcade pew-pew and v3 explosion patches. Only the
fixture attaches them to successful player laser fire and the player's final
explosion. Pause silences AY volume without advancing its software envelope;
the first resumed frame restores the saved volumes before the normal tick.

Both builds first reproduced the 47,674-byte ordinary game byte for byte.
Assembled-code checks located the pause and exit hooks, a single PT3 tick
and AY update before VBlank, no timer module, the correct channel numbers
and the exact selected module ending immediately before the actor tables.
These checks do not establish realtime cadence or an audible pass.

FAE on Oryx, checkout 573 (`ac7934ca48d2`), profile
`~/Agon/mystuff/nurples/.emulator/mus001-terminator-fae-r01`.
Pinned RC3 MOS and stock native VDP verified; both guest binaries and all
runtime assets read back from the same card. The load-only startup selects
Terminator; from MOS use `CD /test/bugs2`, `LOAD nurples.bin &040000` and
`RUN` for Bugs2 2. Both guest filenames are `nurples.bin`.

| Tune | Bytes in executable | Executable SHA-256 |
| --- | ---: | --- |
| Terminator | 59,601 | `3dbd85521708a2ae55fa140d37ad59c25e58f594153ae74ef6d6f5d7dbdd8a03` |
| Bugs2 2 | 59,979 | `e9d35cecc8662fd51f38554aa4ad8fb594f2fe54110f32259e8db92bd058b1a9` |

The final staged card hash is `bcbe3dc808bd142a8235e74f0b4363f6bc3c7e160cca19abb3c630f2aa86780a`.
Unmodified Bugs2 2 (5,597 bytes) is cached alongside Terminator; the
[sample manifest](samples.json) records both native downloads. No production
file or physical card changed.

## R01 listening review — 2026-11-21: both tunes passed

Smith played **Terminator** and **Bugs2 2** on FAE. For both, P silenced
the music and unpausing resumed it from the same spot. The tune continued
through player death and respawn, then stopped at Game Over.

This is a pass for the audible music behavior. It is not a measured
frame/tick count or a separate verdict on the laser/explosion mix, new-game
restart or Escape cleanup; those parts of MUS-001-04 remain unverified.
Smith closed FAE. A fresh whole-host check found no emulator processes;
Oryx checkout 573 was released at 15:26:59 Eastern with no waiting request.

## Mix and retrigger feedback — 2026-11-21

Smith found the R01 music too loud to hear the effects clearly. Keep the
agonAMP channel assignments. For the next audition, try its existing
half-level AY amplitude table (peak 21 instead of 42) for music alone.
Do not lower VDP master volume: that would lower effects too. Half level
is a proposed starting point, not an accepted final mix.

Repeated effects must replace themselves immediately. Every successful
laser shot should cut off the previous laser and start its attack and pitch
sweep from the top. For a layered effect, restart all its own voices
together without touching other effects or music. R01 only sets up patches
once and then sends bare play commands; stock VDP can refuse one on a busy
channel. AgonArcade's replace path reselects the waveform and reapplies the
patch before play. Verify with sustained rapid fire during music and during
an explosion: one fresh laser attack per shot, no stacked or dropped shots,
and no interruption on the other channels.

## R02 — two-track mix ready for listening

Smith approved half-volume music, unchanged effect levels, and restarting
each cue on repeat. Both fixture binaries select the single-chip player's
existing half-amplitude table after AY initialization (peak 21 instead of
42; master volume untouched). On each trigger, reselect and configure only
the cue's own waveform(s), then play from the beginning. For the blast,
both voices are prepared before either play command.

Compared the assembled R02 packets to R01 for both tunes: original patch
setup and note/level bytes match exactly, now in the retrigger order. The
selected table halves every AY amplitude (rounded up). Both modules and
the ordinary game control were byte-identical to R01. This is not yet a
listening verdict on balance or rapid-fire attacks.

New stopped FAE profile on Oryx: `.emulator/mus001-mix-fae-r02`, slot
574, request `3235fd67806e`. Both guest binaries and runtime assets read
back correctly. Terminator defaults to `RUN`; for Bugs2 2, use
`CD /test/bugs2`, `LOAD nurples.bin &040000`, `RUN`.
Card SHA-256: `99329f3f3d42eab82dc4db0549bf05cdb7112f78741bea3b47a21472dd7117b3`.
R01 and production are unchanged.

## R02 listening review — 2026-11-21: mix and laser retrigger passed

Smith tested **Terminator** and **Bugs2 2** on FAE. With music at half
level and effect volumes unchanged, he heard the laser restart on every
shot without cutting off the music. Keep half-level music as the accepted
fixture mix. This listening pass does not measure frame cadence or
separately confirm that both layers of a repeated explosion restart.

Smith closed FAE. A fresh Oryx inspection found no emulator processes;
checkout 574 was released at 15:50:46 Eastern. Flexe was notified that
its FX-323 request was next.

## R03 — private per-level music contract

Use one private build for the whole campaign. Level 00 (modern) uses
Terminator; level 01 (recovered Z80) uses Bugs2 2. Keep R02's opening
control handoff, pause/resume, death/respawn, Game Over, half-level music
and retriggering effects. Stop the level tune as soon as the game publishes
the between-level transition; start the newly selected tune from the top
when the next level opens, including wraparound to level 00. Warp still
accepts player input: keep normal effects live, but add no warp cue yet.
Choosing transition music or a synthesized effect is a separate decision.

Verify local assembly against the ordinary control, the track selection
and lifecycle boundaries, and both embedded modules against the private
manifest. Do not stage another emulator, promote to production, or publish
the songs or binaries containing them. Recover the reviewed R02 from this
checkpoint and the ignored private inputs.

R03 local result (contract `2622e324948c5be839f0f5c3c31e2066b36f13c5`):
the fixture builder now also accepts `--campaign-songs .build/mus001/audition`
instead of `--song FILE`. It resolves the tracks from campaign TMX identities
in manifest order and checks both private input hashes. Ordinary assembly
and both R02 single-song modes reproduced their previous binaries byte for
byte. The combined 65,296-byte executable is local and ignored:
`.build/mus001-r03/campaign/nurples.bin`, SHA-256
`91ec4e9757c1535a04fa62f754802e63c472e60494bf2550767642028c0e8b1e`.
Assembled-code checks covered the bounded two-entry lookup, exact track
bytes, warp's music-only mute and effects dispatch, and restart after the
map changes without waiting for another player spawn. Not yet playtested or
staged. The listening check is level completion → warp with laser sound →
arrival with Bugs2 2, then Terminator on campaign wraparound.

## R03 amendment — target explosions and presentation music

Smith requested both additions in the same private per-level fixture.

- MUS-001-05 [x] Retrigger the accepted two-voice explosion from the top
  for **each** destroyed ground or air target, not once for clearing the
  whole screen. Cover the existing destruction paths whether or not the
  visual explosion pool has space. Do not trigger for a pad launch, retreat,
  offscreen despawn, existing crater animation, or each particle of one
  explosion. Keep the player finale, laser, mix and channel assignments.
  Check distinct kills even within a single frame.

- MUS-001-06 [x] Start Spawn's LOADTUNE from its beginning at the first
  post-load controller prompt. Continue uninterrupted through controller
  selection and attract while waiting for the accepted start input. Stop it
  when Start is accepted, before session setup/swoop. On Game Over switch
  back to LOADTUNE from the beginning; do not restart it each waiting frame.
  Stop it on accepted restart before the new game begins. Preserve the
  opening level's control-handoff start, per-level tunes and music-free
  between-level warp. Exit must silence owned audio from either prompt.

Check the private LOADTUNE original against Jukebox's metadata; retain only
provenance and hashes in Git. Verify assembled triggers, music transitions
and private inputs; leave R02 and production unchanged. Hardware and a new
FAE deployment are outside this local implementation tranche.

**Local build and verification.** Contract freeze
`764192a932a7504f916c926d8834c4bcf63e5089`. Both additions are
implemented in the campaign fixture. Destructive air collisions (seeker or
fireball impact) and ground-target cratering (pad, turret, electrode) each
queue one retrigger before visual allocation. A count replaces the original
one-bit event, so kills in the same frame are not merged. Electrode lightning
cleanup, seeker retreat and the player's individual debris particles are not
separate kills. The player's final explosion still uses the same cue.

LOADTUNE's 1,552 private bytes matched Jukebox's SHA-256. Assembled-code
checks verified the exact three modules, two level-table entries, shared
post-load/attract/Game Over music entry, accepted Start and exit stop
hooks, destruction hooks and the per-event counter drain. Ordinary and both
R02 binaries reproduced byte for byte. Current private binary:
`.build/mus001-r03-presentation/campaign/nurples.bin`, 67,003 bytes,
SHA-256 `c18f8afb1043f46fbe2dd54faf78514cb9f15d3fa639d9ef7db49ccb28bb570c`.
No listening verdict at assembly time.

**R03 FAE listening review staged (2026-11-21).** At Smith's request the
combined presentation/campaign binary above was staged without rebuilding
in `~/Agon/mystuff/nurples/.emulator/mus001-campaign-fae-r03` on Oryx.
Checkout 577 (request `2b7e1d780c5a`) was held for Smith's run. Verified
the pinned FAE, RC3 MOS, stock VDP, load-only startup and byte-for-byte
FAT readback of the executable, game/UI containers and font. Card SHA-256
`482be7b0c8e879762820917432feabb8a66233c66c9106a96a61a240244290f8`.
Left stopped. Launch `./fab-agon-emulator` in the profile and type `RUN`
at MOS. This is one campaign; no directory or tune switching is required.

**R03 FAE listening review — passed, 2026-11-21.** Smith heard LOADTUNE
at start and Game Over, Terminator stop for warp, and Bugs2 2 start on entry
to the next level. Destroying both ground and air targets produced audible
explosions. Same-frame kill counting was checked in the assembled code,
not distinguished by ear; frame cadence was not measured in this run.

Smith closed FAE. A fresh whole-host inspection found no emulator processes.
Oryx checkout 577 was released at 16:38:27 Eastern and Flexe was notified
that its FX-325 request was next.

**Console8 deployment — 2026-11-21; hardware verdict pending.** R03 source
and the FAE pass were published on `dev` at `7ba955ac326939e9be984ad0ea350ad0c01ce89d`;
no audio module or executable containing one was published. Deployed the
same FAE-reviewed 67,003-byte executable to the Console8 SD on Oryx at
`/mystuff/nurples/nurples.bin`, together with the two AGNBs and font.
All four installed file hashes matched. No Nurples install was found under
`/test` or `/tests`. The prior install, including its no-mode diagnostic
and app-local stock VDP copy, was backed up off-card before replacement.
Root firmware files and autoexec were unchanged; no firmware was flashed or
bundled. The card was synchronized and left mounted.
[Deployment receipt](../../../.build/console8-r03-7ba955a/deployment-receipt.txt).

**Console8 accepted; production promotion authorized, 2026-11-21.** Smith
played the installed R03 campaign and approved it for production. Promote
that exact behavior to the ordinary build on `dev`: same level assignments,
LOADTUNE, pause/resume, mix, retriggers and kill cues. No new audio design,
hardware deployment or firmware change. Keep all three PT3s and all binaries
containing them out of Git. Preserve the ordinary local `tgt/nurples.bin`
filename. Verify a clean assembly byte-for-byte against the hardware-approved
R03 above, and check the outgoing Git content before publishing.

**Production integration.** The normal orchestrator now stages the private
inputs; the music, pause, exit and kill hooks are in the game source, not
injected by the review script. The three unmodified song modules remain local.
The executable, symbols, both containers and font in `tgt/` were promoted
from or matched against the accepted R03 package (all five hashes passed).
The normal game's expanded campaign assembly matched the original R03 recipe
in 15,436 statements, including the INCBIN payload hashes. Missing/altered
song checks and the no-clobber test passed. Pi request `62799b99d53c`
tracks the separate native rebuild check. No card rewrite or new hardware
review is needed for the promoted artifact.

**Independent rebuild, 2026-11-21.** Pi generation 1151: the ordinary
`--assemble` output compared byte-for-byte equal to the accepted Console8
R03 (`c18f8afb1043f46fbe2dd54faf78514cb9f15d3fa639d9ef7db49ccb28bb570c`).
The host suite ran 230 tests (one failure, one error, one skip): the isolated
presentation fixture does not define `mus_campaign`, and the assembly-safety
test tried to read the native symbol file as UTF-8. These need follow-up;
neither was a mismatch in the ordinary binary. Logs are in
`.build/mus001-production-verify-1151/`. Released the compiler slot to Rally.

## 50 Hz loader experiment

Make LOADTUNE playable during asset loading without pacing the upload at one
block per tick. First prove timing and asset integrity under this demanding
workload; do not migrate gameplay or promote before reviewing the result.
Smith approved this loading-first contract. Nurples owns the isolated fixture.

- MUS-001-07 [x] Prime one immutable tracker command batch before arming an
  idle 50 Hz PRT. ISR only acknowledges/counts. Foreground owns the VDP;
  send due music at complete command boundaries, then prepare the next batch.
  Preserve loader errors, restore timer/vector, and never steal a busy timer.

- MUS-001-08 [x] Keep 8 KiB SD reads and use independently framed 512-byte
  appends (short final append). Check after reads and between appends; no
  waiting or per-tick quota. Measure elapsed loading, service lateness and
  missed periods; distinguish measurements from wire-time estimates.

- MUS-001-09 [ ] Build isolated control and candidate from the accepted
  production baseline, verify ordinary output and container bytes remain
  unchanged, and qualify loader continuity into the prompt, clean exit/rerun
  and throughput against control. Stop for listening/timing review. Gameplay
  keeps its existing frame clock; its conversion is the later tranche.

Recovery: ordinary production at `71a74f865c3a71b4dddf6670f6f2a1a45933fdab`.
Retain its local binary, private song inputs, existing profiles and cards.
Build fixtures under ignored `.build/`; no song-bearing binary or module in
Git. Use serial leased Pi assembly and an isolated Oryx FAE review; no hardware
deployment. A 60-tick/s renderer playing 50-tick/s music is 20% fast;
do not equate emulator wall time or unmeasured production FPS with that ratio.

**R01 implementation and FAE result (2026-11-21).** Contract:
`916765b1e807d3c35a32757a042b0e006b0ce3e3`. The
[fixture builder](../../../build/scripts/build_prt_loading_review.py) makes
an ordinary hash-checked build, a timing-only 8 KiB control and a private
candidate. Production assembly sources and tgt outputs are unchanged.
[Timer and append code](../../../src/asm/prt-loader.inc) starts just before UI uploads (after
the font), covers UI, game and speech, and hands its already decoded next tick
to the ordinary presentation clock. No second tracker initialization. ISR
does no MOS calls or output. No early send even at the AY library watermark.
R01 borrows PRT1 on a known-idle FAE startup, returns it disabled and
checks the displaced vector on restoration. The control register is
read-to-acknowledge; do not treat probing arbitrary live timers as passive.

Pi generation 1161: ordinary matched the rollback; candidate SHA-256
`e24b6e9dd15a9568c21989195732be8f7a8448c939489caaafbf16295cf3920c`.
Oryx generation 589: same pinned FAE/stock VDP/RC3, serialized headless
runs, dummy audio, no boost change. Both cards ran from MOS twice, exiting
with ESC. Candidate reached the normal controller prompt on both runs.
[Verifier](check-prt.py) checks the returned byte and append counts against
the three actual RIFF files. [Local run evidence](../../../.build/mus001-prt-r01/).

| First-run report | Control | Candidate |
| --- | ---: | ---: |
| MOS clock units across load | 1206 | 1240 (+2.8%) |
| PRT interrupts / serviced ticks / extra periods missed | off | 515 / 514 / 1 |
| Worst observed service delay | — | 1824 PRT counts (25.33 ms nominal) |
| Accounted payload / independent appends | not instrumented | 821839 bytes / 1810 |

Timer/vector teardown check passed, packet overflow clear. Do not claim
hard real-time, physical UART performance, or audible acceptance. No catch-up
bursts: the overdue tick plays once, and the extra period is counted.
The timing file is create-only: the second RUN returned to MOS but did not
replace the first report. Rename `LOADPRT.BIN` at MOS between runs if both
reports are wanted. A single timing pair is not a benchmark distribution.

Pi and automated Oryx slots released; next agents notified. Separate
untouched listening profile `~/Agon/mystuff/nurples/.emulator/mus001-prt-review-r01`
staged with full FAT readback; human-review FIFO request `7a10e2fc84f0`.
After checkout, Smith launches the profile wrapper and types RUN. Listen
especially across the splash and the controller prompt. Before extending
to gameplay, attribute the >20 ms service gap (SD read, VDP operation or
work between callbacks); R01 does not identify which boundary caused it.

**Listening review ready.** Nurples checked out Oryx generation 591 after a
fresh idle inspection. Ownership, runtime, MOS, staged card hash, load-only
autoexec and candidate binary read directly from FAT all verified. FAE was
left stopped for Smith. Hold the slot until he closes FAE and reports back.

Smith accepted the FAE listening audition. Generation 591 released after
a fresh empty-host inspection; no next request. At Smith's request the
exact candidate replaced the Console8 card install at `/mystuff/nurples`.
All five runtime files verified after synchronization. Previous install
retained in `.build/mus001-prt-r01/c8/previous.tar`; autoexec preserved,
no firmware changes.

**Console8 listening result — plays, but tempo is uneven during upload.**
Smith heard tempo changes while assets were still loading that he did not
hear in FAE. He explicitly distinguished these from the expected speedup
when LOADTUNE hands off from the 50 Hz PRT to the frame clock at the prompt.
Slower real UART/VDP throughput is a plausible contributor, not yet a
measured attribution. Next: retrieve the first clean-exit
`/mystuff/nurples/LOADPRT.BIN` and compare missed periods and worst service
delay to FAE. Those aggregate counters cannot distinguish an SD read from a
VDP append, consolidation or progress redraw. If attribution is needed,
time those boundaries separately in RAM; keep diagnostic output off the
UART during loading. Hardware timing acceptance remains open.

**Console8 report retrieved read-only.** Installed binary and three AGNB hashes
match the FAE candidate. [First-exit report](../../../.build/mus001-prt-r01/c8/LOADPRT.BIN)
SHA-256 `2a602b22f0154c677b2802a3680bff688006f05de3241e3ced02ca7dc559d0e9`.
Verifier passed: 821839 bytes in 1810 appends, load complete, no packet
overflow, vector restored. Across the measured upload, Console8 counted
619 IRQs, 548 tracker steps, 71 missed periods (11.5%), 5780 /256 counts
worst lateness (80.28 ms nominal), and 1464 MOS clock units. FAE missed
1/515 (0.2%) with 25.33 ms worst lateness. R01 does not catch up lost
periods: hardware lost 1.42 s of nominal tracker progression. The prompt
clock handoff lies outside these counters. Before tuning chunk size, tag the
intervals bracketing SD reads, VDP appends, record finalization and splash
updates and retain their worst delays and missed-period counts in RAM.

### R02 — RC3 asynchronous asset writes (approved before profiling)

Change only the way the loader sends each asset append. Compare the R01
Console8 result with the same tune, 8 KiB reads, 512-byte maximum payloads,
clock, music service points, graphics and gameplay. No firmware deployment.

- MUS-001-10 [x] Apply tagged Eirene RC3 syscall 25h in a private R02
  fixture. One foreground producer, one positive immutable source at a time;
  callback releases the borrow, not the wire. Drain TEMT before the next
  ordinary VDU output. Keep music and file IO out of interrupt context.
  CTS stalls in the MOS ISR can still mask PRT interrupts.

- MUS-001-11 [x] Assemble on the leased Pi, check R01/ordinary identity and
  unchanged assets. Replace the installed game on Console8, retaining R01's
  report and rollback. Use a distinct create-only report for R02, check
  callback/append accounting, elapsed MOS time, missed periods and worst
  lateness on the next physical run. Hold production and gameplay unchanged.

R02 staged on Console8 following freeze
`238f2b439b29644c98bccb8f5cfde9b4398a3a89`. Each positive append is
one immutable 8-byte header plus payload submitted via 25h. Only bookkeeping
overlaps the transfer; foreground joins callback and TEMT before music, SD
reuse or another VDU command. No claim of throughput gain before measurement.
Pi rebuild matched R01 and ordinary pins. Card binary SHA-256
`a1a458a98e4fac98e8456b2cdd7c88efd9dd41cfd276738115eceec95e8d4994`;
all runtime files verified after sync. Original LOADPRT.BIN preserved,
autoexec and firmware untouched. Await first physical R02 exit report
`LOADASY.BIN` (LPR2, 30 bytes; final u24 counts completions).

Console8 R02: Smith heard neither improvement nor regression. Retrieved
LOADASY.BIN read-only; all 1810 callbacks matched 1810 appends, 821839
payload bytes, status 0x09. IRQ/serviced/missed = 621/551/70, worst lateness
44.79 ms, elapsed 1456 MOS units (R01: 619/548/71, 80.28 ms, 1464 units).
The better single maximum does not establish a repeatable transport benefit.

Found the [legacy storage-only measurements](../../../../agon-extender-legacy/agm/design-considerations.md#measured-sequential-read-ceiling)
for the same blocking ffs_fread API: 8 KiB at 196.54 KiB/s implies 40.7 ms
per call on average on that setup. The current containers require 82 full
8 KiB reads across 16 records before counting tails. Suggested next control:
cap each SD read at 1 KiB, check music between reads, retain R02's 512-byte
VDP appends and all other settings. The old 1020-byte benchmark implies
about 5.6 ms per read. These are aggregate-derived estimates, not Console8
per-call traces or proof of attribution.

### R03 — 1 KiB SD reads, stock MOS and VDP

R02 retired by Smith. Ordinary distribution must not require Eirene or
VDPlet. Use R01 as the synchronous comparison, not R02 as the base.

- MUS-001-12 [ ] Remove the async build arm and its transport/callback.
  Cap each AGNB payload read at 1024 bytes, preserve short tails, service
  music between reads and retain 512-byte synchronous VDP appends.
  Keep gameplay and the accepted production rollback unchanged.

- MUS-001-13 [ ] Verify R01 and ordinary binary pins, stock ABI references,
  unchanged asset hashes and R03 card readback. Preserve prior reports;
  separate create-only LOAD1K.BIN. No firmware writes. Obtain a Console8
  timing/listening verdict; distinguish stock compatibility by inspection
  from a run under identified stock firmware.

R03 staged and verified on the Console8 SD, contract
`8b35f4fbfb1445192cbd748d3a1c17da12ce5da3`. Pi generation 1165:
ordinary and synchronous R01 controls reproduced exactly. R02 submit/join
and callback are absent from the loader and its build arms. Only historical
diagnostics/report parsing retain the RC3 experiment. R03 binary SHA-256
`519cd653e7c5555d170169f573b4a2873e18b7866b7bf2979ed140372a9597c6`.
The timer uses upstream mos_setintvector; reads use upstream ffs_fread;
VDP buffer writes use ordinary RST 18h. Both earlier timing reports were
restored byte-for-byte, no firmware or autoexec changes. Physical result
pending. For whole-game stock qualification also exercise attract/Game Over
and exit: presentation_renderer_enter calls overlay_feature_begin and the
old restore-path comment cites VDPlet or a compatible fix. Not a new
loader dependency; do not infer whole-game firmware qualification from this build.

**R03 Console8 pass.** Smith heard only a brief slowdown at the text-to-splash
boundary, accepts it, played through to Game Over, and exited to MOS. First
report: 1478 MOS clock units, 633 IRQs, 630 serviced, 3 missed (versus 71
on synchronous R01), 5482/72 = 76.14 ms peak lateness. All 821839 bytes
and 1810 appends accounted for, vector restored. The aggregate does not
locate the peak. No firmware identity was reported for this playthrough.

### R04 — send each 1 KiB SD read as a single stock VDP append

- MUS-001-14 [x] Keep the qualified R03 rollback, read size, safe music
  service before/after a whole append, and short final reads. Eliminate the
  inner split loop. Compare a separate Console8 fixture/report against R03;
  do not change the presentation transition, gameplay clock, or firmware.

R04 built against frozen R03 checkpoint
`f76db57649b3e0ee64a3077b23aa4d25784b52db`; ordinary/R01/R03 binary
pins reproduced. Console8 card readback matched candidate
`9d33340d79a9b5711d05306f0c857691225350ab260735067bd20e433f0529ce`,
unchanged assets, and all three earlier reports. LOAD1KW.BIN (LPR4)
should account for 1097 whole-read appends, still 821839 bytes. Await
hardware verdict against R03 (3 missed); no production promotion.

**R04 accepted; production promotion.** Smith heard no material difference
and selected the whole-append version. Console8 LOAD1KW.BIN verifies 1097
appends / 821839 bytes, 629 IRQs, 624 tracker ticks, 5 missed, nominal
87.07 ms worst, 1490 MOS units. R03 had 3 missed, 76.14 ms worst and 1478
units. Adequate, not a measured speed improvement. Publish the normal build
on dev byte-identical to R04, retaining private tunes/binaries out of Git;
the card already has that version. No firmware or card rewrite.

- MUS-001-15 [ ] Next tranche, after this promotion: drive music during
  controller selection/attract, level play, death, respawn and Game Over at
  50 Hz independent of rendering. Preserve silent warp, pause position,
  track transitions, speech/SFX and game speed; test exit and repeated RUN.

Production checkpoint: build_00_all_the_things uses the shared R04 loading
configuration. Compared its prepared assembly tree against the frozen recipe
at 8ee7de0: all 117 files matched except one comment. Promoted the actual
hardware-accepted binary/symbols into tgt; all six hashes matched the R04
manifest. No separate compiler run claimed; the queued rebuild was withdrawn.
No card change. MUS-001-15 awaits Smith's go-ahead.
