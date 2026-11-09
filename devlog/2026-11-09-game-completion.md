# 2026-11-09 — Complete-game presentation and flow

Purple Nurples already has a strong transition from splash and asset loading
into the start of play. The new `GAME-001` task treats that sequence as the
quality baseline and collects the remaining gameplay-flow and presentation
work needed for the game to feel complete. This is a running Author-directed
design record; `TODO.md` remains the sole authoritative unfinished-work list.

## 1. Player death through restart

The existing cooperative `player_dying` implementation supplies INITIAL,
DELAY, FINALE and FINISH phases and already spreads effect allocation across
outer frames. The desired presentation is more specific than its current
behavior:

1. When player death begins, background travel stops. The scene should read as
   the world falling away from normal play rather than normal scrolling merely
   continuing underneath a death effect.
2. The player's ship continues moving forward after control is lost. Its path
   becomes erratic rather than stopping in place or following normal input.
3. The ship retains the last left, centre or right sprite frame commanded by
   the player. The death sequence does not snap it to a generic pose.
4. While moving forward, the ship wiggles randomly back and forth around that
   retained pose.
5. The existing fire effects are too directionally uniform: fireballs can
   leave in any compass direction. Variation should remain, but their dominant
   motion must trail behind the damaged ship so the effect reads as fire and
   smoke pouring from it.
6. The moving, burning phase ends in a recognizable crash beat. The crash then
   produces several explosions moving in varied outward directions.
7. The subsequent transition into respawn/restart needs additional polish; its
   exact timing, screen treatment and control handoff will be refined from
   visual review rather than guessed in this first note.
8. The damaged flight path is screen-position-sensitive. At the left or right
   edge, the ship must reverse toward the middle of the playfield. It should
   commit to a broad inward sweep rather than immediately accepting another
   random reversal at the boundary; this produces the intended out-of-control
   zig-zag and makes the trailing fire waggle coherently.

9. Normal play will reserve a top-of-field **death runway**. Player input uses
   a lower logical ceiling, while the physical sprite boundary and viewport
   remain unchanged for other actors and effects. On death, the damaged-flight
   controller may enter the reserved band and spend that guaranteed distance
   on the forward zig-zag before the terminal crash. This is cleaner than
   repairing an arbitrary near-top starting position after death begins.

The preferred top-edge treatment therefore combines the reserved runway with
a soft **crash ceiling** near its far end. Upward velocity eases toward zero
while the lateral zig-zag continues for a short beat, then the terminal crash
occurs with the complete ship and explosion field still visible. A brief
inward/downward recovery remains only a defensive fallback for an invalid or
debug-injected starting position inside the crash ceiling.

The current playing field is 336 pixels high and the ship is 16 pixels high.
The normal-play ceiling must be a player-specific constant rather than a
change to the shared `sprite_top`, because enemies, explosions and other
effects still own the entire viewport. The exact runway depth will be chosen
together with the retuned death duration and forward speed; a provisional
48–64-pixel band is a useful visual starting range, not yet a frozen value.

Two alternatives remain available if visual review rejects that treatment:

1. Trigger the terminal crash immediately upon reaching a top-safe boundary.
   This is simple and readable, but death near the top may feel too abrupt.
2. Reflect downward from the top as well as inward from the sides. This keeps
   the ship alive longer, but risks looking like an ordinary screen-saver
   bounce instead of a vehicle breaking up.

## 2. Architectural boundary

This task complements `ASM-001F`; it does not authorize a separate blocking
death loop or a rewrite of functioning legacy dispatch. Death choreography
must remain actor-owned and bounded per outer frame. Any background freeze is
a published death-phase policy consumed by the relevant world actors, not a
master-loop routine that reaches into and rewrites their local states.

The initial implementation review should distinguish spatial background
travel from local animation. The current requirement freezes travel; whether
individual active scenery animations continue during the sequence remains a
visual-design choice to settle before that behavior is changed.

## 3. Review questions retained for implementation

1. How far and how frequently may the ship wiggle before it stops reading as
   forward motion?
2. What rearward cone and speed variation make the fire trail feel organic
   without returning to all-heading noise?
3. What duration and explosion count give the terminal crash sufficient
   weight without making repeated deaths tedious?
4. What should remain visible between the final explosion and the restored
   player: the frozen field, a cleared field, or an explicit restart cue?
5. How deep should the normal-play death runway be once forward speed and death
   duration are retuned? Begin visual review in the 48–64-pixel range.

## 4. Enemy behavior during player death

Enemies need explicit behavior for both PLAYER_DYING and PLAYER_DEAD rather
than continuing their ordinary attack programs until the level is rebuilt.
Seekers have the first defined retreat behavior:

1. As soon as player death begins, a seeker stops targeting and damaging the
   player and selects a retreat destination for itself.
2. The preferred destination is the nearest usable seeker launch pad. Launch
   pads are active `tile_pad_small` records, so only a live, visible record may
   be selected; a destroyed pad transformed into a crater is not available.
3. On reaching its pad, the seeker docks or otherwise disappears cleanly and
   releases its sprite-table record.
4. If there is no usable launch pad, the seeker chooses whichever horizontal
   screen edge is nearer, flies out through that edge and releases its record.
5. PLAYER_DEAD preserves the retreat already in progress. It does not restart
   targeting, select the player spawn position, or abandon a valid destination
   merely because the lifecycle advanced.
6. Respawn clears every retreat destination and stage before normal seeker
   behavior can resume.

The current `kill_player` hook changes all live seekers to
`move_enemy_seeker_orbit`, which creates a generic orbit around fixed screen
origins. That is the behavior this requirement replaces. Destination choice
belongs to each seeker's mini state machine after it reconciles with the
published player lifecycle; the global game loop must not assign individual
seeker move programs or destinations.

Nearest-pad selection can compare squared 16-bit integer-pixel distance without
a square root. Once selected, the pad record may later disappear or scroll
away, so the seeker must revalidate it each update and fall back to the nearer
horizontal edge if it ceases to be usable. The edge fallback should be latched
to prevent indecisive left/right switching near screen centre.

Fireballs, firing turrets and any later enemy classes still need their own
PLAYER_DYING and PLAYER_DEAD dispositions. They must not inherit the seeker
retreat behavior by accident.

## 5. Enemy movement variety

The existing seeker is fun to dodge but is deliberately difficult to shoot.
It should remain the dangerous evasive pressure enemy rather than becoming the
template for the entire enemy roster. A complete game needs additional enemy
classes with distinct, more predictable movement and generous firing windows.

The existing `move_enemy_seeker_orbit` routine is retained. Returning seekers
to launch pads replaces only its current use as the blanket player-death
behavior; the orbit itself remains available for holding patterns, transitions
and future enemy designs.

