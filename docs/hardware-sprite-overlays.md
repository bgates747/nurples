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
physical X=136; a 208x16 prompt centers at X=152. Their Y positions must not
overlap, both to keep the wording legible and to ensure the scanline compositor
sees at most one wide overlay on any row. Hardware sprites do not inherit the
256-pixel graphics viewport, so code must validate the full physical extents
before every placement.

The current source masters are 240x64 and 208x16, exact Agon64 colors with
binary alpha. They use the provenance-pinned original Lode Runner uppercase
font extracted by the 6502 project from the running game's RAM sprite table at
`$AD00`. The verified 10x11 source carries three blank top rows and its eight
art rows fit exactly in one byte for A-Z, so the 8x8 adaptation is a lossless
crop rather than a resize. Enlarged overlay pixels use integer replication
only—never bilinear, bicubic, quadratic, antialiasing or palette reduction.

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

## Exact feature-variable preservation design

The official buffered-command API can preserve variable 2 without adding a
CPU-side response protocol. Conditional call command 6 performs a true VDP
variable-presence test, including the distinction between absent and present
with value zero. Command 48 can then copy the variable's 16-bit value directly
into another VDP buffer. The frozen transaction uses these currently unused
IDs:

| ID | Role |
|---:|---|
| 63984 | one-byte original-presence marker, initialized to zero |
| 63985 | prepared `VDU 23,0,F8,2,value16` restore-set command |
| 63986 | prepared `VDU 23,0,F9,2` restore-clear command |
| 63987 | mark-present program: set marker to one, then read variable 2 into the restore-set value bytes |
| 63988 | snapshot dispatcher: conditionally call 63987 only if variable 2 exists |
| 63989 | restore dispatcher: call 63985 when the marker is nonzero, otherwise call 63986 |
| 63990–63993 | two title and two prompt bitmap buffers |

The production image manifests currently end at 2816, the sole runtime font is
64000, and the existing loose-image temporary buffer is 65534. Startup already
clears every VDP buffer before loading application assets, so these six control
buffers do not need to preserve an earlier program's buffer contents. They do
need to remain allocated until overlay teardown has restored the incoming
feature-variable state.

Overlay setup writes the marker, both restore commands, the mark-present
program and both dispatchers, then calls the snapshot dispatcher. Only after
that call returns may it set variable 2 and mark sprites 20/21 as hardware.
Overlay teardown hides both sprites, restores the active prefix to 20, clears
their frame lists, demotes them to software, releases their four bitmap
buffers, calls the restore dispatcher, and finally clears control buffers
63984–63989. It must not clear a dispatcher while that dispatcher is running.

The standalone qualification fixture must begin from each of these incoming
states and prove exact state after repeated setup/teardown: variable 2 absent,
present with zero, and present with a nonzero sentinel such as `0xBEEF`. It
must also prove the marker is reset for each new transaction, a prior present
run cannot contaminate a later absent run, and every control/bitmap buffer is
released after restoration. This is a frozen implementation design, not target
evidence; it still requires pinned Alexe and real Console8 qualification.

## Remaining qualification gates

1. **Feature-variable preservation.** Variable 2 is enabled by *presence*, even
   when its value is zero, and clearing it does not demote existing sprites.
   Implement the frozen VDP-resident transaction above and qualify all three
   incoming-state cases. Unknown incoming state is not clean handback, and the
   design alone is not permission to set the flag in production.
2. **Runtime font and assets.** The 2,048-byte hybrid `.font`, metadata and
   16x16 proof atlas are generated and byte-tested. Make it the sole selected
   runtime font only after a loading/gameplay visual review. Add the four
   immutable overlay RGBA2222 images to the ordinary AGNB catalog with
   collision-free 16-bit buffer IDs. Keep their buffers alive until both
   overlay sprites are detached.
3. **Standalone Alexe proof.** Before production integration, run a bounded
   fixture on the pinned Alexe profile in mode 20. It must show both title
   frames, each prompt frame, transparency over a moving background, immediate
   move/show/hide behavior, correct higher-ID ordering, and clean repeated
   setup/teardown. Inspect for scanline instability with each 240- and 208-pixel
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
