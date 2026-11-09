# Hardware-sprite overlay source audit

This note freezes the source-derived integration plan for the game-over and
attract overlays. It is not a runtime qualification and does not authorize a
hardware-sprite dependency in the production game by itself.

## Selected platform and API

New Alexe work uses the project firmware pin recorded in
`devlog/2026-11-08-pinned-firmware-audit.md`: VDPlet
`0.1.2-alpha-async-r1`, binary SHA-256
`dc43ecb8b1c324ffd1a032e6cc0c56d8c05dd33a355ce170c23adb35ffb533ab`,
with Eirene MOS `3.1.0_uart0_rc_3`, SHA-256
`d7a51147f3381cd68d56f3fbe4605434a897dfae1b049269afeba09ec2b7eb50`.
The release preserves VDPlet presentation profile 8. The ordinary sprite
commands below are mainboard VDU commands sent through the game's existing
synchronous `RST 18h` route; they are not native VDPlet submissions.

The relevant official command surface is:

| Purpose | Bytes |
|---|---|
| Set hardware-sprite test flag | `23,0,F8,02,00,00,00` (variable 2, value 0) |
| Clear hardware-sprite test flag | `23,0,F9,02,00` |
| Select sprite | `23,27,4,id8` |
| Clear selected frames | `23,27,5` |
| Add 16-bit bitmap frame | `23,27,26,idLE16` |
| Activate contiguous prefix | `23,27,7,count8` |
| Select frame | `23,27,10,frame8` |
| Show / hide | `23,27,11` / `23,27,12` |
| Move absolutely | `23,27,13,xLE16,yLE16` |
| Mark hardware / software | `23,27,19` / `23,27,20` |

Hardware sprites require RGBA2222 or RGBA8888 frames. They move immediately,
are not clipped by the graphics viewport, and do not need command 15 refresh.
Higher sprite IDs draw above lower IDs. RGBA2222 alpha is effectively binary in
the scanline compositor, matching the overlay generator's deliberate alpha
contract.

The exact local references used for this audit are:

- `/home/smith/Agon/agon-docs/docs/vdp/Bitmaps-API.md`;
- `/home/smith/Agon/agon-docs/docs/vdp/System-Commands.md`;
- `/home/smith/Agon/agon-docs/docs/vdp/VDP-Variables.md`;
- `/home/smith/Agon/mystuff/agon-vdplet/src/vdplet_vdu_validate.h`;
- `/home/smith/Agon/mystuff/agon-vdplet/tests/check_sprites.cpp`;
- `/home/smith/Agon/mystuff/agon-vdplet/docs/tasks/VDPL-024/MECHAGON-SPRITE-SOURCE.md`.

## Nurples ownership and layout

The existing actor pools own one contiguous prefix of 20 sprite IDs:

| Pool | Count | IDs |
|---|---:|---|
| player lasers | 4 | 0–3 |
| player | 1 | 4 |
| seekers | 3 | 5–7 |
| fireballs | 4 | 8–11 |
| explosions | 8 | 12–19 |

`sprites_init` resets the sprite engine, defines every slot and activates that
20-sprite prefix. The overlay allocation therefore reserves:

- sprite 20: `GAME OVER`, with two colour frames;
- sprite 21: prompt, with `CONTINUE` and `START` frames (only one phase uses it
  at a time).

Overlay entry defines both slots while they are outside the active prefix,
selects normal paint mode 0, marks them hardware, positions them in physical
screen pixels, hides any sprite not yet ready, and only then activates 22
sprites. Overlay exit hides both, restores the active prefix to 20, clears the
two owned frame lists, demotes both owned slots to software, and only then
releases their backing buffers.

The game runs in physical mode 20, 512x384. Its central playfield begins at
physical `(128,48)` and is 256 pixels wide. A 240x64 title therefore centers at
physical X=136; a 160x16 prompt centers at X=176. Their Y positions must not
overlap, both to keep the wording legible and to ensure the scanline compositor
sees at most one wide overlay on any row. Hardware sprites do not inherit the
256-pixel graphics viewport, so code must validate the full physical extents
before every placement.

The current source masters are 240x64 and 160x16, exact Agon64 colors with
binary alpha. Smith subsequently selected the original Lode Runner uppercase
font; those masters remain a rollback until the 6502 agent supplies a
provenance-pinned extraction. The replacement must preserve source pixels and
use integer nearest-neighbor scaling only—never bilinear, bicubic, quadratic,
antialiasing or palette reduction.

The extraction is also intended to become the game's canonical 8x8 runtime
font, not merely private overlay geometry. Its Agon `.font` is exactly 256
glyphs × 8 rows = 2,048 bytes. If the source really contains only uppercase,
copy each accepted A–Z bitmap into both its uppercase and lowercase ASCII slots
so existing mixed-case strings deliberately render in all caps without runtime
case conversion. Preserve the current `Lat38-VGA8_8x8` cells as fallback for
punctuation, digits and every source slot Lode Runner does not define; do not
invent missing glyphs during extraction. The overlay generator then reads the
same frozen 8x8 bit rows used by the VDP font and expands pixels by integer
replication. This keeps loading/status text and overlay lettering visibly one
family while retaining a complete 256-character Agon font.

## Remaining qualification gates

1. **Feature-variable preservation.** Variable 2 is enabled by *presence*, even
   when its value is zero. Clearing it does not demote existing sprites. The
   production game has no qualified variable-readback helper today, so it must
   not assume the incoming flag was absent. Add and qualify an exact
   presence/value snapshot and restoration path, or establish a narrower
   process-wide baseline explicitly. Unknown incoming state is not clean
   handback.
2. **Runtime font and assets.** After the font extraction is accepted, generate
   and verify the 2,048-byte hybrid `.font`, its metadata and a 16x16 proof
   atlas. Make it the sole selected runtime font only after a loading/gameplay
   visual review. Add the four immutable overlay RGBA2222 images to the ordinary
   AGNB catalog with collision-free 16-bit buffer IDs. Keep their buffers alive
   until both overlay sprites are detached.
3. **Standalone Alexe proof.** Before production integration, run a bounded
   fixture on the pinned Alexe profile in mode 20. It must show both title
   frames, each prompt frame, transparency over a moving background, immediate
   move/show/hide behavior, correct higher-ID ordering, and clean repeated
   setup/teardown. Inspect for scanline instability with each 240- and 160-pixel
   sprite separately; do not infer wide-sprite capacity from slot count.
4. **Lifecycle proof.** Production qualification must repeat GAME OVER → new
   game and TITLE → new game cycles, proving the active prefix returns to 20,
   slots 20/21 are software with no frames after teardown, backing buffers are
   released only after detachment, and feature variable 2 is restored exactly.
5. **Target confidence.** Alexe is the first correctness gate. Real Console8
   confirmation remains necessary before declaring scanline capacity and
   cleanup portable to hardware.

Until these gates pass, `GAME-001J` and `GAME-001K` may consume the artwork and
this ID plan in design/tests, but the production game must keep its current
20-sprite software prefix and must not set feature variable 2.