Initial movement families worth prototyping are:

1. **Formation sweep:** a group crosses the field on a fixed diagonal or broad
   horizontal lane, optionally making one documented turn. Easy to read and
   lead with the laser.
2. **Dive and withdraw:** an enemy telegraphs a straight or shallow dive,
   passes the player and exits rather than turning into permanent pursuit.
3. **Orbit then break:** enemies visibly hold around a point using the retained
   orbit routine, then leave on a predictable tangent or attack line.
4. **Slow heavy target:** a durable enemy follows a simple path and rewards
   sustained fire, providing contrast with the seeker's agility.

These are prototype directions, not four promised enemy classes. Selection
should be based on which patterns are most enjoyable to shoot in a short mixed
encounter. Every adopted type needs explicit PLAYING, PLAYER_DYING,
PLAYER_DEAD, level-transition and respawn behavior owned by its local
controller.

## 6. Xevious design study

The project contains a substantial dormant Xevious experiment rather than a
few isolated pictures:

1. `tiles/xevious/` retains four 16-by-128 tile maps, their file metadata, a
   combined tile sheet, a sprite sheet and fifteen extracted sprite frames.
2. `src/asm/images_tiles_xevious.inc` describes 799 generated 16-by-16 tile
   images across four buffer ranges.
3. `src/asm/levels_xevious.inc` contains four generated level maps.
4. `build/scripts/xevious.py` and `build/scripts/xevious_sprites.py` are dormant
   image-slicing experiments; their hard-coded paths no longer match the
   retained tree and they are not part of the current build.
5. `assets/img/design/xevious_explode.xcf` and its classic-tiles counterpart
   preserve explosion-art experiments.

A useful public technical reference exists at
<https://github.com/jotd666/xevious>. At inspected commit
`28d10c52b72109834cc244051cf0cef3b7e3c37c`, it contains the original GAME and
SUB CPU disassemblies plus a line-by-line 68K transcode produced through
reverse engineering. Its README describes the gameplay core and pseudo-random
behavior as matching the original arcade game. The repository carries no
obvious license file, so it is a behavioral research source, not code to copy
into Purple Nurples.

The initial source survey already reveals several useful movement structures:

1. **Toroid:** initially vectors toward the player, then proximity triggers a
   committed left or right swing while its turning animation advances.
2. **Torkan:** approaches on a calculated vector, fires after a randomized but
   bounded timer, then recomputes direction and withdraws. This is an excellent
   candidate for an easier-to-shoot telegraphed attacker.
3. **Jara:** approaches, tests a proximity band, then commits to a lateral
   spinning pass. The non-shooting form separates movement challenge from
   projectile pressure.
4. **Kapi and Terrazi:** begin with an intercept vector, then change velocity
   incrementally through an attack/escape curve while firing on timers. Their
   acceleration stages are more interesting than continuous homing but remain
   learnable.
5. **Sheonite pair:** uses explicit approach, docking/combining and retreat
   phases, demonstrating how a short enemy encounter can tell a miniature
   story without persistent pursuit.
6. **Enemy selection:** flying-enemy type tables and an `enemy_AI_level` vary
   the roster as play develops instead of increasing difficulty only by making
   every actor faster or more accurate.

The most promising first homage is therefore not a literal Xevious enemy. It
is a Nurples-native **approach, telegraph, fire, withdraw** actor inspired by
the Torkan structure, followed by a proximity-triggered lateral spinner. Both
would create fair firing windows and strong contrast with seekers.

MAME's maintained Xevious driver is a complementary hardware reference for the
three-Z80/shared-memory architecture, but it intentionally emulates the machine
rather than explaining the original gameplay routines. The reverse-engineered
transcode is the better movement-pattern source. No commercial ROM download is
needed for this design study, and retained original-looking art should not be
assumed distributable in a future release merely because it exists locally.

## 7. Concealed Xevious port

The Author's longer-standing intent is to smuggle a playable Xevious port into
Purple Nurples, not merely borrow a few enemy ideas. Its final presentation is
still open:

1. A **campaign level** makes the homage discoverable and lets it participate
   in ordinary progression, but requires the Xevious rules to coexist cleanly
   with normal Nurples balance and state transitions.
2. A hidden **Easter-egg mode** may own a different player craft, weapons,
   enemy director, scoring and completion condition. It is easier to isolate
   technically and better suits the joke, but needs a memorable discoverable
   entry condition rather than an undocumented developer shortcut.

The Easter-egg form is the preferred first architecture because it establishes
a clean mode boundary. If it proves cohesive, the same content can later be
promoted into the campaign without first entangling experimental rules with
normal play.

A credible compact port needs more than the retained maps:

1. A dedicated mode descriptor and complete entry/exit reset contract.
2. The appropriate player craft and a decision on whether to reproduce the
   original air/ground dual-weapon distinction or adapt it to Nurples controls.
3. Map-position-driven enemy encounters, not only random timed spawns.
4. A small representative roster demonstrating approach/withdraw, lateral
   evasion, ground threats and at least one staged encounter.
5. Independent score, life, death, restart and completion behavior that cannot
   leak state into the normal campaign.
6. A provenance decision for every retained art and code asset before any
   public release. Behavioral study and clean-room reimplementation are
   distinct from copying the original program.

The first implementation milestone should be a short vertical slice using one
retained map, one player weapon, one ground interaction and two contrasting air
enemy patterns. That is enough to decide whether the mode is genuinely fun
before attempting exhaustive arcade fidelity.

## 8. Exact Xevious machine emulation on eZ80

A more audacious descendant leaves Nurples entirely: emulate the original
Xevious machine on the Agon's single eZ80, including all three guest Z80 CPUs.
This belongs to the separate **Arcade** project, not Purple Nurples. It is
distinct from `GAME-001D`'s native Nurples-engine port and is recorded here
only to preserve the idea and ownership handoff. The attraction is precisely
the absurdity of running three classic Z80 contexts on one descendant
processor.

MAME's maintained driver documents Xevious as logically sharing the Namco
three-Z80/shared-memory architecture used by Galaga, despite its physically
different board and video hardware:
<https://github.com/mamedev/mame/blob/master/src/mame/namco/galaga.cpp>.
That source defines an 18.432 MHz master clock and divides it by six for the
3.072 MHz Z80 configuration. Three guest clock streams therefore represent an
aggregate 9.216 million Z80 T-states per second. On an 18.432 MHz eZ80, the
naive budget is only about two host clocks per aggregate guest T-state before
instruction decode, shared-memory arbitration, interrupts, custom devices,
video or sound. A conventional cycle-accurate interpreter is therefore not a
credible real-time first assumption.

