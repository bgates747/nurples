# MUS-001 R07 — longer, louder shield klaxon

Smith could hear Rally's revised speech over the music and effects, and liked
the whoop before the shield warnings. Keep that speech bank for this audition.
Make the klaxon itself last four times as long and raise only its volume.

- MUS-001-22 [ ] Keep the existing pitch, timbre and sweep speed. Sound four
  pairs instead of one: 1,280 ms of sweeps and a 176 ms release, four times
  the original 320 + 44 ms. Raise channel 14's note level from 40 to 80 out
  of 127. Do not change master, speech, effects or music levels.
- MUS-001-23 [ ] Keep the downward-crossing, critical-priority and cancellation
  rules. Send speech at the first foreground poll safely after the sound ends,
  not by blocking frames or delaying an alert without playing anything.
- MUS-001-24 [ ] Assemble and test the outgoing stock VDP packet, including
  the audible pulse count, envelope lengths, gain, unchanged pitch contour
  and first safe speech tick. Stage a new stopped FAE profile with the verified
  louder speech bank; leave the R06 profile/card intact. Smith judges the mix.

Continuation of the R06 cue contract at `c0d6cbb`. The private R06 controls
are in `.build/mus001-prt-r06/` (voicelevel and combat); keep existing
uncommitted integration out of the plan commit. No firmware, physical SD,
or production promotion in this iteration.
