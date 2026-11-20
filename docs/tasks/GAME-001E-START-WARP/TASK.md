# GAME-001E-START-WARP — Match the opening starfield to the level transition

Smith wants the first game entrance to open at normal starfield speed, accelerate
to fast scroll, then ease back to normal before terrain enters. It currently
starts at top speed. Use the between-level warp as the model for the speed
sequence, while keeping it separate from the terrain scroller.

## Scope and acceptance

- GAME-001E-START-WARP-001 [x] **Reuse the warp sequence at game start.**
  Begin with a visible normal-speed approach, climb through the same ordered
  speeds to a brief fast hold, then decelerate to one scanline per frame.
  Share the existing level-transition speed ladder and phase logic where
  practical. Do not change how the between-level transition itself behaves.

- GAME-001E-START-WARP-002 [x] **Preserve the entrance and handoff.** Leave
  the banked ship swoop, settled endpoint, and one-second grace behavior
  intact. Do not admit terrain until the opening returns to normal speed.
  Mid-level respawns retain their existing entrance with no starfield warp.
  Leave the one-scanline terrain code and the accepted starfield artwork alone.

- GAME-001E-START-WARP-003 [ ] **Verify the opening in the ordinary game.**
  Check the starting, peak, and final speeds; full progression in both
  directions; no exposed seams or bezel motion; and terrain entry only
  after the final normal-speed phase. Repeat after Game Over → fresh start.
  Confirm the mid-level respawn and existing between-level warp are unchanged.

## Timing decision

Smith approved implementation with the existing 112-frame ship entrance.
Use the existing level-transition controller and speed ladder with a short
opening profile: 12 normal-speed approach frames, eight acceleration
steps at four frames each, 40 fast-hold frames, then seven deceleration
steps at four frames each. Preserve the existing 300-frame level profile.
These contexts do not overlap: only initial game entry selects the short
profile; mid-level respawn does not advance or reset it.

## Build evidence

The renderer consumes one shared controller frame before each opening
ship update. The effective start is 16 frames at one scanline per frame
(12 approach plus the first acceleration step); peak speed is 16 for 44
frames (the final acceleration step plus the hold). The last four frames
are at one scanline again, then the existing grace/terrain handoff runs.
The ship keeps its own unchanged 112-frame motion timer. The ordinary
level transition reinitializes the shared controller with its original
300-frame profile. Mid-level respawn leaves that controller alone.

Full build, asset checks, and the test suite passed (220 passed, one
headless-input skip). The expanded eZ80 schedule fixture assembled; its
frame-by-frame cases have not been run on the guest. Game and UI containers
match the prior build byte for byte. Ordinary binary `1b4941df…94aa`.
No emulator staging, visual acceptance, commit, or push.

## FAE review staging — November 20

Oryx checkout 530 (`aa255daa1bcf`) holds the stopped review profile at
`/home/smith/Agon/mystuff/nurples/.emulator/opening-warp-fae-r01`, with
stock FAE/VDP and pinned Eirene RC3. The
[SD readback](../../../.build/opening-warp-r01/staging-receipt.txt)
verified the ordinary binary, accepted assets, and load-only startup.
All diagnostic overrides are disabled. The Author launches the wrapper,
types `RUN` at MOS, selects a controller, and starts normally. Look for
slow → fast → slow starfield travel during the 112-frame banked swoop,
with terrain entering only on the handoff to normal play. Await the
verdict and emulator shutdown before releasing Oryx.

## FAE visual PASS — November 20

Smith confirmed that the slow → fast → slow scroll looked good with the
ship swoop and terrain did not enter until the scroll returned to normal.
This passes the ordinary opening and terrain handoff; it does not add
visual evidence for restart, mid-level respawn, or the between-level warp.
Smith closed FAE. Checkout 530 was
[released](../../../.build/opening-warp-r01/release.json) after a fresh
whole-host idle check, and TRS-80 request `8e0043ddf6aa` was notified.