The shared architecture makes this more valuable than a one-game stunt. A
well-factored Arcade implementation could separate reusable three-CPU
scheduling, shared-memory arbitration, Namco bus/custom-device interfaces and
common sound support from each game's memory map and video board. MAME groups
Bosconian, Galaga, Xevious and Dig Dug around this logical family while noting
their board differences. Xevious can therefore be the demanding first target
that opens a path to several other Namco games, provided Arcade models the
common platform rather than hard-coding a single ROM set.

The Author identified a broader toolchain requirement behind that work. Arcade
should own a native eZ80 assembly toolkit, built with `ez80asm`, for Z80-family
ports, emulators and translated programs. New original Agon titles remain free
to use C or C++ through AgonDev, but software whose essential structure is Z80
registers, flags, interrupts, address spaces or instruction timing naturally
belongs close to the machine. The toolkit boundary should collect reusable CPU
context layouts and swaps, deterministic scheduling, interrupt arbitration,
mapped-memory/I/O dispatch, timing counters, trace hooks, VDPlet protocol
bindings and headless-test seams. Game and board modules then supply memory
maps, devices and policy without duplicating that foundation. This is an
Arcade/common-tooling project requirement, not implementation scope for
Nurples.

Useful implementation tiers are:

1. **Exact but initially slow:** three complete Z80 contexts, instruction- and
   interrupt-accurate scheduling, shared address-space handlers and deterministic
   frame stepping. Correctness comes before real-time speed.
2. **Block-translated guest execution:** translate safe Z80 basic blocks into
   eZ80-native code while retaining exits for mapped memory, I/O, interrupts,
   contention and self-modification. The common ancestry makes this unusually
   tempting, but direct execution cannot simply ignore address virtualization.
3. **Hybrid machine:** preserve the guest CPU/memory/device-visible behavior
   while the Agon VDP renders tiles and sprites from emulated video state and
   synthesizes the output surface. This can retain CPU determinism without
   reproducing every original pixel circuit in eZ80 software.
4. **Native transcode:** use the reverse-engineered program structure to run
   equivalent native routines. This is likely fastest but is a port, not exact
   emulation, and must not be described as the same accomplishment.

An exact-emulation project would also need the Namco bus/control and I/O/sound
devices, interrupt cadence, shared-RAM visibility, memory maps, watchdog, video
timing and PROM-driven graphics behavior. The CPU cores are only the funniest
part of the machine, not the whole machine.

The sensible Arcade proof is headless and tiny: run the three CPU ROM contexts
through initialization with deterministic interleaving, verify known
shared-memory and interrupt milestones, and measure the achieved guest-T-state
rate. That result will decide whether real-time pursuit needs block translation,
selective native replacement, or acceptance that the first exact build runs
slowly. No ROM image or emulator implementation belongs in the Nurples
repository; a future Arcade emulator can require user-supplied, hash-identified
ROMs in the usual manner.

The preferred full-system boundary is therefore not to reproduce every board
function on the eZ80. First determine the core work performed by each guest Z80
and preserve the CPU-visible contract: execution contexts, interrupts,
shared-memory communication, mapped registers and the timing relationships that
affect gameplay. A dedicated VDPlet can then consume that emulated machine
state and perform the expensive presentation work on the ESP32: tile and sprite
composition, palette/PROM interpretation, display timing, sound synthesis and
other device behavior that does not need to occupy eZ80 cycles. This keeps the
interesting three-Z80 program behavior intact while exploiting the Agon's
actual coprocessor architecture instead of making the eZ80 impersonate an
entire arcade cabinet alone.

That boundary needs a deliberately narrow, versioned protocol rather than
game-specific VDU command chatter. Candidate messages include frame/state
epochs, dirty tile or sprite records, palette changes, sound-device register
writes and synchronization acknowledgements. The VDPlet may optimize how those
effects are rendered or synthesized, but the Arcade host remains authoritative
for emulated CPU state and guest-visible ordering. If designed around Namco
device abstractions instead of Xevious screen objects, the same division can
support the wider related-game family.

The overclock discussion was bench banter, not a serious Arcade proposal. The
Author does not intend to replace the eZ80 crystal or accept the resulting MOS,
UART, timer, memory and qualification work. Do not create an overclock task,
design dependency or Arcade handoff item from that exchange. Arcade targets the
ordinary supported Agon clock.

## 9. Player entrance and mid-level respawn

The current `player_init` performs too many lifecycle operations as one
instantaneous action. It resets shields, laser and fuel state; allocates and
positions the single player record at x=0 and `sprite_bottom`; loads the sprite
frames; moves and shows the sprite; changes `player_state` directly to
`player_alive`; and enables its collision bit. Both initial level construction
and `player_dead` call this same routine. That explains the ship popping into
the lower-left corner with live play beginning immediately.

Replace that compound action with bounded seams:

1. Reset the new life's player-owned resources and timers.
2. Prepare or reuse the player sprite record while it is noncollidable.
3. Enter an actor-owned `player_spawning` controller.
4. Advance a visible entrance path toward the preferred play position.
5. Settle at the destination, establish any respawn grace policy, then enable
   collision and transition to `player_alive`.

The preferred destination is horizontally centred and approximately one third
of the playfield height up from the bottom. Level entry and mid-level respawn
should use the same visible animation rather than two special cases. The exact
path is now chosen: a short rising swoop with bank frames, eased acceleration
and a visibly smooth settle into the destination rather than a straight
constant-speed translation. Its control points remain relative to the resolved
endpoint, allowing the same controller to serve both contexts without a final
snap. Exact offsets and duration are visual-tuning parameters, not separate
level-start and respawn behaviors.

Only endpoint resolution differs. Initial level entry may accept the canonical
point because the Author confirms that it need not negotiate ground
collidables. Mid-level respawn preserves the live level's map/scroll position
and other earned progress, then resolves the canonical point before beginning
the animation. A candidate is safe only when the player's final hitbox does not
overlap an active ground collidable and a short forward corridor is also clear;
the latter prevents placing the ship immediately behind an obstacle that will
become an unavoidable collision as scrolling resumes. Search centre-out for
the nearest acceptable endpoint, with small vertical alternatives around the
preferred lower-third row if necessary.

The player remains intangible throughout the entrance. Grace is a distinct
actor state, not a timer flag buried inside ordinary play:

```text
player_spawning -> player_grace -> player_alive
```

Once the entrance settles, the global coordinator may return to `PLAYING` and
ordinary input and world motion may resume immediately, while the player actor
enters `player_grace`. That state owns the visible ship flashing, its bounded
timer and suppression of player damage/collision. Timer expiry explicitly
transitions the actor to `player_alive`; only that state restores ordinary
collision. The chosen grace duration is one real-time second, measured from the
game's frame/timestamp cadence rather than a CPU-speed delay loop. Grace is the
intentionally simple answer to surviving airborne actors, projectiles and
marginal moving clearances, avoiding special threat-clearing or a complex
respawn planner.

