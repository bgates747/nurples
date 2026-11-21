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

- VOX-001-02 [x] Render all four lines using the reference voice. Provide
  mono PCM in the Agon's supported sample encoding, with an explicit sample
  rate and byte length per clip (target signed 8-bit PCM at 16,384 Hz,
  the stock VDP default). Provide WAV previews made from the **final**
  PCM so listening review covers the conversion. Record the generation recipe,
  clip checksums and playback settings. Keep experimental audio local until
  publication is decided; no online dependency at game runtime.

- VOX-001-03 [x] Trace the shield update and HUD band selection. Hook genuine
  downward band changes, not a per-frame color test. Check threshold equality,
  repeated hits within a band, a direct green-to-red hit, healing and respawn.

- VOX-001-04 [x] In an isolated audition build, play welcome and Game Over
  once per entry and use a voice channel distinct from music and combat SFX.
  A critical warning must not wait behind a stale depleted warning. Verify
  clean stop on exit and no loss of music or laser/explosion audio.

- VOX-001-05 [x] Smith reviews pronunciation, intelligibility, timing and
  balance. Stop before production promotion, card deployment or changing
  the Oryx alert service. No emulator slot is needed to hear the WAV previews.

Recovery: the ordinary campaign build and Console8 installation stay intact;
any audition outputs go in a separate ignored build directory. The Oryx
alert implementation is a read-only voice reference, not a game dependency.

## R01 — samples ready for listening review

Freeze: `15c7149485611c185e3e6f127495f76a144a405b` on `dev`.
The [renderer](render_samples.py) used the Oryx alert environment for
synthesis only. No alert played. FFmpeg converted each response to mono
signed 8-bit PCM at 16,384 Hz with 2 dB of headroom. The WAVs wrap these
samples with the unsigned bias required by 8-bit WAV; reversing that bias
reproduced every raw clip byte for byte.

Local [manifest with clip hashes and playback settings](../../../.build/vox001-announcer-r01/samples.json).
Welcome: 50,332 bytes; Game Over: 20,447; Shields depleted: 30,671;
Shields critical: 28,312. The ZIP of raw samples and manifest passed its
integrity check. No game code changed and no playback on Agon hardware
is claimed. Next: Smith listens to the four WAVs before any integration.

## R02 — full-game FAE listening audition, approved 2026-11-21

Smith accepted the voice and authorized the four cues above in a private
campaign build. Add the second metal strike from Rally's A/B preview (hollow
hull, not bright anvil) once per registered enemy air/ground contact with
the ship. Keep laser kills, scoring, damage, laser fire and the player's
final explosion unchanged. Shield speech follows downward color crossings
only; multiple drops in one frame must not leave a stale yellow warning.
Use separate speech and impact channels. Retain the R01 samples unchanged
and compare the fixture against the ordinary campaign. Stage a stopped FAE
profile on Oryx with RC3 MOS and stock VDP and give Smith the launch command.
No Console8 change or production promotion before listening review.

R02 freeze: `6179692636d8aeee67f7abbd7f0fff4e5f6cdb0c`. Built with the
ordinary private campaign as a byte-matched control. Startup loads four
AGNB 0.2 audio records once. Each decoded PCM payload matched its approved
WAV; named buffers FB00–FB03 play at 16,384 Hz on channel 11. The verified
hollow hull patch plays on 8–10 at registered seeker, fireball, electrode
and lightning contacts. Contact destruction retains its visual result
without a duplicate blast; laser kills and the player finale retain the
ordinary blast. All 12,675 combinations of old/new shield values (0–64)
and pending warning (none/yellow/red) passed the compiled branch check.

Pi compilation released. Oryx checkout **584** held for Smith's listening
review. FAE profile `~/Agon/mystuff/nurples/.emulator/vox001-announcer-fae-r02`
is staged and stopped. Pinned FAE/stock VDP/RC3 MOS verified; every guest
payload read back identically, including `voice.agnb`. Audition executable
SHA-256 `c8923ea258d58956995bec847d2a179e5eb1893b49eae1809cb2917c57294c94`.
No production install changed. Pending: Smith's actual listening/mix and
exit verdict; release Oryx after he closes FAE.

## R03 — omit welcome; swap the two existing gameplay cues

Smith revised the audition mix: keep the welcome recording in the original
R01 sample set, but do not package, load or play it. LOADTUNE still plays.
Keep Game Over and the two downward shield warnings. Enemy contact with the
ship gets the existing explosion; a player laser destroying an air or ground
target gets the existing hollow ping. Each event plays once; leave firing,
score, destruction visuals and the player's finale untouched. The separate
tin-roof candidate is not part of this audition. Verify the three-record
voice bank and event routing. Preserve R02 and production as controls.

R03 freeze: `0aa91f237a16631e2685c93b06e8b195522b4af3`. Verified
the three voice records independently against their WAVs (FB01–FB03);
FB00 and its 50,332 sample bytes are absent. The voice bank is 50,376
bytes smaller. The original signed welcome file still matches its R01 hash.
Checked compiled conditional Game Over entry, ship-hit blast, air-kill
ping and crater gating. The ordinary control binary still matches R03
campaign music. Isolated audition binary SHA-256:
`a607169c4431b54b0c34b28cb6e4ba827fc9aa7198d4cb21e9af1fc1c1d0457e`.
Read back all staged guest payloads on Oryx; the R03 FAE profile is
`~/Agon/mystuff/nurples/.emulator/vox001-announcer-fae-r03`. No emulator
or hardware was launched. R02 checkout 584 released after Smith's feedback
and fresh empty-host proof, Flexe notified, R03 queued as `794792370f33`.
Oryx checkout **587** granted for R03 after a fresh idle inspection.
Verified ownership, runtime/firmware manifests, staged card hash and
load-only autoexec. Left FAE stopped for Smith to launch. Await listening
verdict and closure before handback. Rally's standalone FAE tin-roof
capture remains separate; it is not in this profile.

**R03 FAE listening review — PASS, 2026-11-21.** Smith confirmed an
explosion when his ship was hit, a ping when his shot destroyed a target,
and no welcome announcement. He reported FAE closed. Game Over and the two
shield warnings were not individually reported in this run. This is an
audition verdict, not a production promotion.

Oryx checkout 587 was released at 18:48:39 Eastern after a fresh
whole-host inspection found no emulator processes. No request was waiting.

**Production rollback contract.** Smith approved promoting the R03 audition
and publishing a critical rollback commit on `dev`. Preserve byte identity
for the executable and the complete runtime bundle, including the three-cue
voice bank. Make the normal build and deployment manifests reproduce it from
the committed inputs plus the existing private PT3s. Do not publish music or
music-bearing executables, add welcome to the app, change the Console8 card
or move `main`.

**Production result:** the normal `--all` build reproduced all five accepted
guest files byte-for-byte. R02's welcome was deliberately retired in R03,
not lost; its original remains in the private R01 audition files. R03's
listening verdict above is the scope of the human acceptance. The committed
WAVs, hull command bytes, normal assembly hooks, voice-bank builder and both
production deployment manifests are the rollback inputs, together with the
private PT3s documented in MUS-001. See the [rebuild and verification
instructions](../../python-tooling.md#production-soundtrack). No new card
deployment or emulator run. The two previously recorded host-test issues
were corrected in their diagnostic fixtures, without changing game bytes.
