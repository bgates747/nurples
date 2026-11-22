# MUS-001 R07 — longer, louder shield klaxon

Smith could hear Rally's revised speech over the music and effects, and liked
the whoop before the shield warnings. Keep that speech bank for this audition.
Make the klaxon itself last four times as long and raise only its volume.

- MUS-001-22 [x] Keep the existing pitch, timbre and sweep speed. Sound four
  pairs instead of one: 1,280 ms of sweeps and a 176 ms release, four times
  the original 320 + 44 ms. Raise channel 14's note level from 40 to 80 out
  of 127. Do not change master, speech, effects or music levels.
- MUS-001-23 [x] Keep the downward-crossing, critical-priority and cancellation
  rules. Send speech at the first foreground poll safely after the sound ends,
  not by blocking frames or delaying an alert without playing anything.
- MUS-001-24 [x] Assemble and test the outgoing stock VDP packet, including
  the audible pulse count, envelope lengths, gain, unchanged pitch contour
  and first safe speech tick. Stage a new stopped FAE profile with the verified
  louder speech bank; leave the R06 profile/card intact. Smith judges the mix.

Continuation of the R06 cue contract at `c0d6cbb`. The private R06 controls
are in `.build/mus001-prt-r06/` (voicelevel and combat); keep existing
uncommitted integration out of the plan commit. No firmware, physical SD,
or production promotion in this iteration.

**Result.** Frozen at `7a427057b9bcc9ab50cef92fe095b685f179d04f`.
Pi 1174: ordinary private game SHA-256
`7f6b4a809cde1bdff6b658ef3961141769591251cdab630ce366af0f66bad3e6`;
232 tests, one skipped. The linked-packet test verifies all eight pulses,
the pitch contour, 80/127 note level, 1,456 ms nominal duration and 74-edge
speech threshold. The previous 14 FAE readbacks apply to the sequencing logic
in R06; no claim of a second headless run or of measured output loudness.
R07 readback passed with the same louder speech bank. **FAE listening PASS:**
Smith liked the longer, louder klaxon and could clearly hear the spoken shield
warning after it. FAE closed; Smith explicitly retained Oryx checkout 599
for recording a session of the same game. No Console8 or promotion claim.

- MUS-001-25 [x] Prepare a one-command OBS recording of the unchanged R07
  FAE profile: record the game window and game sound, start automatically,
  finalize a local video when Smith closes FAE, and leave the reservation
  held. Do not enable streaming, microphone, or desktop capture.

**Recording ready.** On Oryx, from `.emulator/mus001-prt-klaxon-r07`,
run `./record-fae`, not the normal launcher as a second command. At the
recording-ready message type `RUN` in MOS. Keep FAE visible and at its
launch size. Closing its window finalizes a local, timestamped MKV under
`~/Videos/Purple-Nurples/`; the terminal prints the exact path. No game changes.
Source: [record-fae.py](../../../tools/record-fae.py).

Verified on Oryx using the actual R07 game on a disposable card copy and
a private X display/audio server. Closing FAE through the window manager
produced a 29.75 s H.264 960×720/60 + AAC file. Decoded the splash frame
and measured non-silent game audio (peak −6.5 dBFS). OBS confirmed recording
stopped and exited. An earlier XComposite probe had encoded black frames;
the selected OBS media source explicitly grabs the FAE window ID using X11,
and follows only the game process group's audio into a private speaker sink.
No microphone or desktop scene.

**Smith's recorded playthrough:** on Oryx at
`/home/smith/Videos/Purple-Nurples/2026-11-21_22-30-13-dadlvggk/Purple-Nurples.mkv`.
Finalized MKV: 416.884 s, H.264 960×720/60 and stereo AAC at 48 kHz.
Decoded frames at 25, 145, 280 and 394 s show gameplay and Game Over.
The game-only audio track was present at all four checks; full-track peak
−1.9 dBFS. OBS stopped recording but its private process stayed open after
finalization. Verified it no longer held the MKV, stopped that instance
only, then released checkout 599 with no emulators or waiting request.