This is a natural first real consumer of the scaffolded global
`STATE_NEW_RESPAWN` phase, but the global coordinator should only publish the
respawn policy and completion boundary. The player controller owns its local
`player_spawning` path and reconciles itself with that global phase, consistent
with the project's actor-state contract.

The score decision is also settled. `player_dead` currently restores
`player_score` from `player_starting_score` before consuming a ship. This was a
deliberate legacy rule, not a bug. The Author has revised that earlier design
decision: death now retains the score earned during the life because respawn
also retains the current level position. The obsolete rollback should be
removed as part of this reviewed slice. Score rollback would only be coherent
if death rolled the entire game state back to the beginning of the level, which
this design explicitly does not do.

## 10. Assembler decision

The Author explicitly approved retaining `ez80asm` as Purple Nurples' assembler
rather than migrating the game to AgonDev. This is a deliberate toolchain
decision: new state-machine and gameplay work should remain compatible with the
existing eZ80 ADL source dialect, include structure, listings and symbols. A
toolchain migration is not part of `GAME-001` or the state-machine repair.

## 11. Victory and hyperwarp level transition

The current implementation confirms the Author's recollection. On the frame
that `tiles_plot` consumes the final map row, it latches
`tiles_level_complete`, posts `STATE_NEW_EVENT_LEVEL_COMPLETE`, assigns the
legacy `game_level_transition` handler and returns before ordinary actor work.
That handler performs only `next_level` and immediately restores
`game_playing`. Because the current tileset has one level, `next_level` wraps
through `next_tileset` to level zero and `init_level` resets the scrolling
state. There is no visual transition and no deliberate blank-starfield entry.

The chosen replacement sequence is:

1. **Terrain coast:** latch the end of source rows, stop admitting new terrain
   and continue normal one-pixel world scrolling until the remaining terrain
   clears. Blank starfield naturally enters at the top. Existing active scenery
   may finish its ordinary animation and leave with the terrain, but cannot
   damage the player during the victory sequence.
2. **Victory loop:** while the field becomes clear, the player actor follows an
   automated banked loop or broad celebratory curve. Ordinary steering is
   suspended and no new hostile actors spawn.
3. **Centre and settle:** the loop resolves smoothly into the exact screen
   centre, using the ship's bank frames to make the correction legible rather
   than snapping its coordinates.
4. **Hyperwarp acceleration:** with the playfield now only starfield, increase
   background displacement per frame through an eased ramp.
5. **Fast hold:** sustain the high starfield rate briefly so the transition
   reads as travel rather than a momentary renderer glitch.
6. **Hyperwarp deceleration:** ease back to the ordinary one-pixel rate.
7. **Next-level admission:** only after normal speed is restored may
   `next_level` initialize and admit the first terrain row. The exact player
   path from screen centre back to its ordinary lower-third play position
   remains a visual choice for the transition design.

This is global choreography and belongs to the scaffolded
`STATE_NEW_LEVEL_TRANSITION` phase. The coordinator owns the ordered phase and
completion conditions, but it publishes policy rather than writing actor-local
handlers. The player reconciles that phase into its own victory-loop/centring
behavior; enemy and tile actors independently reconcile into retreat,
non-hostile completion or despawn behavior. All updates remain bounded to one
outer scheduler frame.

### Separate starfield-only warp renderer

The Author is rightly protective of the optimized terrain path.
`tiles_scroll_background` scrolls exactly one scanline, narrows the graphics
viewport to that one newly exposed line, draws the background behind it, and
feeds the carefully synchronized 16x16 terrain-row machinery. Do not generalize
that code to arbitrary multi-scanline steps. Terrain and its active records
always coast off at the existing one-pixel rate; fast-scrolling terrain is
explicitly outside this design.

The four currently built starfield images are confirmed 256x256 full-field
background blocks, not members of the 16x16 terrain tileset. Hyperwarp therefore
uses its own renderer and state, entered only after every terrain row and active
terrain record has cleared. For a starfield displacement `N`, the dedicated
routine scrolls the playing-field viewport by `N`, changes the drawing viewport
to y=0 through y=`N-1`, and plots the appropriate portion of the selected
256x256 starfield block into that clipped strip. It then restores the full
playing-field viewport before actor rendering. The fixed bezel and cockpit are
outside this viewport and never participate in the scroll.

The starfield-only source position advances by the same `N`, including correct
handling when a frame crosses a 256-pixel image boundary. Entry into warp must
seed this renderer from the ordinary scroller's current background image and
offset so the handoff has no visual seam; exit hands back at the ordinary
one-pixel rate before the next terrain row is admitted. The separation lets the
warp path optimize multi-scanline exposed strips without risking the terrain
loop the Author worked to optimize.

Candidate ramp values and phase durations are visual-tuning parameters. Begin
with powers-of-two-like steps because they make exposed-strip validation easy,
but use time/easing tables rather than changing speed abruptly. Deterministic
tests should verify the background source offset and exposed viewport height at
every step, especially around image-boundary crossings.

## 12. Original Z80 main-level port

The Author confirmed that the recovered pre-Agon Nurples project should supply
one novel production level for the new transition: only the main level
definition used by the most recent Z80 game and its original artwork are in
scope. The old executable assembly is not. The smaller twelve-row construct
test remains historical and is not part of the modern campaign.

The recovered `tiles_level_1` definition contains exactly 255 sequential rows.
Each row uses an eight-bit pair-occupancy mask followed by packed tile nibbles,
producing sixteen modern cells. Its complete legacy vocabulary and modern
mapping are:

| Old ID | Meaning | Modern ID and policy |
|---:|---|---|
| 0 | Empty | 0, empty |
| 1 | Passive blank panel | 17, imported old art |
| 2 | Passive circle | 18, imported old art |
| 4 | Passive conduit cross | 19, imported old art |
| 5 | Passive horizontal conduit | 20, imported old art |
| 7 | Landing/launch pad | 1, modern animated `tile_pad_small` |
| 8 | Laser cannon/turret | 5, modern animated `tile_turret_fireball` |
| 11 | Passive vertical conduit | 21, imported old art |

IDs 17 through 21 are deliberately the first slots beyond the modern active
tile callback domain. The imported decoration therefore remains passive, while
the two mapped active IDs automatically retain current launcher/firing
animation, collision, destruction, and in-place crater transformation. No old
pad or turret bitmap enters the runtime catalog.

