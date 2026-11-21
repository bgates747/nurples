# MUS-001 — Audition tracker music in the Nurples game loop

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
