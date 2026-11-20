# GAME-001-EXIT — Console8 does not return to MOS

On the ordinary Console8 build at `8bab625`, Smith reports that exiting
Nurples appears to reset the Agon. The VDP branding reappears, but the MOS
branding and cursor do not, and keypresses get no response. This is a
hardware exit failure, separate from the intermittent Game Over flicker.
The report does not establish which processor reset or where the handoff
stalled. The initial decision was to check the exit problem before changing
the Game Over renderer. The latest comparison below did not reproduce it.

## Next checks

- GAME-001-EXIT-001 [ ] **Establish the exit path and reproduce.** Smith
  confirmed that exit fails from both ordinary play and Game Over.
  Exit after continuing into a second game remains unreported. Keep
  the exact deployed executable and assets as the control and record the
  Console8 firmware identity if available. Compare exit paths independently.

- GAME-001-EXIT-002 [ ] **Locate the failure.** Check the shutdown path
  from overlay teardown and sprite/buffer cleanup through display
  restoration and the return to the caller. If visuals cannot distinguish
  the steps, propose bounded diagnostics that identify the last completed
  step. Do not treat the VDP banner as proof that MOS restarted.

- GAME-001-EXIT-003 [ ] **Qualify the remedy on Console8.** Escape in
  ordinary play and at Game Over must return to a responsive MOS prompt,
  without a reset. Confirm keyboard input and a repeat `RUN` work, including
  after a Game Over → new-game cycle. Preserve SD startup and firmware.

## Initial source trace

Both routes reach the common `main_end` teardown. It shuts down presentation
state (active hardware overlays are torn down conditionally), selects the
system font, resets sprites, clears VDP buffers, restores the entry mode and
text settings, then returns through the MOS calling frame. A live Game Over
overlay is not necessary to reproduce the failure. Start by instrumenting or
observing the shared teardown and mode/return boundary; retain the conditional
overlay branch as a comparison, not the sole suspected cause.

Report and read-only trace only. No code change or new hardware run yet.

## First diagnostic — stock VDP on Console8

Smith requested the same game and MOS with one variable changed: stock
VDP in place of VDPlet. His hypothesis is that VDPlet mishandles memory
when freeing buffers, when restoring the screen mode, or both. The first
comparison can implicate or weaken the firmware hypothesis; it cannot
identify a particular allocator call on its own.