The strict importer converts only `tiles_level_1` into Tiled's top-to-bottom
CSV order; the existing map compiler reverses that back to the original
runtime row order. The frozen modern-cell stream contains 4,080 cells and has
SHA-256
`0ba9dfa16c0489a0ac998c42962bb99322bd0b65253d9498d245f66b245197d2`.
It includes 20 modern pads and 25 modern turrets. Tests enforce the row count,
tile vocabulary, active-tile counts, digest, and deterministic generated
include.

`maps/campaign.toml` now keeps the existing modern Agon map first and places
the original Z80 main level second. This order supplies actual novel content
for the planned victory/hyperwarp transition without sacrificing the current
map. The five unchanged source PNGs are retained with original hashes under
`tiles/dg/legacy_z80`; their runtime PNGs, RGBA2222 payloads and atlas cells are
reproduced through Agon64 and canonical `agonutils` as part of the ordinary
asset build.

The checkout had been moved between architectures and its venv still contained
x86-64 Pillow and other native wheels while running on AArch64. The pinned
requirements and editable canonical `agonutils` extension were rebuilt for
AArch64 before generating the imported assets; `pip check` then reported no
broken requirements. This is an environment repair only, not a source change.

Implementation is not visually accepted yet. The remaining gate is a normal
build plus Author play-through confirming the old layout and artwork and the
modern pad, turret and crater behavior. That review also supplies the first
real second-level target for `GAME-001F` once its transition choreography is
implemented.

The deterministic asset check and all 21 map/import/container tests passed.
The Pi checkout has no `ez80asm`, so the documented architecture/toolchain
fallback was used for one finite Oryx compile under FIFO generation 399; no
emulator was launched. Oryx assembled the production source successfully with
assembler SHA-256
`0a323563278dc80b3a9a46d9cc8edb5a6d2632f1860e475410e7324d7c88ee4a`.
The published binary is 40,529 bytes with SHA-256
`702a73e36d97abdb9305e57fe1ebad127261085ef1e74a55d781e10070ac5814`;
its binary-matched symbols have SHA-256
`4b65edcb6c1eb6d0852fac91e105c998f783a7aa88a68d5969b1d6cf042e6078`.
The isolated staging directory was removed and the fresh empty-host proof was
accepted on release.

The complete local discovery run reached 92 tests: 83 passed, one was skipped,
and its remaining results were pre-existing workspace/tooling conditions rather
than this import. Eight assembly-backed cases could not start because the Pi
lacks `/home/smith/.local/bin/ez80asm`; the production assembly was instead
completed successfully on Oryx as recorded above. The one assertion failure is
the older target-directory whitelist not yet accounting for the unrelated,
in-progress `nurples-uart001.bin` and matching symbols already present in this
dirty checkout. Those UART files and their owner’s source changes were
preserved untouched.

## 13. Recovered-level human review and Console8 deployment

The Author played the recovered Z80 level in a dedicated stopped Alexe profile
on Oryx. The fixture used the current 40,529-byte production binary with only
the two level pointers exchanged, a four-byte delta that made the recovered
level index zero without changing game code or campaign data. Its SHA-256 was
`031654ed67e3fbdf14ab80570ade60bbfaf3e182456ca144049ce8c7abf05bef`.
The exact pinned runtime tuple was VDPlet `0.1.2-alpha-async-r1`
`dc43ecb8b1c324ffd1a032e6cc0c56d8c05dd33a355ce170c23adb35ffb533ab`
and Eirene MOS RC3
`d7a51147f3381cd68d56f3fbe4605434a897dfae1b049269afeba09ec2b7eb50`;
Oryx CPU boost was zero.

The level loaded and played, and the existing immediate level transition
successfully entered the following modern level. The Author enjoyed seeing the
old layout again and judged the modern terrain art visibly stronger than the
recovered passive artwork. This accepts the usefulness and basic runtime shape
of the port, but `GAME-001G` remains open until pad, turret and crater behavior
has been explicitly confirmed or the Author waives that final acceptance line.

Alexe animation was subjectively only one-half to one-quarter normal speed.
That observation is recorded as an emulator pacing report, not a Nurples cause
assignment. The review autoexec used `CD /apps/nurples` followed by
`RUN nurples.bin` and was not accepted cleanly. The Author selected the durable
contract for future Alexe profiles: change to the application directory, load
`nurples.bin`, then stop at MOS so the operator types `RUN`. Hardware autoexec
files are a separate boundary and remain untouched unless explicitly requested.
The exact runtime inputs, speed observation, autoexec result and request for a
canonical MOS-compatible `nano`-style editor were forwarded to the Flexe/Alexe
owner in durable mail
`2026-11-09_02-51-27_nurples_game001g-alexe-review-one`.

The same recovered-level-first fixture was then deployed atomically to the
mounted Console8 card at `/run/media/smith/AGONC8/mystuff/nurples` through the
canonical manifest-driven deployment engine. The FAT volume label was
`AGONC8`. The ownership receipt records the review binary and unchanged
`game.agnb`, `ui.agnb` and `Lat38-VGA8_8x8.font` hashes. An initial
misunderstanding also installed a recoverable Nurples load-only autoexec block
and temporarily commented the active AppleDisk pair. The Author clarified that
hardware `autoexec.txt` files must not be changed without a separate explicit
request. The canonical recovery operation restored the AppleDisk lines, removed
the Nurples block and deleted its exclusive-history record. A byte comparison
and SHA-256 check prove the final hardware autoexec is exactly the pre-deployment
file: `39a4b45c0f51c3e8c66b7c7ecb669962b247acc910c3fd61ca27af75dd0a6513`.

The Nurples application bundle remains deployed and owned at its requested
Console8 destination. The deployment engine synchronized the physical
filesystem, and the card remains mounted as required. Going forward, software
deployment to a hardware card preserves its existing autoexec unless the Author
explicitly and separately asks to modify that hardware startup file.

The Author then ran the deployed recovered-level-first payload on Console8 and
observed normal game speed. This is the same application binary and asset bundle
that appeared approximately one-half to one-quarter speed in Alexe, so the
qualitative pacing defect is now isolated to the Alexe/emulation path rather
than ordinary Nurples execution on hardware. The Console8's actually flashed
firmware identity was not captured during this play-through, so this comparison
does not claim an otherwise exact firmware-matched performance experiment.

The hardware play-through intentionally stopped before the level transition;
repeating that transition was not germane to the pacing check. Its functional
behavior was already observed in Alexe, and there is no current evidence of a
hardware-specific transition defect. This does not reopen or add an acceptance
gate. `GAME-001G` remains open only for its previously stated explicit
pad/turret/crater confirmation or an Author waiver of that line.

## 14. Turret slew and fireball directional-rendering defect

