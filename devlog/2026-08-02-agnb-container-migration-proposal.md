# AGNB bitmap-container migration proposal

Date: 2026-08-02

Status: approved design contract; implementation has not begun.

## Purpose

Purple Nurples currently opens and seeks through 396 individual RGBA2222
bitmap files during startup. The intended replacement is the established AGNB
RIFF container system used by AgonWolf3D and Wolf3DOrig. This is a storage and
startup-loading migration only: renderer-visible buffer IDs, image dimensions,
and payload bytes must not change.

The current VDU font format is mature and deliberately remains outside this
migration. The one active runtime font, `Lat38-VGA8_8x8.font`, will continue to
be loaded as a loose font file. No new AGNB font form will be designed, and no
upstream font-producing workflow will be disturbed.

## Reference implementation decision

Use the canonical image loader from
`/home/smith/Agon/mystuff/agon-utils/examples/agnb/api/agnb_api.inc` as the
authority. AgonWolf3D currently carries that implementation byte-for-byte.
Wolf3DOrig's later copy uses the same container layout and loader architecture;
its differences are application dependency integration rather than a newer
image-container format.

The repair repository's Python 3.14 environment already installs canonical
`agonutils` 1.1.0 in editable mode from
`/home/smith/Agon/mystuff/agon-utils`, and `pip check` passes. Image conversion
continues through that canonical installation. Purple Nurples must not
reimplement or vendor the native `agonutils` package or its palette logic.

## Deployable layout

The generated application will contain:

- the existing loose `fonts/Lat38-VGA8_8x8.font`;
- `ui.agnb`, containing the 46 active UI bitmaps; and
- `game.agnb`, containing the 350 active game bitmaps: four backgrounds, 159
  sprites, and 187 DG tiles.

The failed music experiment remains removed. Although the canonical AGNB API
supports audio containers, Purple Nurples currently has no active sample asset
catalog, so this migration will not generate an empty or speculative audio
container.

## Startup sequence

1. Clear VDP buffers.
2. Load the sole loose VDU font and emit one console breadcrumb only after its
   buffer has been uploaded and converted into a VDP font. This preserves the
   useful "working, not crashed" feedback without changing the font format.
3. Load `ui.agnb` without the marching-logo callback. This container supplies
   the splash background and logo needed by the following phase.
4. Establish the loading display.
5. Load `game.agnb` with an application callback invoked after every finalized
   bitmap.
6. On each of the 350 callbacks, redraw the splash, advance and draw the logo,
   and present the frame only after the AGNB loader has completely uploaded,
   consolidated, selected, and created the current bitmap.
7. On success, continue through the existing keypress and gameplay setup. On
   failure, stop before gameplay, display the AGNB error code, and return to MOS
   through an explicit acknowledged path.

Progress drawing must occur only between completed VDP upload blocks. No VDU
output may interrupt a command-0 payload transfer.

## Exact progress displacement

Purple Nurples already contains the intended 16.8 fixed-point accumulated-logo
approach. Its current divisor is wrong: it uses `sprites + tiles + UI = 392`,
while the animated phase actually performs `backgrounds + sprites + tiles =
350` loads.

The migration will derive the divisor from the exact `game.agnb` record count.
The callback will accumulate the fixed-point displacement once per finalized
record. The last callback will explicitly place the logo at its defined final
coordinate so integer-division remainder cannot leave it short of the endpoint.
The record count used by the packer, generated assembly metadata, verifier, and
progress initializer must have one source of truth.

## Build-side contract

Create one deterministic catalog for each container. Every catalog record will
contain the asset family, stable name, existing buffer ID, width, height,
RGBA2222 source path, and expected payload size. Container writers and generated
assembly metadata must consume those catalogs rather than independently walking
directories or assigning IDs.

The packer will emit canonical AGNB 0.1 records:

```text
RIFF AGNB
  VERS 0,1
  LIST BUFR
    BHDR bufferId
    IMAG width,height,format=1
    DATA unchanged RGBA2222 bytes
```

All RIFF sizes are little-endian 32-bit values and every chunk receives
four-byte zero padding. Existing image buffer IDs remain explicit in `BHDR`;
the runtime loader must not derive, allocate, increment, or remap them.

The build must remain incremental. An assembly-only build must not regenerate
assets unexpectedly. Any selected asset, dimension, buffer-ID, or payload
change must invalidate and rebuild its affected container. A full build that
recreates `tgt/` must also recreate both containers before assembly or staging.

Once the container path is proven, generated pathname strings and loose-load
records may be removed. Constants and runtime lookup tables consumed by the
renderer must remain.

## Verification gates

An independent parser, not merely the writer's own helpers, will verify each
generated container before assembly or deployment:

- RIFF/AGNB identity, declared total size, version, record count, and order;
- valid four-byte alignment and zero padding;
- unique, non-`0xFFFF` buffer IDs;
- exact IDs, dimensions, and format against the authoritative catalog;
- `DATA size == width * height` for RGBA2222; and
- byte-for-byte equality between every `DATA` payload and its source file.

The existing Python environment checks, unit tests, Ruff checks, canonical
`agonutils` test, and eZ80 assembly must pass. The result will then be staged
under `/tmp/nurples` and launched through the durable isolated emulator profile
for Author review.

## Change-control boundary

Implementation will occur only in the persistent `nurples-repair` repository
on `dev`. The canonical Purple Nurples checkout remains untouched. The work
will remain uncommitted and unpushed until the Author has visually qualified
the emulator result and explicitly approves a commit. Integration into `main`
requires a later deliberate instruction.
