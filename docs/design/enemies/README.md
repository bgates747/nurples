# Enemies

**Current game:** Tiled shows an enemy marker on the Spawns layer; the player sees the terrain painted on Tiles. A functional seeker pad is different: it owns its launches and provides a retreat destination. Small, medium and large ships take one, two and three shots respectively. See the [air enemy design and movement record](../../tasks/AIR-001/TASK.md) and [tile registry and editor mappings](../../tasks/MAP-002/TASK.md).

**Proposed:** [modular bosses and encounter placement](bosses/README.md).

## Art to work into the game

Smith adapted these from [Gustavo Vituri’s Space Shooter pack](https://gvituri.itch.io/space-shooter). The six edited PNGs are tracked in [the pending sprite folder](../../../assets/img/orig/sprites/pending/), outside the production builder’s nonrecursive PNG scan. The original download and its provenance file remain local to Oryx.

- [ ] **Flitter enemy ship** — `flitter_1.png`, `flitter_2.png`. Choose its movement and encounter before assigning a spawn.
- [ ] **Missile** — single: `missile_3x6_1.png`, `missile_3x6_2.png`; double: `missile_3x6_double_1.png`, `missile_3x6_double_2.png`. Shooter and firing behavior are undecided; the artwork need not be paired with the Flitter.

## Sprite spawns versus active tiles

Use **sprite spawn**, not *air spawn*, for the generic map marker. It describes
what gets created, not where it can travel. A fixed turret is an active tile
even though it fires moving projectiles. Earth already permits sprite markers
over independent scenery; [MAP-005](../../../level-design/sprite-spawns.md)
extends that authoring rule to DG.

**Future possibility, not scheduled:** a Xevious-inspired tank could spawn
from the same marker layer as a flying ship, but use a movement program
that only admits road tiles. That suggests a terrain property such as
“road” rather than hard-coded artwork IDs. How a tank chooses a turn at a
junction or reacts to a dead end is an encounter-design question for later.

## Opening stages — chosen movement direction, not yet implemented

- **Stage 1, Earth:** Space Invaders-inspired movement. The threat should read as a formation advancing together. Exact march timing, descent and formation size are open.
- **Stage 2, outbound space:** Galaxian/Galaga-inspired movement. Contrast the Earth formation with attackers that can peel away into individual runs. Dive paths and wave composition are open.

These are references for how movement feels, not specifications to reproduce the arcade games.

### Earth: independent actors first

Start with authored spawn positions, directions, spacing and timer phases. Each alien owns its timer and follows its normal movement and playfield bounds. The goal is a readable march without overlap, not a shared formation controller. Check an entire sweep and reversal: independently bouncing ships can cross even if their initial spacing looks right. Destroying one must not change the clocks of the others. Coordinated group turns remain an alternative, not the starting design.

#### Earth wave membership and sprite slots (proposal, 2026-11-24)

Smith wants a way to designate formation spawns and represent **one active
formation at a time** to start. Its job would be membership and draw-slot
ownership, not steering: each alien keeps its own movement program and timer.
This is separate from the seeker chase-formation idea below.

One Tiled option, not yet selected: keep painting the existing ship markers
and add a map property, `formation_rows`, listing one inclusive row range per
wave. A marker inside a range joins that wave; the same tile outside remains
an ordinary spawn. No duplicate layout, extra tile ID, new layer or change to
the one-byte terrain map. A spawn still cannot share its cell with ground art.
Ranges assume all eligible markers in those rows are members; moving a wave
vertically would require updating its range as well.

The compiler sees all ranks before the game sees the first one. It could emit
a separate row/column lookup assigning the **top rank the lowest software IDs**,
with left-to-right ties, independent of spawn order or earlier kills. Print a
roster (row, column, ship, slot) for review and reject waves over eight members.
The live object could hold a wave reference and an eight-bit live mask. Keep
ownership until the final authored row has entered **and** the mask is empty:
clearing the first rank early must not retire an incoming second rank. Flag a
second wave arriving before release; don't silently steal slots. No format,
allocator or runtime object has been implemented. See the
[draw-order evidence and proposed A/B](../../tasks/SPR-HW-001/TASK.md).

### Outbound space: two dive patterns observed in Galaga

Smith observed two distinct attacks in MAME. Sometimes a leader dives with two escorts, and the trio keeps the same orientation relative to the screen that it had in the main formation. Other times the leader flies its own dive while a lone non-leader flies a separate path, usually mirrored left-to-right. These are observations of the motion, not claims about Galaga's internal code.

For stage 2, those suggest two different jobs: keep a recognizable group silhouette through a swoop, or send independent threats down opposite sides. Neither requires the seekers' proposed seniority or slot-acquisition rules.

**What the disassembly shows.** In [Glenn Neidermeier’s annotated Galaga disassembly](https://github.com/neiderm/arcade), look up `c_1079`, `c_1083` and their common setup `j_108A` in [the main CPU's gg1-2.s](https://raw.githubusercontent.com/neiderm/arcade/master/galag/galagao_ASxxx/rom0/gg1-2.s). The boss and each of its one or two wingmen receive individual flight records, seeded from their own sprite positions in 9.7 fixed point, with a pointer to flight data. The boss/escort path takes a direction bit from the selected slot; an ordinary diver derives one from its formation position. The annotations identify this as reversing the turn angle for a right-side departure.

**Interpretation, not a traced result.** Identical per-tick movement from different starting positions preserves screen-relative spacing even as each ship banks through a curve. That would explain the observed trio without wingmen chasing a leader. Mirroring the turns explains opposite-handed dives; it does not prove the observed singleton and boss were deliberately dispatched as a pair.

**Nurples experiment, not a committed format.** Author an original dive in segments of (duration, forward speed, turn per tick). Choose handedness and start tick at launch. Try the same script from three offset slots on one tick for an escort; opposite turn signs for a pincer; and a delayed second launch for a crossing threat after the first dodge. No ROM tables or Galaga-specific flight interpreter are required.

## Seekers: chase formations (proposal)

Seekers tend to bunch up when they all chase the same target. Let the oldest chasing seeker lead and assign the younger ones slots in spawn-seniority order, not actor-table order. Four ships form a diamond, three a delta, two an echelon; one chases alone. When a ship is lost, the surviving order determines the new leader and slots. This is a chase behavior, not a replacement for launch, orbit or retreat.

Two open choices:

- **Slot coordinates:** fixed offsets in screen axes are simple and readable. Leader-local offsets rotate with its heading, so the team banks around turns like an airshow formation. The latter costs more math and asks more of an outside wingman during a tight turn.
- **Slot acquisition:** snapping to the slot is useful as a diagnostic to check geometry and succession. For flight, decide the maximum speed for joining and holding station. A follower should take time to catch up, may lag during a turn, and must not teleport on promotion or re-slotting.

Spacing near the bezel, how followers recover after a reversal, and what to do with more than four chasing seekers are still open. No implementation or tuning values selected.