The Author reported that the projectiles fired by the historically misnamed
laser cannons do not rotate to match their direction of travel and may not be
animating correctly. Code inspection found a concrete split between motion and
presentation rather than an uncertain visual-only symptom.

`targeting_computer` returns the solved projectile heading in `Heading_p` and
decomposes that heading into `Vp_x` and `Vp_y`. `enemy_spawn_fireball` copies
those two components into the new projectile's velocity, but sets
`sprite_heading` from the direct target bearing `Bearing_t` plus the bitmap's
128-degree artwork correction. Whenever target motion produces nonzero lead,
the sprite therefore points along one angle while travelling along another.
The correction itself remains useful; its input must instead be the actual
launch heading used for the velocity vector.

There is a second, independent spawn-frame defect. A fireball record is reset
to frame zero and receives a heading and velocity, but spawn does not call the
heading/animation frame selector. The movement handler waits for its eight-tick
animation timer to expire before it toggles the animation bank and computes a
directional frame. Thus a newly spawned or reused projectile can display stale
frame zero for its first interval. After that delay,
`sprite_frame_heading_animation` correctly selects one of 32 orientations from
each of two banks, but it is still using the incorrectly stored direct-bearing
heading. `GAME-001H` fixes both boundaries and tests initial selection and bank
alternation separately.

The Author recalls visibly wrong cases in which target lead did not appear to
be a plausible explanation, as well as an apparent lack of animation. The
unconditional eight-tick frame-zero interval is therefore the better candidate
for those particular observations: it affects every shot, including one at a
stationary target, whereas the direct-bearing/lead mismatch becomes visible
only when the solved lead differs from the direct bearing. Both are real code
defects, but neither should be used to explain all observed symptoms until the
initial-frame and subsequent bank-alternation tests have been run independently.

The turret itself currently has no physical tracking model. Every update runs
the lead solver and writes its returned ideal heading directly into
`tile_heading`, so a stationary target becoming mobile can make the barrel snap
from direct aim to full lead in one frame. That looks unnatural and removes a
useful player tactic: a close lateral pass should be able to exceed the
turret's tracking rate when the player cannot safely engage it.

`GAME-001I` therefore gives each turret two distinct quantities: the desired
lead heading and the actual barrel heading. The targeting computer continues
to solve the ideal heading, while the active-tile controller advances the
barrel toward it by a bounded amount along the shortest circular arc. The
initial candidate rate is two angle units per 60 Hz frame, approximately
168.75 degrees per second; the internal heading remains smooth even though the
32-orientation artwork changes only when it crosses a frame boundary. This is
a tuning value, not a frozen balance constant.

The limitation must affect gameplay rather than merely the drawing. At firing
time, the actual barrel heading is decomposed with the fireball speed to obtain
the projectile velocity; the visual fireball heading is derived from that same
launch vector and then receives the artwork correction. For the first version,
reload cadence remains independent of aim error: when its timer expires the
turret fires where the barrel currently points. A player who outruns the slew
therefore induces visible misses instead of disabling the turret by keeping it
perpetually outside an arbitrary lock tolerance. That policy can be tuned after
playtesting without changing the desired-versus-physical heading architecture.

The existing internal names are retained. Renaming the legacy laser/turret
symbols while state stabilization is underway would add noise without fixing a
demonstrated unsafe branch; terminology consistency remains work for the
separately reviewed post-stabilization pass.

## 15. Death-effect correction and explosion edge bounce

The Author corrected the earlier death-effect brief after another play-through:
the existing explosion sprites already form a satisfactory trail behind the
dying ship, and the terminal explosion is satisfactory. Those are accepted
behavior to preserve, not effects to redesign under `GAME-001A`. The remaining
ship-path, pose, screen-boundary and respawn choreography still applies.

The reported explosion-particle bounce is real, but the current code makes it
axis-dependent. `move_sprite` owns the generic boundary policy: it clamps a
sprite to the playfield edge, reverses the applicable velocity component and
writes collision bits to `sprite_screen_edge`. `move_explosion` intends to
override that policy by deactivating an explosion whenever the generic mover
reports an edge. It currently tests register `A` immediately after the call
instead of loading the documented `sprite_screen_edge` result.

For a left or right collision, `move_sprite` reaches `@collide_x` and explicitly
reloads `A` from `sprite_screen_edge` before returning, so the explosion is
deactivated. For a top- or bottom-only collision, it records the Y collision,
reverses `sprite_yvel` through `neg_hlu`, and then returns through the no-X-edge
path. `neg_hlu` uses `xor a`, leaving `A` zero. `move_explosion` therefore
mistakes that vertical collision for no collision and retains the already
reversed velocity, producing the visible bounce the Author remembered.

The narrow repair is to follow the established fireball and player-laser
pattern: after `call move_sprite`, explicitly load `A` from
`sprite_screen_edge` before deciding whether to deactivate. Tests must exercise
all four individual edges and both corner combinations. Explosions deactivate
at every boundary; other sprite classes keep their existing, actor-specific
edge policies.

## 16. Hardware-sprite game-over presentation

The game needs a deliberate game-over sequence after the accepted player-death
finale. Hardware sprites make it practical to place a colourful animated title
over a world that remains visibly alive without turning the overlay into part
of the terrain renderer. The selected composition is a large, stylized
`GAME OVER` title with alternating colour treatments and a smaller
regular-font `Press any key to continue` notice that flashes clearly beneath or
within it.

The current `game_over` handler cannot support that presentation. It prints two
static text strings, calls a blocking delay and then blocks in `mos_getkey`.
While either wait owns control, terrain, active scenery, sprite animation,
input arbitration and VBlank pacing are outside the ordinary scheduler. The
replacement belongs to the scaffolded global `STATE_NEW_GAME_OVER` phase and
must use bounded ENTER, UPDATE and EXIT responsibilities.

ENTER removes the player and hostile projectiles, disables enemy damage and
score changes, establishes a released-key input gate, creates the required
hardware-sprite resources and initializes both animation clocks. It does not
clear the playfield or the seekers. UPDATE advances the ordinary
terrain/background path by exactly one scanline, continues harmless
active-scenery animation, advances the title's colour frame and the prompt's
slower visible/hidden cadence, polls input and returns after one VBlank. The
title remains screen-fixed above the playing field and must not disturb the
cockpit or bezel viewports. No general fast scrolling or transition renderer
is involved.

The Author specifically selected the retained seeker orbit behavior for this
background choreography. Launch pads continue to spawn seekers during
GAME_OVER, but every such actor is non-hostile from birth. Existing and newly
spawned seekers perform a bounded few complete orbits and then reconcile into
their PLAYER_DEAD retreat behavior. A seeker returns to the particular launch
pad that created it if that pad is still active, visible and still the same
logical tile. If its pad has scrolled off, been destroyed or its active-tile
slot has been recycled, the seeker exits through the nearer horizontal screen
edge instead. It must not opportunistically dock at an unrelated pad.

