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