Staged the official [Bistromathics v2.16.0 release](https://github.com/AgonPlatform/agon-vdp/releases/tag/v2.16.0)
`firmware.bin` at `/mystuff/nurples/firmware.bin` on the Console8
`AGONC8` card attached to Oryx: 1,077,840 bytes, SHA-256
`b807beef35823b13a0a056f11b7464cd1b1c6356dce0e4098b78ebe059ded35f`.
GitHub's release asset digest and the
[SD readback](../../../.build/console8-game-over-8bab625/stock-vdp-v2160/sd-receipt.txt)
match. The four deployed game files, ownership receipt, and `autoexec.txt`
verify unchanged. The binary is a one-off test file, not a change to the
runtime bundle definition. No flash, boot, run, or card unmount was performed.

Await Smith's report with Bistromathics confirmed on boot. Repeat the
regular-play and Game Over exits from fresh runs; note whether the VDP
banner returns, whether MOS presents a cursor, and whether a key works.

### Console8 A/B result — November 20

Smith confirmed that the same ordinary build exits to a working MOS
prompt from **both** regular play and Game Over with stock Bistromathics.
Both exits had failed on VDPlet. This strongly points to a firmware-dependent
failure, but does not prove the allocator hypothesis or distinguish buffer
cleanup, mode restore, and link/recovery behavior. The previous flashed
VDPlet identity and first-fault/reset evidence were not collected.

Smith directed VDPlet to draft an investigation task and stop before
execution. [Durable request](/Agon/mystuff/agon-dev-env/cross-agent/agentcoms/messages/2026-11-20_15-41-34_nurples_console8-nurples-exit-plan-only.md)
and immediate chat delivery were accepted. Await the plan for review;
no diagnosis run, instrumentation, firmware change, or bitmap-renderer
work is authorized by this handoff.

## Author-requested mode-restore discriminator — November 20

Smith requested a separate manual Console8 test. Put the latest published
VDPL firmware file at the root of the Console8 SD card, named
`VDPL_<version>.bin`, matching the numeric style of his renamed stock
image but without a branding suffix. Verify the release digest and the
SD readback. Do not flash, change firmware settings, or edit startup.

Build a temporary Nurples variant that skips **only** the final restoration
of the saved entry screen mode in the common exit-to-MOS path. That is the
mode switch relevant to the reported failure; restarting a new game from
Game Over is a separate in-process path. Keep sprite reset, buffer freeing,
the farewell and the actual MOS return intact. Keep the published game as
a byte-verified control. Stage the variant beside it under a distinct name;
do not modify tracked assembly, assets, or the installed ordinary binary.
Verify that the variant differs only at the disabled call.

Smith owns flashing and running the two variants from equivalent fresh
starts. Start with ordinary-play Escape; use the same entry mode for each
arm. A pass with mode restoration omitted and failure in the control
implicates the mode switch or its ordering after buffer free; a failure in
both means the switch is not necessary for the symptom. The variant is
expected to leave MOS in the game's screen mode if it returns. This local
one-variable test does not release VDPlet's VDPL-059 investigation.

### Staged diagnostic

- Console8 SD root: `/VDPL_0.1.3.bin` is the latest published
  [v0.1.3-alpha release](https://github.com/bgates747/agon-vdplet/releases/tag/v0.1.3-alpha)
  application image, selection `0.1.3-alpha-async-r1`. Release
  `SHA256SUMS` and [SD readback](../../../.build/console8-exit-no-mode-r01/firmware-sd-receipt.txt)
  match SHA-256 `90edc4c52f3120c1709e3f09dd675492e8326f64b36061570b3ea4f2a0efeccc`.
  Existing firmware files, including Smith's
  `VDP_2.16.0_Bistromathics.bin`, were not changed.
- SD game directory: `/mystuff/nurples/nurples-nomode.bin`, SHA-256
  `906bde2912f29ad28eb22466fb6fbbdd04afc0417464d486a04379ad288ec504`.
  [Binary diff](../../../.build/console8-exit-no-mode-r01/binary-diff-proof.txt)
  proves that only the four-byte `CALL vdu_set_screen_mode` at
  `$04B732` was replaced by four NOPs. The version-controlled source and
  the ordinary deployed game and assets were not changed. SD readback and
  sync passed; `autoexec.txt` was unchanged and the card was left mounted.

After Smith confirms the selected firmware on boot, run
`CD /mystuff/nurples`, `LOAD nurples-nomode.bin &040000`, `RUN`. Use
`nurples.bin` instead for the control. Start with the same MOS entry mode
in both arms. No flash or device run was performed by Nurples.

### Current firmware comparison — November 20

Smith reports that both the ordinary executable and the no-mode-restore
diagnostic exit cleanly to a working MOS prompt on both VDPlet and stock
Bistromathics. This is four passing firmware/build combinations in the
current manual test; the report does not enumerate separate ordinary-play
and Game Over exits for each combination.

The Console8 control readback was the `8bab625` runtime, with ordinary
binary SHA-256 `1b4941df8bfb9a00b85cec8195bf8d92789082f24274370258f893a941ed94aa`.
The four runtime payloads matched their manifest before and after staging
the diagnostic. Raw receipts and rollback files remain in local `.build/`.

The original failure is not reproduced in this comparison. The ordinary
build passed with mode restore and buffer cleanup still enabled, so the
diagnostic bypass is not needed in the current configuration and should not
enter production. An older VDPlet image may explain the earlier failure,
but its flashed identity was not captured; no version-specific regression,
allocator cause, or repair is established. Retain the diagnostic as
evidence and keep VDPL-059 at its existing plan-only gate.

Reopen device diagnosis if the exit hang recurs. Before changing the test
configuration, record the exact flashed firmware digest, the entry mode,
which escape path was used, and the last clear evidence from VDP and MOS.
The current result is a clean baseline for the Game Over renderer comparison,
not a reason to omit mode restoration from the ordinary game.