That requires a durable pad association established at seeker spawn. A raw
active-tile pointer is insufficient because table slots are reused; the
association must be validated with stable slot identity plus a generation or
equivalent token before docking. Those values, the orbit count and the local
retreat phase belong in explicitly named custom fields in the existing seeker
record, not in a parallel association table. Orbit completion and transition
to retreat belong to each seeker actor, not to a global loop that rewrites all
enemy states. Completed docking or edge exit releases the sprite record, so
pads can continue the launch/orbit/retreat cycle for as long as the prompt is
left on screen without exhausting the fixed pool.

Laser turrets also remain visibly active, but not hostile. This is not a
GAME_OVER-only effect: the same search behavior is selected whenever the player
is DYING, DEAD, RESPAWNING or in GAME_OVER. Each active turret owns a direction
bit and a countdown. It latches clockwise or counter-clockwise motion for a
bounded random duration; when the countdown expires it flips the direction,
chooses a new random duration and continues. This deliberately mirrors the
simple `en_nav_seek_zag` idiom: stable motion for long enough to read visually,
then a cheap unpredictable reversal, with no centralized choreography.

The search controller never calls the player targeting solver and never fires.
Its direction request feeds the same physical-barrel slew limit specified by
`GAME-001I`, and the rendered frame is computed from the resulting actual
heading. A reversal is immediate as an intent change but cannot snap the
barrel. When the player becomes targetable again, the turret leaves search,
obtains a new lead solution and slews toward it from its current physical
heading rather than snapping.

The Author corrected an implementation shortcut proposed during discussion:
do not overload `move_step`, `move_timer` or another generically named field
merely because the new behavior does not yet have a name in the record layout.
Both sprite and active-tile records are 64 bytes, while their currently named
fields end at byte 40. The established framework deliberately leaves bytes 41
through 63 available for actor-specific state. Turrets should define explicit
record-local fields for desired heading, search direction and search duration;
seekers should likewise define fields for orbit progress and validated origin-
pad identity. Per-type aliases may share physical tail offsets when their pool
types cannot overlap, but their symbolic names and ownership must remain clear.

No parallel global table is needed. Allocation, spawn and every relevant policy
entry must explicitly initialize the custom tail fields because the current
default-copy boundary ends at `sprite_frame`/`tile_frame` and a reused record
may otherwise retain old tail bytes. Newly admitted turrets reconcile the
current global policy before their first local update; returning to live play
initializes the ordinary reload field independently of the dedicated search
timer. Active-tile scrolling, destruction, crater transformation and
bottom-of-playfield deactivation remain unchanged.

Using separate hardware sprites for the title and prompt is the initial
implementation preference because it allows independent timing without a
combinatorial sprite sheet. The title may alternate between two or more
pre-rendered colour frames; the prompt uses the game's accepted ordinary font
at a readable size and alternates visible and hidden frames. If the qualified
hardware API makes one composite sprite materially simpler, it may instead use
combined frames, provided the two cadences and fixed placement remain the same.

Input dismissal is edge-triggered rather than level-triggered. GAME_OVER first
observes that all keys are released, then accepts the next fresh keypress. This
prevents fire, movement or another key held during the fatal frame from
instantly skipping the presentation. EXIT hides and releases every overlay
sprite, clears its timers and input latch, and requests `NEW_GAME`; it must not
return through the asset-loading splash or repeat uploads that are already
resident.

The hardware-sprite extension is not yet a proven dependency of the production
game. Before implementation, its exact VDP command ABI, sprite ownership and
cleanup behavior, supported dimensions/frame limits and required VDPlet release
must be frozen and qualified under the project's firmware policy. That gate is
part of `GAME-001J`, not permission to substitute the earlier affine-transform
experimental firmware or to change a hardware deployment silently.

## 17. Actor-record framework review gate

The Author's original intent was not a bag of loosely related sprite fields.
Each actor type was meant to declare its own record schema in its header, while
standard allocation, initialization, behavior-selection and release routines
kept actor construction on rails. The current code partially realizes that
design: sprites and active tiles use fixed 64-byte records, common offsets,
compact defaults blocks, bounded pools and shared spawn routines. Type headers
already own defaults and local behavior entry points.

The contract is nevertheless fragmented. Common definition copies end at byte
40; bytes 41 through 63 are available but have no canonical per-type schema
pattern or shared initialization contract. Multiple spawn variants duplicate
the common-field clearing sequence. Some transformations replace definitions
in place while preserving other record state. Behavior changes can be written
directly into callback fields, and recent validation hardening knows the legal
pool/type/program combinations without yet expressing a complete actor
lifecycle. These are signs of a framework that needs consolidation, not a
reason to discard it.

`ASM-011` is therefore an audit and design gate before the new seeker and
turret fields are implemented. It inventories every sprite and active-tile
type, pool, defaults block and lifecycle routine. For each path it records:

- the exact common and actor-specific byte schema;
- which routine allocates the physical record and how failure is reported;
- which bytes are copied, cleared, preserved or actor-initialized;
- ownership of `IX`, `IY`, `table_pointer` and `tile_table_pointer`;
- the callback/state identity installed and how it is validated;
- behavior on pool exhaustion, record reuse, cross-domain spawn, in-place
  transformation and deactivation.

The target convention remains the Author's existing model. Shared record
headers define universal fields. Each actor include declares clearly named
aliases for its custom tail bytes, together with bounds/overlap assertions and
an initializer that establishes every custom field after generic allocation.
Generic routines own only generic state; actor initializers and actor-owned
state transitions own type-specific state. Validated behavior setters should
express legal transitions without requiring callers to know callback-storage
details. Parallel state tables and ad hoc globals are disfavored when the
record has appropriate space.

The first deliverable is documentation, assertions and sentinel tests, not a
mechanical rewrite. Any remediation must be staged from demonstrated ownership
or initialization defects and reviewed before it changes working legacy paths.
This audit directly supports the enemy-controller and strategy-boundary stages
of the state-machine migration, but it is distinct from the post-stabilization
style unification reserved for `ASM-001M`.

## 18. Living attract mode and future autonomous replay

The post-load title boundary is currently static and blocking. After the image
containers have loaded, `game_init` runs the blocking joystick question, clears
the display, prints `Press any key to continue.` as ordinary text in a
one-line viewport, flips once and waits in `mos_getkey`. The gameplay mode,
bezel and starfield are initialized only after that keypress. There is therefore
no attract update loop, transparent prompt or moving background today.

