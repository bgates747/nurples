# VOX-001 — British announcer for Purple Nurples

Try the British English voice from the Oryx spoken alerts as the game's
announcer. First deliver four Agon-playable samples and matching listening
previews. Then audition them in context, with shield warnings tied to the
colors the player actually sees. Owner: Nurples. Smith approved the sample
tranche on 2026-11-21: stop after delivering WAV previews and wait for
listening review before wiring any cue into the game.

## Cues

| Event | Exact spoken words | Proposed trigger |
| --- | --- | --- |
| Welcome | Welcome to Attack of the Purple Nurples | Once when the graphical loading screen finishes and awaits input, alongside LOADTUNE. |
| Game Over | Game over | Once on entry, not on each flash of the prompt. |
| Yellow shield bar | Shields depleted | Once on a downward crossing into yellow. |
| Red shield bar | Shields critical | Once on a downward crossing into red. |

Use the existing voice recipe (gTTS 2.5.4, English, `tld="co.uk"`),
without the alert prefix or desktop playback. Keep the game music, laser and
explosion mix unchanged. If damage skips yellow entirely, play only
“Shields critical”; healing is silent and rearms the corresponding warning.
Initialization, respawn and HUD repaints should not generate warnings.

Smith explicitly confirmed: warn on a change **into a lower color band**,
not on every deduction. The existing HUD in `src/asm/player_cockpit.inc`
uses `bar_yellow = 33` and `bar_red = 17`: green 33–64, yellow 17–32,
red 0–16. Use the HUD constants, not independent speech thresholds.
Acceptance sequence: 34→33 silent; 33→32 “Shields depleted”; 32→31
silent; 17→16 “Shields critical”; 16→15 silent. A 40→12 hit produces
only “Shields critical.”

## Deliverables and review

- VOX-001-01 [x] Freeze the reviewed contract on `dev` before implementation.

- VOX-001-02 [ ] Render all four lines using the reference voice. Provide
  mono PCM in the Agon's supported sample encoding, with an explicit sample
  rate and byte length per clip (target signed 8-bit PCM at 16,384 Hz,
  the stock VDP default). Provide WAV previews made from the **final**
  PCM so listening review covers the conversion. Record the generation recipe,
  clip checksums and playback settings. Keep experimental audio local until
  publication is decided; no online dependency at game runtime.

- VOX-001-03 [ ] Trace the shield update and HUD band selection. Hook genuine
  downward band changes, not a per-frame color test. Check threshold equality,
  repeated hits within a band, a direct green-to-red hit, healing and respawn.

- VOX-001-04 [ ] In an isolated audition build, play welcome and Game Over
  once per entry and use a voice channel distinct from music and combat SFX.
  A critical warning must not wait behind a stale depleted warning. Verify
  clean stop on exit and no loss of music or laser/explosion audio.

- VOX-001-05 [ ] Smith reviews pronunciation, intelligibility, timing and
  balance. Stop before production promotion, card deployment or changing
  the Oryx alert service. No emulator slot is needed to hear the WAV previews.

Recovery: the ordinary campaign build and Console8 installation stay intact;
any audition outputs go in a separate ignored build directory. The Oryx
alert implementation is a read-only voice reference, not a game dependency.
