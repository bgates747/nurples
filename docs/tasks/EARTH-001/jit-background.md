# EARTH-001 — Just-in-time Earth background scanlines

Approved FAE audition; owner: Nurples. Instead of uploading Earth’s whole
map at startup, send the next 256 pixels when Earth actually scrolls. Keep
the fast-warp starfield resident and the two facility strips ready for an
immediate cut. The question is whether this saves VDP memory and startup
time **without buying either at the cost of a missed frame or music tick**.
Smith selected one mutable VDP row for the first audition; alternating two
rows is the next option if plot and overwrite overlap. Build and stage the
six-wave diagnostic on Oryx FAE only; leave the ordinary game, Olimex inputs
and production packaging intact. Hardware testing remains a later gate.

## Control and budget

Control: dev `16c94299dbb4af50610937f4ab2453b93dd0091f`, including the
FAE-accepted sprite exits. Use the same six-wave map, audio and sprite
workload for the comparison. In `nurples_tools/current_assets.py`,
`earth_records` already bakes structures into sixteen ordered 256×256
RGBA2222 records in `game.agnb` (IDs 1280–1295). Spawn markers remain
compiled map bytes. In `src/asm/tiles.inc` an actual one-pixel terrain scroll
opens a 256×1 viewport and calls the renderer bound by
`src/asm/earth_opening.inc`. The framebuffer scroll carries previous rows;
we need to supply only the newly exposed one. Don't assume a one-row bitmap
plots faster than a clipped strip: measure it.

At one row per 60 Hz gameplay update the payload is **15,360 B/s (15 KiB/s)**,
not counting VDU commands, sprites, sound or flow control. At 1.125 Mbaud
8N1 the 256 data bytes alone take about 2.28 ms of the 16.67 ms frame.
The music clock is independently 50 Hz. Neither average UART bandwidth nor
FAE performance establishes worst-case SD or VDP latency on hardware.

## Proposed tranche and gates

- EARTH-001-008 [ ] **Build the stream, preserve the control.** Generate
  a separate AGNB container in flight order from the existing packed artwork:
  strip 00 first, bottom row first within each strip, pixels left to right.
  Stream strips 00–13; keep 14–15 as ordinary resident bitmaps used by both
  the playable approach and the future ending. Keep all four warp starfields
  resident. Remove 00–13 from startup uploads in the candidate only; verify
  progress totals, asset IDs, staging manifests and missing/short-file errors.
  Reconstruct all 4,096 rows and byte-compare to the control, including baked
  structures and the join into 14. No Tiled edits or spawn changes.

- EARTH-001-009 [ ] **Prove one row on stock MOS/VDP.** Test one reusable
  256×1 VDP bitmap with unmistakably different consecutive rows. Verify that
  updating its existing backing bytes changes what gets plotted, and that
  reusing it after a plot doesn't change an earlier row or race a queued
  draw. Use a second row buffer only if ordering requires it and the stock
  path is verified. No per-row heap churn, bespoke firmware or async MOS
  dependency. With one row slot the Earth bitmap payload target is
  **128 KiB + 256 B**, versus 1 MiB today. Measure peak memory separately.

- EARTH-001-010 [ ] **Integrate at the bound Earth renderer only.** Prime
  an eZ80 read-ahead buffer before the first Earth row is due. Open and
  validate the single DATA record once per Earth entry; keep its cursor
  open for sequential reads, without reopening or seeking between rows. Start with
  sequential 1 KiB reads (four rows), refill at a measured low-water mark,
  and measure the longest blocking read, not just average throughput.
  Send one complete row update and plot at the existing exposed scanline;
  allow music service only at complete VDU command boundaries. Advance the
  stream once per actual terrain step, never by wall clock. Preserve
  foreground drawing, active tiles, and the single existing call in the
  normal scroll loop; other levels and fast warp do no stream work.
  At strip 14 switch to the resident approach without a duplicate row or
  upload; after 15 restore the usual background. Pause/frozen states consume
  nothing; respawn continues at the current row. Restart/level wrap rewinds
  and primes; normal exit and errors close the file. Reject an underrun or
  truncated read visibly in the fixture; never plot stale bytes as new rows.

- EARTH-001-011 [ ] **Compare on FAE, then on hardware with Smith.** Verify
  first/last row, all seams, six-wave timing, frozen/resumed scroll, restart,
  and the Earth-to-starfield handoff. Verify the two facility strips are
  available immediately without implementing the cutscene. Compare startup
  time, VDP payload and peak RAM where observable, worst refill and row
  upload times, missed update deadlines and 50 Hz music lateness against
  the preload control. Check during dense waves with music and SFX on;
  report instrument limits and audible/visible results separately. No
  promotion on an emulator pass alone.

- EARTH-001-012 [ ] **Decision.** If the picture matches and hardware keeps
  pace, present results for production approval. If not, retain the preload
  control, identify whether SD, UART, or plotting consumed the margin, and
  bring back the previously proposed three-strip ring plus two pinned strips
  as an alternative. Do not implement both strategies in this tranche.