`GAME-001K` replaces only that post-load boundary; it does not redesign the
accepted splash, AGNB loading progress or asset lifetime. The minimum TITLE/
ATTRACT phase establishes the playfield presentation and advances a starfield
by exactly one scanline per scheduler frame. A fixed hardware-sprite overlay
contains a stylized, transparent `Press any key to start` message in a readable
ordinary-sized font. It uses the same qualified overlay ownership and teardown
conventions as the GAME_OVER composition, while keeping its own frames, timers
and input latch.

The attract phase polls rather than waits. Entry first requires all keys to be
released so the key that dismissed a loader or configuration step cannot also
start the game. UPDATE scrolls once, advances overlay animation if any, samples
input and returns through one VBlank. A fresh keypress requests `NEW_GAME`;
EXIT removes attract-only sprites and clears its local state without deleting
resident bitmaps or reloading either AGNB container. The existing blocking
joystick Y/N question cannot remain inside this autonomous path. Its eventual
replacement must be a separate bounded configuration/start-flow state; the
exact UX can be settled without coupling it to attract rendering.

The later form is a classical title/demo loop backed by the planned autonomous
qualification player. Rather than attempting to run a language model on the
eZ80, the development agent learns to play through the headless RAM/input
harness, selects a good deterministic run and exports its input trace with the
matching game binary, map, initial RNG seed and terminal oracle. Attract mode
replays that performance through the real game controllers. Any human start
keypress interrupts it immediately; natural completion, death, timeout or a
runtime fault returns cleanly to the title presentation.

Demo execution is a disposable session. Its score, lives, RNG progression,
actor tables, map position and pending events must never become the starting
state of the human game. Replay identity is build-sensitive: a trace is admitted
only for the exact compatible build/map contract, and a mismatch falls back to
the minimum moving-starfield attract screen. This keeps the first implementation
small while providing a direct promotion path once `TEST-001` can produce a
game worth showing off.

A further stretch form is a genuinely live, nondeterministic AI attract game
rather than a selected recording. The enabling boundary should be a pluggable
per-frame input provider: human controls, deterministic replay and live policy
all produce the same normalized input snapshot consumed by the ordinary player
controller. Game logic must not branch into a separate AI simulation path.

The full development agent cannot be assumed to exist beside deployed Agon
hardware. A portable live attract therefore needs a bounded policy that can run
locally—whether a compact heuristic controller, a distilled learned policy or
another small representation produced after autonomous training. During
emulator development, an external agent may drive that same interface from RAM
telemetry, but that is a qualification and training arrangement rather than a
shipping dependency. A varying admitted seed can supply different live games;
tests retain a pinned seed and deterministic policy mode.

Live policy evaluation must fit inside one scheduler frame, remain interruptible
by a fresh human keypress, and use the same death/completion/stall/fault
watchdogs and disposable-session cleanup as replay attract. This is an exciting
promotion path, but it does not delay the minimum moving-starfield title or the
first deterministic showcase replay.

## 19. Deliberative play and mass-training research

“Teach the agent to play Nurples” has two intended meanings. The first is a
deliberative pilot: the development agent receives a structured snapshot of
the game state, reasons about threats, terrain, weapons and objectives, chooses
a keyboard action for a bounded number of frames, observes the result and
repeats. This is valuable precisely because its decisions can be inspected and
discussed turn by turn. The resulting observation, reasoning, action and state
trace must be replayable without asking the model to make the same decisions
again.

The second experiment is deliberately much larger: iterate a compact bot over
millions of games and retain the maximum earner. That loop must not invoke a
language model once per turn. It belongs in local compiled/headless tooling
that can reset cheaply, run without rendering, execute many isolated workers,
checkpoint progress and evaluate a small policy or search controller at high
speed. This avoids token cost and makes the result scientifically reproducible.

Both experiments consume one shared episode/step contract. An observation is a
versioned structured view of public game RAM: global and actor states, player
position and resources, visible actors and active tiles, map/scroll progress,
score, lives, timers, pending events and fault latches. An action is the same
normalized key snapshot used by a human controller, optionally held for a
bounded number of frames. Reset specifies the exact binary, initial snapshot,
map and RNG seed. Step returns the next observation, score delta and an explicit
terminal reason. The bot may supply input; it may not improve its result by
patching gameplay state.

`TEST-001` remains a small, stable regression player whose job is to exercise
known scenarios reliably. `AI-001` is the separate research laboratory. Its
mass-training algorithm is intentionally not frozen before throughput and
observation benchmarks: evolutionary search, cross-entropy optimization,
reinforcement learning or snapshot-assisted planning may each fit the game.
Whichever is selected must produce a compact policy and a complete provenance
record rather than an opaque unrepeatable high score.

The raw in-game score is the champion objective. Training may use auxiliary
signals to learn, but the hall-of-fame ordering cannot quietly substitute a
shaped reward. Episode caps and terminal oracles reject hangs, faults and
infinite farming artifacts; completion, level progress, remaining lives and
elapsed frames are retained as tie-break evidence. The laboratory keeps both a
highest-scoring individual run and an across-seed evaluation, because the most
spectacular attract replay and the most generally competent policy need not be
the same artifact.

Every promoted champion carries the game-build hash, map identity, initial
seed, policy/trainer identity, score, terminal result, observation checkpoints
and complete input trace. It must replay on the canonical emulator before it is
eligible for the attract screen. A trace can become the deterministic showcase;
a sufficiently small qualified policy can become the live nondeterministic
attract player. Neither promotion silently authorizes hardware deployment.

### MechAgon as the companion bot laboratory

The Author identified MechAgon as the natural partner for this work because it
is a bot-driven game by design rather than a human game being adapted for an
agent. It can exercise population management, bot-versus-bot matches,
tournaments, opponent sampling and policy evolution more directly than
Nurples. Nurples supplies the complementary problem: maximize score and survive
in a scrolling action game through the same kind of bounded observation/action
interface.

The collaboration should share infrastructure, not game semantics. Episode
process control, worker isolation, deterministic seeding, snapshot transport,
checkpoint/resume, artifact hashes, policy provenance, leaderboards and trace
storage belong in a reusable layer. Each project owns an adapter defining its
observation schema, legal actions, scoring or fitness objective, terminal
conditions and fault interpretation. Neither game should link against or
special-case the other's rules.

This gives the interface two genuinely different consumers early. A runner
that works only for one score counter and one keyboard map is not yet generic;
a runner that supports MechAgon populations and Nurples single-player training
without conditional game logic has earned that description. Cross-project
implementation and repository placement should be coordinated with MechAgon's
owner when `AI-001A` begins rather than improvised from this game checkout.
