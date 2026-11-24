# SPR-HW-001 R04 — Interactive scanline lab

Build a diagnostic version of Nurples in which Smith can arrange real hardware
sprites with the mouse, then leave the scene untouched to watch scanout. The
key comparison is the **same sprites in different positions**, first with the
game running and then with the game workload off. Approved sequence: build
and audition the standalone editor first. Do not link Nurples, simulate its
workload, instrument firmware or deploy to hardware in this round.

### Frozen first-round controls

Use C for reusable pointer capture, hit testing and scene editing, with a
separate Agon mouse/sprite adapter. Click selects; drag retains the grab
offset. A palette drag creates one sprite only on a valid canvas drop.
Trash removes an existing sprite on release. Escape while captured cancels
without deletion or displacement; fresh Escape when idle exits to MOS.
Align Tops changes only Y and Undo Arrangement restores the exact previous
layout. Show selection position and dimensions. Expose estimated peak
scanline load, not a measured miss counter. No continual sprite submissions
or toolbar repaint while idle. Test on FAE under human-held ordinary FIFO
custody until Smith finishes; no automation timeout for his playtest.

## Operator contract

- SPR-HW-001-R04-01 [ ] Put the controls in the upper panel (screen Y 0–47).
  Drag a sample from a palette onto the screen to spawn it; drag an existing
  sprite to move it; drag it back to a delete target to despawn it. Pick the
  topmost sprite on overlapping hits and retain the grab offset. Offer small,
  medium, large and wide samples with their actual bitmap dimensions shown.
  Use screen pixels, not world coordinates. The entire 512×336 area below the
  toolbar is fair game, including both bezels and the lower instruments. Do not
  change production collision or clipping rules to accomplish this.

- SPR-HW-001-R04-02 [ ] **Later integration, not first-round:** add the live invulnerable Nurples workload;
  preserve the independent 1/2/3 render switches and M. Supply a separate
  **game workload on/off** switch. Off must stop game simulation, ordinary
  drawing/scrolling, gameplay sprites, music IRQ and game audio service,
  leaving the arranged sprites, minimal editor input and display timing.
  Switching back must preserve the test layout and prior switch settings,
  without replaying elapsed ticks. Show active counts separately for lab and
  game sprites. Map the available VDP slots first; never silently evict game
  actors or mistake the eight-enemy pool limit for the compositor limit.
  Report capacity exhaustion visibly.

- SPR-HW-001-R04-03 [ ] Provide two repeatable arrangements: align the same
  sprites on one scanline, then stagger them vertically without changing their
  count, bitmaps or animation frame. Keep manual placement available after
  either preset. Show peak intersecting sprite count and summed bitmap widths
  on the busiest line (an estimate of work, not measured compositor time).
  For a count-vs-width check, compare one wide sprite with several narrow ones
  of the same total width. Keep toolbar text static during sampling; park the
  mouse cursor in the toolbar so it isn't another variable.

## Timing and acceptance

- SPR-HW-001-R04-04 [ ] Design optional timestamp probes at frame start,
  after game update, after terrain submission, after sprite submission, and
  at the VBlank wait boundary. Store bounded records in RAM; dump after the
  capture. Record frame ID, switch mask, arrangement and timer resolution.
  Distinguish submitting VDU bytes from the VDP completing work. Compare
  probes off/on against the same untouched scene; don't insert synchronous
  VDP waits or live serial logging into the baseline. Allow the operator to
  mark an observed jump without treating a human keypress as its timestamp.

- SPR-HW-001-R04-05 [ ] Verify interaction and scene repeatability in FAE.
  On Console8, start with zero lab sprites, then increase the load and bracket
  the first repeatable visible jump. Compare aligned/staggered and game
  on/off at that same count, with equal observation windows. Record both clean
  and affected runs, firmware/mode, sprite dimensions and positions. Report a
  limit **for that arrangement**, not a universal maximum.

An eZ80 trace can show a late game update or a long submission; it cannot
identify the physical scanline that missed its deadline. If the screen jumps
while the host timings stay normal, correlate with video or propose a separate
VDP-side probe around scanline composition and line delivery. Keep stock
firmware as the baseline; instrumented firmware is not part of this fixture.

### Standalone audition R01 — awaiting Smith

Built `diagnostics/scanline-lab` with AgonDev for ADL MOS at 0x40000. Native
editor tests passed for boundary drops, overlap selection, drag offset and
cancellation, deletion, capacity, alignment/spread, undo and idle input.
The four stationary opaque samples are 8×8, 16×16, 32×32 and 128×16.
Guest readback: `/test/scanlab/scanlab.bin`, 13,202 bytes, SHA-256
`eb15924889ee8542d9743b2bdea752453f4c8327a052c3629067eea000af3e87`.
Staged alongside the game on Oryx checkout 603, not on hardware. FAE mouse,
presentation, repeated edits and exit to MOS still require human validation.

Smith confirmed spawning, dragging, selection, Align and Spread on FAE.
R01 only treated Trash as a drop target. R02 also deletes the current selection
on a click released inside Trash; release outside cancels. Both paths use
the same deletion routine. Host regressions passed; R02 verified at the same
guest path, SHA-256 `1d52a5961087ee4a5f645e11300cca79db1d27992235696d7ae5af850947577f`.

Smith approved R02 for Console8 testing. Deployed that exact binary to
AGONC8 at `/test/scanlab/scanlab.bin` and verified its SHA-256 on the card.
Root autoexec and the existing Nurples executable were unchanged.

**Console8 R02 observation:** Smith saw flicker begin at a displayed peak
summed sprite width of about 128 pixels per scanline. With the same 32×32
sprites, aligning them produced flicker; staggering them did not produce
visible flicker. Artifacts occurred on sprite-occupied lines, usually nearer
the sprites' bottoms. This implicates the concentration of work on shared
lines rather than total sprite count alone. Running out of timing margin
over successive dense lines is plausible but unmeasured. The counter sums
bitmap widths, not compositor time; 128 is not a universal hardware limit.
