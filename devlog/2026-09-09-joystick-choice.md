# Optional joystick on the splash

ASM-011 implements the Author's requested `Enable joystick (y/n)?` choice
before the existing `Press any key to continue.` prompt. The choice is
case-insensitive, rejects other keys, waits for all held keys to be released,
and is asked afresh on each game initialization, including a later MOS RUN.

The investigation compared main `4c01651` with repair dev `a4dd2ad`:
`player_input`, `player_alive`, and `move_sprite` were identical. Both Nurples
and AgonWolf3D test keyboard-map byte 7, bit 1 for Up. Nurples additionally
interprets PC3 low as joystick Down. With Extender's UART wired to Port C,
the P4 drives that pin low as its RTS/Agon's CTS ready signal, so Down cancels
the keyboard's upward velocity. PC1 is also a UART signal, and the separate
PD5 joystick-fire read must be optional too.

The MOS contracts used are `mos_getkey` for the blocking character choice and
`mos_getkbmap` for the 16-byte held-key map. See the read-only official
`agon-docs/docs/mos/Keyboard.md`, `agon-docs/docs/mos/API.md`, and MOS v3.0.2
`src/keyboard.asm`; Up's source mapping is KEY(058). Zilog's eZ80F92/F93
PS015317-0120, page 43, specifies that GPIO data reads sample the physical pin
in every mode. Extender's r03 wiring README identifies PC3 as Agon CTS from
P4 GPIO11. This finding does not alter either processor's UART contract.

The Author proposed a small self-modifying gate to preserve the existing input
routine. Two NOPs are baked into each of the direction and fire paths. Choosing
N writes a two-byte JR at each gate; the skipped direction read leaves D at
0xff (all joystick directions released). Choosing Y restores all four NOP
bytes. Only splash code patches these locations, before gameplay; every store
is one byte, and assembly guards reject displacements beyond the forward JR
range. No per-direction option comparisons or GPIO reconfiguration were added.

Validation completed:

1. Canonical assembly-only build passed; game image is 36,439 bytes, an increase
   of 127 bytes. Matching symbols were regenerated.
2. All 40 existing `test_asm_safety.py` checks passed.
3. `test_player_input.py` executes the actual input and patch routines in the
   headless eZ80 runner. Nine cases cover the original PC3/Up cancellation,
   corrected Up movement, all-low GPIO with no phantom movement/fire, both
   keyboard diagonals and their sprite-frame selections, Space, and repeated
   disable/enable/disable choices. The emitted gates also start as NOP pairs.
   Its MOS/VDP observation stubs do not qualify the interactive splash or
   actual joystick hardware. The fixture is never a physical MOS application.
4. Ruff passed for the new Python test; the diff check passed with
   `git -c core.whitespace=cr-at-eol diff --check`, preserving the assembler's
   existing CRLF symbol-file format.

Prepared a new ignored project-local emulator profile through the canonical
setup tool, with the required generated launcher and pinned MOS. Its runtime
uses the rebuilt game plus the existing AGNB containers and font. The Author's
visual review of the prompt, release/start behavior, and keyboard gameplay is
pending at that checkpoint. No commit or push was made; the prior
deployment-manifest edits remain intact.

## Emulator acceptance and SD deployment

The Author confirmed that the change worked in the emulator and requested SD
deployment. ASM-011's implementation and emulator review are accepted; its
completed item was removed from TODO. Hardware behavior is not inferred from
that emulator report.

The canonical `nurples-runtime` bundle deployed the exact reviewed 36,439-byte
game image, both existing AGNB containers, and the runtime font to
`/mystuff/nurples`, with `nurples.bin` directly in that directory. Source and
the redundant `tgt` directory are excluded. The deployer verified and synced
the files; the root `autoexec.txt` was preserved byte for byte.

Deployed `nurples.bin` SHA256:
`63c89d2676f6645631b2240f23002b8b68d45069979a4804ca7d14f34e720088`.
No firmware update, commit, or push accompanied this deployment.

## Hardware acceptance

The Author reported that the SD-deployed change passed on the Agon and
explicitly requested committing and pushing it. This accepts the joystick-off
keyboard gameplay fix on hardware as well as the earlier emulator review.
The card was safely unmounted after deployment. The reviewed image and source
remain unchanged; no additional build or firmware operation was needed.
