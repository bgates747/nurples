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
