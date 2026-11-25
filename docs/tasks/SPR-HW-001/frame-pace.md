# Aligned Earth frame pacing: bombing vs no bombing

Two sibling diagnostics, same map, assets, aligned formation, JIT renderer and clock probes. Disable the air-bomb update call in the control; leave turret fire alone. Do not modify the existing human-tested packages.

Measure MOS-observed VBlanks between consecutive returns from the existing frame wait. MOS adds two clock units per VBlank IRQ. Count only unpaused game_playing and game_level_transition iterations, sampled by pre-dispatch state. Exclude setup, menus and Game Over, and keep the baseline current across excluded frames. Print skipped update opportunities, total ticks, completed updates and any zero-tick waits at ESC; reset only at MOS RUN. No music dependency, new hardware timer, serial logging or extra VDP synchronization. This measures game pacing, not actual scanned pictures.

**2026-11-25, baked control (historical):** aligned, background off: 4/4629
skipped; background on: 335/4684. Both runs had zero zero-tick waits.
Background off also removed visible city structures in these two measurements.

**Independent planes revision:** verified 943 structure cells and 72 spawn
cells in each linked executable (seven positions contain both). Background
strip pixels and used tile bitmaps match the Tiled inputs. Both fixtures use
identical assets; their binaries differ only at the four-byte bombing call.
Updated and hash-verified both card directories. Hardware retest pending.

Staggered FAE audition only: use the author's 12-ship template at rows
234..229. Repeat at 48-row intervals: the last rank starts at Y=-6 on
tick 80 and clears Y=336 after another 684 ticks; next admission at tick
768. Hold ranks at scroll speed for 80/64/48/32/16/0 ticks. The hull
footprints span X=96..255, allowing a 96-pixel march left and back.
Keep the aligned control's no-bomb pacing configuration and separate
rendering planes. No Olimex deployment until visual acceptance.
