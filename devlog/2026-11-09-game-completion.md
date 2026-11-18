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

### GAME-001AE Alexe review result and sprite-publication correction

The first stopped Alexe review failed the visible entrance-motion gate. The
ship appeared at the lower-left start position in its right-bank frame but did
not move. After the bounded entrance timer expired it jumped directly to the
centre endpoint, performed the flashing grace interval and entered ordinary
play. The Author also reconfirmed Alexe's broader approximately one-half to
one-quarter speed behavior, but other scene motion remained functional; that
host-speed observation is separate from this deterministic player defect.

The entrance controller was advancing its phase timer and 16.8 coordinates.
The defect was the VDP publication boundary: sprite position commands remain
queued until a drawing operation or explicit `VDU 23,27,15` sprite update.
`player_spawning` deliberately pauses terrain drawing, so its successive
absolute sprite moves had no operation to publish them. When entrance settled,
`player_grace` resumed world travel and the first background drawing operation
published only the already-final centre position, producing the observed
teleport.

`player_draw_record`, shared by entrance and damaged-flight choreography, now
issues `vdu_sprite_update` after setting the scripted position and frame. This
is intentionally local to scripted player states: normal gameplay already has
subsequent drawing work, while both entrance and death pause the terrain path
that previously supplied the implicit flush. The next visual gate must confirm
continuous entrance travel and continuous damaged-flight motion; this failed
candidate remains uncommitted.

The corrected stopped review is Oryx generation 413, request
`d07f7bf63061`, in `.emulator/game001ae-player-flow-r02`. Its 41,504-byte
binary has SHA-256
`ae8976d5503c6b7037c5afdd7f8afed82db1c289c6b71d4ae4a6d8c514c820d4`.
FAT-image readback matches that binary, its symbols, both AGNB containers, the
Lat38 font and the load-only autoexec byte-for-byte. All pinned runtime, MOS and
firmware manifests verify; the profile is stopped pending Author review.

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

## 20. First accepted implementation slice

The actor-record audit remediation and `GAME-001H` fireball presentation repair
were visually accepted on 2026-11-09. The accepted production behavior is:

1. every new fireball receives its directional frame at spawn rather than
   displaying pooled frame zero for the first eight-tick interval;
2. its displayed heading uses `Heading_p`, the final lead-adjusted projectile
   heading, before applying the artwork's required half-turn; and
3. both animation banks retain that orientation as the projectile travels.

The diagnostic Alexe fixture disabled seeker spawning and fireball/player
collision, then shortened turret reloads from the production 24-55-frame range
to 6-13 frames. This made moving-player lead shots easy to observe without
changing production balance. The Author first confirmed that stationary shots
were already correct and that only lead shots diverged, then accepted the
corrected build. The fixture-only rate and collision changes were not copied
back into the game.

The same accepted slice makes explosions deactivate at all playfield edges and
establishes the actor-tail generation, initialization and transition contracts
needed for the next player, seeker and turret states. The Oryx checkout was
released immediately after acceptance with fresh empty-host proof.

## 21. Player-flow implementation candidate R01

The next bounded candidate implements the parts of `GAME-001A` and
`GAME-001E` whose behavior is already explicit, without claiming visual
acceptance or closing either task:

1. Normal input is clamped below a 64-pixel death runway. The shared sprite
   boundary remains unchanged for enemies and effects.
2. A fatal hit saves the last commanded left/centre/right frame, clears player
   collision and initializes a slow forward damaged-flight vector. Lateral
   direction changes are random but latched for 24-55 frames; a side collision
   accepts the generic mover's inward reflection and holds it for 32 frames.
   The displayed frame wiggles around the saved commanded pose on its own
   shorter random cadence.
3. Only the existing long trailing-explosion phase advances damaged flight.
   Its previously accepted trail cadence and terminal explosion allocation are
   unchanged.
4. Passive terrain and active-tile spatial movement independently consult the
   published player lifecycle. They stop while the player is DYING, DEAD or
   SPAWNING, while active-tile animation/controller callbacks keep receiving
   one bounded update per outer frame. Ordinary travel resumes when the
   entrance settles into GRACE, matching the accepted respawn policy.
5. `player_dead` no longer restores `player_starting_score`, reinitializes the
   level, or clears runtime actor/tile tables. It decrements the life count and
   begins a new player-record lifetime in the existing world.
6. Initial level entry and mid-level respawn share one 56-frame banked rising
   swoop. Three fixed-point legs land exactly at X=120, Y=224: 16 frames at
   (2,-1), 24 at (3,-2.5), then 16 at (1,-1.25) pixels per frame. Assembly-time
   invariants reject one-sided timing/velocity changes that miss that endpoint.
7. The settled ship enters a distinct 60-frame `player_grace` handler. Control
   and weapons are enabled, collision remains disabled, and the sprite blinks.
   Expiry explicitly shows the sprite, enables collision, publishes
   `player_alive`, and asks each orbiting seeker to re-enter its initialized
   seeking behavior.

This candidate deliberately leaves the mid-level safe-endpoint search open.
Grace prevents immediate collision during evaluation, but `GAME-001E` cannot
close until the preferred endpoint is moved away from a ground collidable and
its immediate wake. The larger seeker return-to-own-pad protocol, turret search
and game-over behavior likewise remain separate actor slices.

Static qualification passes 46 runnable safety tests and Ruff on the Pi host.
The one assembly smoke test is unavailable there because `ez80asm` is absent.
The first native Oryx compile exposed two defensive-assertion issues before any
profile was staged: `sprite_bottom` is defined after `player_state.inc`, and
ez80asm evaluates compound arithmetic strictly left-to-right rather than giving
multiplication conventional precedence. The entrance now owns a named start-Y
constant, `screen.inc` proves that it equals the ordinary sprite floor, and
each product in the endpoint proof is isolated before the final sum. Focused
tests pass and the native build now assembles without diagnostics.

Oryx request `ce27bce496c3` checked out generation 411 with fresh empty-host
evidence. The resulting review binary is 41,500 bytes with SHA-256
`6d49b057fe83b5d0dbd3de4f84e4aea3f66a8af795216fc62521f1d451ee94dd`.
It is staged, but not run, in the dedicated load-only Alexe profile
`.emulator/game001ae-player-flow-r01`. SD-image readback matches the binary,
symbols, both AGNB containers, Lat38 font and autoexec byte-for-byte. The
profile retains pinned VDPlet `dc43ecb8...` and MOS RC3 `d7a51147...`; runtime,
payload, launcher syntax and canonical executable-layout verification all
pass. Human visual acceptance remains required before these gameplay changes
may be committed.

### Safe-endpoint follow-up contract

The remaining `GAME-001E` endpoint resolver can stay entirely within the
existing actor and active-tile frameworks:

1. Run one deterministic resolver for both initial entry and mid-level
   respawn. The initial empty active-tile set naturally accepts the canonical
   point; no separate level-start animation is needed.
2. Search 16-pixel-aligned endpoints centre-out. Try X=120, then alternating
   left and right offsets of 16 pixels through the legal 0..240 sprite range.
   For each X, try Y=224, 208, 192 and 176. This preserves the lower-third
   composition while covering the entire useful horizontal field.
3. Validate every dense-stack slot with `tile_stack_pointer_valid`, every
   record with `tile_table_record_pointer_valid`, and require a live record.
   Treat nonzero `tile_shield_damage` as the authoritative ground-collision
   predicate because those are exactly the active tiles whose controllers can
   collide with and damage the player today. Pads, turrets and craters remain
   scenery or projectile sources rather than inventing new solid-body physics.
4. Test a 16-by-80 rectangle at each candidate: the player's final 16-by-16
   box plus a 64-pixel corridor immediately forward (toward decreasing Y).
   Reuse `check_collision_box` through a bounded scratch collider rather than
   duplicating its inclusive AABB rules.
5. Store the accepted fixed-point target X/Y and the entrance direction in
   named unused fields of the player actor record. An endpoint at or right of
   centre starts 120 pixels to its left and uses the current positive-X swoop;
   a left endpoint starts 120 pixels to its right and mirrors all X velocities
   and bank frames. Y starts 96 pixels below the chosen endpoint. Thus every
   candidate uses the same 56-frame curve with no terminal snap from an
   unrelated path.
6. The finite grid should always find a legal point in authored content. If it
   does not, latch a dedicated player fault and retain collision suppression;
   never silently publish an unsafe `player_alive` actor. Deterministic tests
   must cover canonical acceptance, left/right mirroring, vertical fallback,
   invalid stack/record rejection and the no-candidate fail-closed result.

## 22. Provenance-pinned Lode Runner display font

The 6502 workspace already contained a committed, reproducible extraction of
Lode Runner's original status lettering. It comes from the running game's RAM
sprite table at `$AD00`, not the Apple II character ROM or a screenshot. The
extraction records the source RAM hash, the exact sprite mapping, and the
annotated reverse-engineering reference. This resolves the provenance question
without interrupting the 6502 agent's active performance lease.

Nurples now has a deterministic 8x8 hybrid generator. The original 10x11
letter cells contain three blank top rows; all eight remaining A-Z rows fit in
the first storage byte. The adaptation therefore copies those eight rows
exactly with no scaling or thresholding. It writes each letter into both its
uppercase and lowercase ASCII slots so existing mixed-case strings deliberately
render ALL CAPS. The current Lat38 font remains byte-for-byte authoritative for
digits, punctuation, symbols and every other glyph.

The same source rows now generate both alternating `GAME OVER` frames and the
two prompt sprites. Their SVGs are integer rectangles and their transparent
PNGs use exact Agon64 colours with binary alpha. The title remains 240x64; each
longer original-font prompt is 208x16. Ten focused tests prove exact letter
rows, uppercase/lowercase aliases, preservation of all 204 fallback glyphs,
current generated outputs, overlay geometry, palette and alpha. Ruff passes.

This is asset preparation, not runtime adoption. The generated 2,048-byte font
is not yet named in `RUNTIME_FONT_NAMES`, copied to `tgt/fonts`, or selected by
assembly. The existing Lat38 runtime therefore remains unchanged for the
pending player-flow review. Runtime adoption and hardware-sprite integration
retain their separate visual, lifecycle and feature-variable gates.

## 23. GAME OVER and attract integration boundary

The next presentation work has a narrow reusable seam, but production hardware
sprites remain gated. The source audit found no need to invent another global
state model: the replacement graph already owns distinct TITLE and GAME_OVER
states and legal transitions to NEW_GAME. What remains is to replace the two
legacy blocking handlers with bounded lifecycle callbacks after the standalone
overlay protocol is qualified.

The current implementation boundaries are:

1. Legacy `game_over` prints opaque text, delays, blocks in `waitKeypress`, and
   returns through `game_init`, which clears every VDP buffer and reloads all
   resident assets. The replacement must instead use bounded ENTER/UPDATE/
   RENDER/EXIT work and advance directly to clean session initialization.
2. The existing title prompt and blocking joystick question are embedded in
   `game_init` after both AGNB containers load but before gameplay display
   setup. TITLE therefore needs to become the owner of the moving background,
   overlay, input latch and teardown; joystick selection becomes its own later
   bounded start-flow boundary rather than an attract-loop dependency.
3. Both states require identical release-then-press semantics. A new isolated
   `input_gate.inc` now implements that policy as a one-byte, caller-owned
   state machine driven by a normalized any-key-down sample. It performs no MOS
   call, delay or wait. Its pure-CPU fixture covers held entry input, arming on
   release, one event per fresh press, repeated held samples, rearming and
   explicit reset. It is not included in the production binary yet, and native
   assembly/headless execution remain pending on a host with `ez80asm`.
4. The first safe production dependency is still the standalone overlay
   qualifier described in `docs/hardware-sprite-overlays.md`. It must prove
   absent, present-zero and present-nonzero feature-variable restoration,
   repeated cleanup, wide-sprite stability, transparency, ordering and both
   title/prompt frame sets on the pinned Alexe firmware. Production must not
   set feature variable 2 or activate sprite slots 20/21 before that result.
5. After qualification, GAME OVER is the smaller first integration. It already
   begins from a constructed playfield and can exercise overlay ownership,
   one-scanline world scrolling, non-hostile actor policy, the shared key gate,
   cleanup and asset-preserving NEW_GAME entry. TITLE then reuses the accepted
   overlay/key ownership while moving post-load display setup out of the
   blocking `game_init` boundary.

The staged player-flow profile remains byte-frozen while this audit proceeds.
None of the isolated gate source is linked into its binary or changes the
pending visual review.

The first source-only part of that qualifier is now prepared, likewise outside
the production include graph. `hardware_overlay_feature.inc` builds the six
frozen VDP-resident control buffers, snapshots true variable presence and the
full 16-bit value, enables feature variable 2, then restores via SET or CLEAR
before releasing its programs. Four host tests independently decode the exact
command payloads, prove snapshot-before-enable and restore-before-clear order,
and prove the module is not linked into `nurples.asm`. These tests pass, but
native assembly and all Alexe/Console8 runtime gates remain open; source
preparation is not evidence that the feature is safe to adopt.

The vector generator now also emits canonical RGBA2222 payloads for all four
overlays through the installed `agonutils` adapter. Payload sizes are exactly
240x64 for each title and 208x16 for each prompt; regeneration and eleven
focused art/protocol tests pass. They remain outside the production AGNB
catalog, so the staged game binary and its runtime containers are unchanged.

A standalone runtime fixture is source-complete for the eventual gate. It uses
the real production sprite IDs: transparent software slots 0..19, a visible
software ordering witness at 19, and hardware overlays at 20/21. For each of
the absent, present-zero and present-`BEEF` incoming states it reloads the four
loose test bitmaps, runs a moving striped background for 360 frames, alternates
both title colours and both prompt frames, moves the title, hides/shows the
prompt, then performs the frozen detach/prefix/demote/release/restore order.
VDP-resident conditionals overwrite `FAIL` with `PASS` only when the exact
post-teardown variable state matches the case. This fixture is not assembled,
deployed or run yet: the Pi lacks `ez80asm`, the current Oryx lease is frozen
for the separate player-flow review, and runtime qualification may not borrow
that review's machine scope.

The fixture now has a deterministic build-only packager. Its source check
validates all four width-times-height payload sizes without an assembler; the
native action will assemble `overlay.bin`, copy only the four qualifier
payloads and load-only autoexec, then write `SHA256SUMS`. Three unit tests prove
the exact source inventory, a self-verifying bundle and drift rejection through
a fake finite assembler. The builder contains no profile deployment or process
launch path, keeping build and emulator deployment separate.

## 24. Isolated turret-slew prerequisite

The turret tracking feature now has a source-level arithmetic primitive without
changing any turret record or production callback. `heading_slew.inc` accepts
current heading, desired heading and a tunable maximum step in the game's 16.8
angle256 format. It discards poisoned ADL upper bytes, selects the shortest
wrapped path, retains all eight fractional bits, snaps only when the remaining
distance fits inside the step, and resolves an exact half-turn in the positive
(clockwise compass) direction. A zero rate is stable without falsely reporting
that the destination was reached.

The accompanying pure-CPU fixture covers equal, sub-step, exact-step,
clockwise, counter-clockwise, both wrap directions, fractional wrap, both
half-turn orientations, zero rate and poisoned upper-byte inputs. It remains
outside `nurples.asm`; therefore the frozen player-flow binary is unchanged.
Native assembly/headless execution must wait for an appropriately scoped build
lease because the Pi host has no `ez80asm` and the current Oryx checkout belongs
exclusively to the pending human player-flow review. After that proof, the
primitive can back `GAME-001I`'s record-local physical barrel heading while the
targeting computer continues to supply only the desired lead heading.

The next source-only layer now defines that record-local boundary. The turret's
unused actor tail has explicit three-byte desired-heading, one-byte search-
direction and one-byte search-duration fields. Live tracking advances the
physical `tile_heading` at two angle256 degrees per frame through the wrapped
slew primitive. Non-firing search latches a direction for 24-55 frames, flips
it on expiry and uses the same limiter; its timer cannot alias the legacy reload
timer. A separate shot-preparation routine converts the physical barrel heading
to projectile components immediately before allocation, so the ideal lead
solution cannot leak back into a fired shot. Five host tests cover wrap motion,
half-turn policy, latched search, named field separation, physical-shot data
flow and production isolation. Its six-case pure-CPU fixture is likewise ready
for the later native build/headless lease.

## 25. Isolated safe-respawn endpoint resolver

The deterministic endpoint search from the `GAME-001E` follow-up contract is
now implemented as a source-only module without changing the frozen player-flow
binary. `player_spawn_resolver.inc` tries the canonical X=120 endpoint first,
then alternates left and right in 16-pixel steps before checking the two legal
screen-edge positions. At every X it tries Y=224, 208, 192 and 176. Its probe is
the final 16-by-16 player box plus the complete 64-pixel forward wake, expressed
as one 16-by-80 inclusive AABB extending toward decreasing Y.

The resolver walks the existing dense active-tile stack rather than creating a
second scenery index. Every stack address and record pointer passes the hardened
validators, every referenced record must be alive, and only records with
nonzero `tile_shield_damage` obstruct a candidate. It calls the production
`check_collision_box` contract. Malformed stack state, malformed record state
or an inactive dense-stack record aborts the whole resolution; exhaustion of
all 68 candidates latches a separate no-candidate fault. Nothing can publish an
unsafe fallback point.

Five host tests prove the frozen search order, forward-wake geometry, horizontal
and vertical fallback, non-collidable scenery handling, full-grid exhaustion,
validator/collision dependencies and absence from the production include graph.
The pure-CPU assembly fixture additionally defines nine cases for canonical,
left, right, vertical, corrupt-stack, corrupt-record, inactive-record and
no-candidate results. Host tests and Ruff pass. Native assembly and repeated
headless execution remain pending because this host lacks `ez80asm` and the
current Oryx lease is still exclusively the Author's player-flow review.

After native qualification and visual acceptance of the current candidate, the
production integration will copy the chosen fixed-point target and mirrored
entrance direction into named fields in the player actor record. It will then
derive the off-screen start point from that actor-owned result rather than from
global resolver scratch.

## 26. Isolated seeker launch identity and retreat policy

The next `GAME-001B` prerequisite is source-complete without changing the
frozen production binary. Every seeker can retain the one-byte physical record
id and allocator generation of the exact small pad that launched it. Resolution
reconstructs that record from the fixed tile table, then requires the hardened
record validator, matching generation, live flag, `tile_pad_small` type and a
body that still intersects the visible field. A recycled record can therefore
never masquerade as the seeker's original pad.

The same isolated module gives each seeker an actor-owned completed-orbit count.
Two complete passes through the preserved orbit behavior enter retreat. A
still-valid own pad selects a non-hostile homing callback and overlap docks by
deactivating the seeker. A missing, destroyed, recycled or off-screen pad
selects the nearer horizontal edge (with X=120 deterministically choosing the
right), computes outward velocity once and deactivates on the first boundary
crossing. Neither retreat callback calls player or weapon collision handlers.

Five host tests cover identity reuse, type/alive/visibility rejection, the edge
tie policy, named actor fields, non-hostile data flow and production isolation.
A twelve-case pure-CPU assembly fixture additionally covers capture, exact
resolution, both retreat publications, orbit completion, edge exit and pad
docking. Native assembly/headless execution remains queued behind the current
exclusive player-flow review; `seeker_retreat.inc` is not included by
`nurples.asm` and cannot affect that staged binary.

## 27. Isolated starfield-only hyperwarp scroller

The existing terrain path already carries the required warning that its
one-scanline loop is gameplay-only and must not be generalized for transitions.
The separate `starfield_warp.inc` prerequisite now advances only the existing
256-by-256 starfield stream by zero through sixteen scanlines in one outer
frame. Every chunk scrolls the playfield viewport, resizes the exposed-strip
viewport to exactly the chunk height, and plots the selected starfield bitmap
at the existing `bg_y_plot` phase. Bezel and cockpit coordinates are never in
the scroll viewport.

A displacement that crosses a 256-line image seam is split into the exact tail
of the old tile followed by the head of one newly selected tile. Because the
maximum step is sixteen, the routine emits at most two bounded scroll/plot
chunks per frame. It rejects larger inputs before VDU output, and restores the
ordinary playfield viewport on every successful return. It does not call
`tiles_plot`, `do_active_tiles` or the gameplay `bg_plot` stepper.

Five host tests exhaustively cover all 256 phases and all seventeen legal
displacements, proving exact displacement conservation, modulo phase, at most
one tile change and at most two chunks. A four-case assembly fixture records
the emitted chunk widths for first-tile, seam-crossing, maximum-speed, zero and
invalid requests. Native assembly/headless execution and production inclusion
remain pending behind the current human review. The victory loop, terrain-
drain phases and acceleration schedule remain separate `GAME-001F` work; this
slice provides only their exact starfield rendering primitive.

## 28. Native prerequisite build batch

The Pi's canonical `~/.local/bin/ez80asm` path is absent, but the user-owned
agon-fsim qualification assets contain an existing aarch64 build at
`agon-fsim/tests/0053-flight011-agon/ez80asm`, SHA-256
`00f194d6e0d3b9906130a3f10e6bc05a3bc767a652ea9dfc687aae3c9559b46d`.
It was used read-only for a finite native syntax build. The first pass caught an
invalid fixture-only `add iy,hl`; replacing it with supported pointer transfer
made the complete batch assemble without diagnostics.

The native fixture binaries and SHA-256 results were:

1. input gate: `dc3a8016b5543b05ffd32a6f84d07ef3351569f683a5319903d66f403ad76ad1`;
2. wrapped heading slew: `c7a6179b5b7427621c6b04883a3148f2049406afc3d12278113a99a007d65c22`;
3. safe spawn resolver: `ea8f8d404e444d128e462f88853a12bf371e289d2896c4c21f44e33b966649ff`;
4. turret tracking/search: `90a1fa1617d5f3ed97a53433d946e84c5f715b792c69472bbe5253e6c5c79c98`;
5. seeker identity/retreat: `c52d8a7dbab2a4ec1c56174c20adf700f4f42c8da7aaba4aab8c0b2f2d59a18f`;
6. starfield warp: `baf8ad5016483541ba5677b08678fa2e0ee7fa81a967595fbff89f8f440851b3`;
7. overlay feature transaction: `8e4a69485c6578b7e2d35711febe72e0aa81fc158ecc8e3500ad5e307dd5de90`;
8. overlay runtime qualifier: `d5caada69760a6587b05c4de6e64efcb36382eecab1d6851c2c4a12d81b76c81`.

The assembler's short filename limit required the two long-named overlay
fixtures to be copied with short names into a disposable `/tmp` mirror; their
source includes and resulting binaries were otherwise unchanged. These hashes
identify ephemeral build evidence, not checked-in products. The headless-test
repository/executable is not present on this Pi, and the current Oryx checkout
is exclusively the Author's player-flow review, so repeated guest execution and
the graphical overlay qualifier remain pending rather than borrowing that
lease's scope.

## 29. Isolated level-transition timing controller

The starfield renderer now has a source-only bounded phase controller. It emits
exactly 636 outer frames: 336 ordinary one-scanline terrain-drain frames, 90
normal-speed victory-centering frames, eight 8-frame acceleration stages at
speeds 1, 2, 3, 4, 6, 8, 12 and 16, a 90-frame speed-16 hold, then seven
8-frame deceleration stages at 12, 8, 6, 4, 3, 2 and 1. Completion is published
only after the final speed-1 frame, so the next level cannot enter while the
starfield is still accelerated.

The controller freezes each frame's speed and terrain-policy bit before it
mutates the next phase. Consequently the global renderer receives exactly 336
terrain-enabled frames and can route every later frame exclusively to the
starfield warp primitive. It owns no VDU or actor calls and fails closed on an
invalid phase. The victory-loop trajectory and integration remain separate.

Host tests prove the 636-frame length, terrain boundary, speed histogram and
absence of rendering/terrain calls. The native fixture assembled without
diagnostics through the same pinned aarch64 tool; its ephemeral binary SHA-256
is `ff989c77d4f12babe41e27b025a8ba34d60411c5bd61d61380db96a056d720e4`.
Repeated guest execution remains pending with the other fixtures.

## 30. Isolated player victory flight

The actor side of `GAME-001F` now has a source-only bounded controller. It first
homes at three pixels per frame to the existing safe lower-centre anchor
(120,224), flies one 64-frame loop at four pixels per frame while its heading
advances four angle256 degrees each update, then homes to the playfield centre
(120,168) and publishes HOLD. The first half of the loop uses the left-bank
ship frame and the second half the right-bank frame; gathering and centring bank
toward their target and settle on the centre frame.

Victory entry removes player collision and never polls ordinary input or calls
hostile collision handlers. Both approach legs have 120-frame fail-closed
timeouts; an impossible motion result latches an error and snaps to the safe
phase endpoint rather than blocking the outer scheduler. HOLD zeros all motion
before reporting ready, allowing the independent 636-frame transition schedule
to keep the centred ship fixed during hyperwarp.

Five host tests prove the complete heading cycle, 32/32 bank split, actor-owned
field layout, collision/input isolation and centre-before-HOLD ordering. The
native assembly fixture covers entry, gather, exactly 64 loop updates, centre,
HOLD and corrupt-phase recovery. It assembled without diagnostics; the
ephemeral binary SHA-256 is
`86a82ff70b475bcc7029cc44d6ca5ac4b9ee7a7e9ff8ed8c0132ebaafd0a7db7`.
It remains outside `nurples.asm` pending guest execution and later production
integration.

## 31. Shared attract/game-over presentation animator

The device-independent part of `GAME-001J/K` now has one bounded controller.
GAME OVER alternates its two title-colour frames every eight outer frames;
both GAME OVER and attract mode toggle their one mode-specific prompt sprite
visible/hidden every thirty frames. Each update clears and republishes explicit
title/prompt dirty bits, then advances the shared release-before-fresh-press
input gate. A held loader/death key therefore cannot dismiss either state.

Enter validates the requested presentation mode, initializes all cadence and
visibility state, marks both outputs dirty and resets the input gate. Exit
clears volatile ownership only after the eventual hardware renderer has
detached its resources. The controller contains no VDU, MOS, delay or blocking
input call and deliberately does not select hardware sprite buffers.

Four host tests prove the independent 8/30-frame cadences, shared input-gate
dependency, device isolation and absence from production. Native assembly
found and corrected two invalid direct-memory `DEC` instructions before the
fixture passed. Its ephemeral binary SHA-256 is
`4cc70ee32e95496e179d63c1309c8b6526683fb35d443edc00e1ccffff0d79e9`.
Guest execution and hardware renderer integration remain behind their existing
qualification gates.

## 32. Durable device-free qualification batch

`tests/run_completion_fixtures.py` now owns the next deterministic gate for all
nine device-free prerequisites: input gating, heading slew, safe respawn,
turret tracking, seeker retreat, starfield warp, transition timing, victory
flight and presentation animation. It copies the assembly sources and fixtures
to a temporary short-name mirror, avoiding the assembler's filename limit,
then either assembles only or runs every binary repeatedly through
`agon-headless-test run-direct` in ADL mode with exact guest status `0x2A`.

Assembler and headless runner paths remain explicit overrides. Discovery first
uses `PATH`, then the canonical local assembler location and the existing
user-owned aarch64 qualification binary. The finite batch defaults to twenty
runs per fixture (180 total), uses per-run instruction/cycle/wall limits, stops
on the first failure and leaves no source-tree build products. Its
`--assemble-only` mode passed all nine fixtures on this Pi. Actual headless
execution remains deferred because that repository/runner is absent here and
the Oryx emulator lease is still frozen for the Author's player-flow review.

## 33. Accepted player flow and integrated level-start starfield approach

The Author accepted the corrected GAME-001AE R02 visual review: the explicit
sprite update restored continuous entrance motion on Alexe. Generation 413
was released with fresh empty-host evidence. The player-flow slice remains
uncommitted until the Author separately authorizes its commit.

The initial level entrance now owns a 56-frame starfield-only deceleration
matched exactly to the already accepted ship swoop. Eight seven-frame stages
run at 16, 12, 8, 6, 4, 3, 2 and finally 1 scanline per outer frame. The last
seven entrance frames therefore already use normal gameplay speed; only on the
following GRACE frame does the ordinary one-scanline terrain pipeline admit
its first row. Assembly-time invariants require the speed table to contain
exactly eight entries and its total duration to equal the three entrance legs.

The player record owns three named fields for whether this presentation is
enabled, its current deceleration stage and its stage timer. The global
gameplay renderer queries that actor-owned policy, selects the separate
`starfield_warp_scroll` path, and bypasses `tiles_plot` for that frame. The
multi-scanline routine remains absent from the gameplay terrain module and
cannot admit or activate terrain.

Fast scrolling is deliberately enabled only by `player_init_level_entry`.
`player_init_respawn` uses the same banked ship trajectory with the starfield
effect disabled: after a mid-level death, the playfield already contains
rasterized terrain and active records, so scrolling the entire VDP viewport
would move pixels without moving their collision state. That context split is
an actor field, not a parallel table or inference from current tile position.

The production game assembles. Fifty-three starfield and assembly-safety tests
pass with the qualified aarch64 assembler, including native assembly of the
starfield chunking fixture. The isolated headless runner remains unavailable
on this Pi. The uncommitted candidate binary is 41,896 bytes with SHA-256
`81704e4b108bd51f71509ccdc3bec4239d7a14133c7b94f97834be95c56f2f2f`;
visual review is still required.

The stopped Alexe review candidate is staged under Oryx FIFO generation 414 in
`.emulator/game001ea-starfield-approach-r01`. Its private card image has
SHA-256 `9af79a00e46e75364db10b0cef4174b846f21a48d58c5007fa691b829220223b`.
The binary and 52,593-byte symbols file were extracted back out of that image
and compared byte for byte with the payload; their SHA-256 values are
`81704e4b108bd51f71509ccdc3bec4239d7a14133c7b94f97834be95c56f2f2f` and
`4bbb80f4b8baa29a1e2106a05a7104cc7aba9d4157b4bc27b7d971c0ad24d391`
respectively. All payload, runtime and MOS release manifests pass, as does the
launcher syntax check. No emulator was launched by the agent.

## 34. Starfield seam repair and damped-sine entrance R02

The Author's R01 review found visible gaps at starfield tile boundaries and
asked for a substantially longer maximum-speed approach followed by a
sinusoidal ship weave whose amplitude decreases into the final position.

The seam audit found a real register bug in the display setup: the intended
graphics background colour was loaded into `A`, overwriting GCOL's plotting
mode, instead of being supplied in `C`. Display setup now uses the documented
register contract, and `init_bg` explicitly selects the starfield's darkest-
blue ground as the scroll-fill colour. An initially added speculative guard
scanline was removed after Author review: the fast renderer retains the exact
minimal `0..chunk-1` viewport, matching the displacement without overdraw.

The entrance is now exactly 112 outer frames. Starfield speed remains 16 for a
new 56-frame hold, then follows the existing eight seven-frame stages at 16,
12, 8, 6, 4, 3, 2 and 1. Consequently deceleration begins after 63 maximum-
speed frames, and the final seven frames still run at normal gameplay speed
before terrain is admitted.

The player starts at `(8,320)` and rises toward `(120,224)` while an angle256
phase advances eight units per frame and a 112-pixel horizontal amplitude
falls by one pixel per frame. This yields three and one-half broad weaves whose
envelope settles at centre; the bank frame follows the sign of the sine
derivative. The generic `motion_damped_sine_position` primitive owns no actor,
player or rendering state. The player schema owns its named phase and
amplitude bytes, leaving the same math available for a later enemy movement
program without a parallel attribute table.

The production game assembles and all 55 focused starfield and assembly-safety
tests pass, including native assembly of the starfield fixture. The uncommitted
R02 binary is 41,954 bytes with SHA-256
`28d5153b1896e4d4f75abd668adced49024c49250206c48a98b2dd90f848a8c5`.
The stopped Alexe profile is
`.emulator/game001ea-starfield-approach-r02`; its private card image has
SHA-256 `14a20dc5139ef4fdc61c22b8e895a61716c75699d01f9ce56bf77a0807062796`.
Binary and symbols were extracted from that image and compared byte for byte,
and all payload/runtime/MOS manifests pass. No emulator was launched by the
agent.

After discussion, the Author explicitly rejected the speculative guard-line
overdraw. R03 restores the fast viewport to exactly `0..chunk-1`; only the
confirmed GCOL/register and scroll-fill correction remains. The production
build and all 55 focused tests still pass. The R03 binary SHA-256 is
`e9276423e8f30b885bbc5b10f48df4fa44fb672caefbc2c88277777846e04da2`.
Its stopped Alexe profile is
`.emulator/game001ea-starfield-approach-r03`, whose byte-read-back private card
image has SHA-256
`9632898136be8986821d27d759592556d59981f78f867fc6072540374333d439`.

## 35. Native OBS evidence for R03 starfield discontinuity

At the Author's request, the exact R03 minimal-viewport build was rerun in an
isolated Alexe profile on Oryx generation 414. A private Xvfb display supplied
only the emulator and OBS; keyboard automation issued `RUN`, answered the
joystick prompt with `n`, then released the splash gate immediately after OBS
started. Oryx CPU boost was not changed.

The retained native crop is 1280x960 H.264 at 30 fps and 7.1 seconds long. It
includes one second at the any-key gate followed by the fast entrance. Later
frames clearly reproduce the horizontal exposed-strip discontinuity with the
speculative guard scanline absent. Evidence is stored outside Git under
`.emulator/captures/game001ea-starfield-r03/`:

- `spawn-native-30fps.mp4`, SHA-256
  `aa74987315c7f533bd86ef52cf0181ca2b14dd19eb1c155f47fba95f86e1c12b`;
- `contact-5fps.png`, SHA-256
  `86d8d35e7e1308f3722f5e6a39a36582655a908acc394295dddd401c2f9186eb`.

All owned emulator, OBS and Xvfb processes were validated by exact executable
path and stopped. Fresh whole-host inspection found no emulator processes, and
generation 414 was released with no queued successor.

Native-frame review established that the discontinuity is cumulative rather
than a bad starfield tile seam. The first exposed row contains red/black
cockpit artwork from immediately above the playfield. Every subsequent fast
scroll pushes that row downward and exposes one more, so the band continues to
grow through the deceleration stages until control returns to the ordinary
one-scanline gameplay renderer. This is direct evidence that the fast bitmap
plot must include the one-scanline VDP clipping guard. R04 therefore restores
the guard only in `starfield_warp_render_chunk`; it does not alter the gameplay
terrain viewport or its optimized redraw loop.

All 55 focused starfield and assembly-safety tests pass with the qualified
aarch64 assembler, including native assembly of the deterministic chunking
fixture and the production game. The resulting 41,954-byte R04 binary has
SHA-256 `28d5153b1896e4d4f75abd668adced49024c49250206c48a98b2dd90f848a8c5`.
It is byte-for-byte the prior visually clean R02 candidate, which independently
confirms that the guard line is the only code delta between R03's accumulating
band and the earlier result.

## 36. Integrated actor flow and level-transition checkpoint

The previously isolated safe-respawn, turret-tracking, seeker-retreat,
victory-flight and transition controllers are now connected to their existing
actor and global-state boundaries. Respawn resolves a safe target before the
player record is published, stores the selected target and entry direction in
named player fields, and reuses the accepted swoop/grace sequence. Live turret
aim now passes through the two-degree-per-frame physical barrel slew; both the
projectile velocity and artwork use that physical heading. During every
non-live player phase, turrets run their bounded alternating search motion and
do not fire.

Each seeker captures its exact launch-pad id and generation at spawn. In a
non-live player phase it becomes harmless, completes two orbits, then returns
to that same still-visible pad or leaves through the nearest horizontal edge.
Launch pads continue their own animation and bounded spawn cadence during
those phases, so newly created seekers enter the same harmless orbit policy.

Level completion no longer jumps directly to the next map. The controller
drains the final 336 scanlines through the original gameplay-only terrain
renderer, gathers the collision-free player into a 64-frame victory loop,
centres it, accelerates the separate starfield renderer to 16 scanlines per
frame, holds, then decelerates through 12, 8, 6, 4, 3, 2 and 1 before admitting
the next level. Score and resident game resources are preserved. The normal
terrain renderer is not generalized to multi-scanline scrolling.

Native assembly exposed and corrected one stale fixture schema boundary in the
victory controller. The overlay tests and builder now use a short-name mirror
because ez80asm rejects the original long fixture basenames; runtime bytes are
unchanged. The complete host suite passes: 163 tests, with only the known
device-free input test skipped because `agon-headless-test` is absent on this
host. The production game assembles. The standalone hardware-overlay bundle
also builds and verifies under `/tmp/nurples-overlay-runtime-r01`; it remains
outside production pending the frozen Alexe visual qualification.

## 37. Resident overlay assets and standalone Alexe gate

The four frozen RGBA2222 overlay bitmaps now belong to the ordinary
`game.agnb` catalog at IDs 63990 through 63993. The game container therefore
holds 359 records and remains deterministic. Generated assembly names replace
the renderer's private numeric literals. This asset-catalog change alone does
not enable hardware sprites: the feature transaction, renderer, controller and
lifecycle modules remain outside `nurples.asm` until the standalone visual gate
passes.

The dormant renderer owns hardware sprite slots 20 and 21 above the existing
20-slot software prefix. It defines both slots before publishing a 22-slot
prefix; teardown hides both, restores prefix 20, clears their frame lists,
demotes both slots and then restores the exact incoming VDP variable-2 state.
The three transactional control buffers are visit-owned and cleared after the
restore program completes. The four AGNB bitmap buffers are instead immutable
process-owned assets and remain resident across attract and GAME OVER visits.
This is required to start a second game without reloading assets; the
standalone loose-file fixture has a deliberately shorter buffer lifetime and
still releases its own copies after every trial.

A stopped pinned Alexe profile was staged on Oryx at
`.emulator/game001jk-hw-overlay-r01` under FIFO generation 415, request
`fa5c12e606f6`. Its card is load-only and its manifests prove VDPlet
`dc43ecb8…` and Eirene MOS RC3 `d7a51147…`. The Author must visually accept
three finite ABSENT/ZERO/BEEF trials—wide title and prompt animation,
transparency, movement, ordering, teardown and exact restoration—before any
production include is enabled. No agent-launched GUI is involved. The Author
later revoked the idle generation-415 lease before launch so another queued
worker could use Oryx. The manager released it without killing any process;
the stopped profile and evidence remain preserved, but they are not an active
checkout and must not be launched without a new reservation and fresh idle
proof. No visual result was obtained, so the production gate remains open. A
fresh non-preemptive request, `GAME-001JK-HW-OVERLAY-R02` / `49dd20da68fb`, is
queued behind the current worker for the same Author-only finite review; it is
only a reservation and confers no checkout or launch authority.

The preceding worker later released Oryx with an empty-host snapshot. R02 then
checked out generation 417 under lease `b97c5606a0a8477a88f373a1583415c2`
using a fresh independent empty-host observation. Before review, every runtime
file passed `.runtime.sha256`: VDPlet is exactly `dc43ecb8…`, MOS is exactly
`d7a51147…`, and the wrapper/backend identities remain those frozen by the
profile. Card readback confirms a load-only autoexec (`SET KEYBOARD 1`, change
to `/apps/nurples-overlay`, `LOAD overlay.bin`) and byte-exact fixture plus four
RGBA2222 payloads. No emulator was agent-launched; the checkout is being held
at the Author-only GUI boundary.

## 38. Source-only bounded presentation lifecycles

Work continued below the hardware gate without changing production dispatch.
`presentation_input_poll` takes one MOS sysvar snapshot and one held-key-map
snapshot per frame, reducing them to any-key-down and normalized Y/N values.
`start_choice_controller` applies the shared release-before-fresh-press gate;
unrelated keys consume an edge but cannot become a later choice while held.
This replaces the design dependency on blocking `mos_getkey` and the busy
release loop without coupling MOS input to presentation policy.

The dormant attract lifecycle advances the blank starfield at speed one,
updates its fixed overlay and returns a bounded dismissal edge. It owns no
terrain, actor or VBlank update. The dormant GAME OVER lifecycle acquires its
overlay before mutating the world, hides the player, clears all player-laser
and fireball records, advances terrain exactly once through the original
one-scanline gameplay path, and continues seekers, explosions, active scenery
and non-firing turret search. It never dispatches the player actor and never
waits or issues VBlank. The player-owned world-travel policy now admits the
published global GAME OVER phase without rewriting the player's local dead
state; this has no effect on the still-blocking production handler until the
lifecycle is integrated.

Sixteen focused Python contract tests pass, with two expected native-fixture
skips on the Pi because its documented local assembler path is absent. Both new
fixtures were therefore assembled in an isolated Oryx temporary directory and
the directory was removed after validation. Their SHA-256 values were
`9631267fe8505fba05b83521d02ad9b7b787cbe51a3d26bae91c61ead3caf5ce`
for the start-choice fixture and
`e83dcc72199d18d56941a67c16b9c1bb6ce386ac664a1cefdaeb0fde8eac89c2`
for the MOS input boundary. The current production include graph also assembles
natively: 44,306 bytes, SHA-256
`712b69c1ba7b55fcfffe248f2e6c3f2989df7990b485b8ff261fe96ccc3edb94`.
The copied Oryx source and generated binaries were temporary validation inputs,
not a deployment.

## 39. One-time resources versus clean-session initialization

The legacy `game_init` boundary is now mechanically split without yet changing
its blocking title/configuration behavior. It remains the sole owner of the
process-entry VDP reset and font/UI/game asset loads. After those loads it calls
`game_prepare_playfield`, which selects mode 20, redraws bezel/cockpit, publishes
the canonical starfield buffer before plotting it, and owns no actor/session or
level state. It then calls `game_new_session_init`, which resets score, starting
score, lives, runtime tables, tiles and player state before publishing
`game_playing`. Neither reusable routine clears or reloads a VDP asset.

Publishing `BUF_BG_STARS_0` before `init_bg` removes a real resident-RAM
assumption: a prior level may have left `bg_bufferId` naming another background.
The split also supplies the exact future boundaries needed by attract and GAME
OVER: attract can prepare its empty playfield after the one-time load, while a
continuation can call clean-session initialization directly without returning
to `game_init`.

The three affected assembly-safety contracts pass. Native Oryx assembly of the
complete production include graph also passes; the refactored binary is 44,317
bytes with SHA-256
`087f8bd40870f765587d142915f5768ae979635adb18a2d9ea44413ab49cbf76`.
The isolated copied source and generated binary were validated and removed; no
deployment or emulator profile changed.

## 40. Frozen presentation-flow transition layer

The remaining global orchestration is now implemented off-production in
`presentation_flow_states.inc`. Resource-complete startup prepares mode 20 and
defines the ordinary 20-sprite prefix before acquiring the attract overlay.
A fresh attract dismissal tears the overlay down and enters a separate bounded
Y/N configuration state. That state takes one input snapshot per frame,
applies the selected joystick policy, redraws the canonical playfield and calls
`game_new_session_init` directly. GAME OVER uses the same configuration and
resident-session path after its own release-gated dismissal.

Neither path calls `game_init`, clears buffers, reloads assets, waits for input
or issues VBlank. A failed overlay transaction or impossible session-init
failure latches a distinct game fault and exits the invocation instead of
retrying resource loading or entering an unbounded failure loop. The module and
its lifecycle dependencies remain excluded from `nurples.asm` pending the
standalone visual verdict.

Six host contract tests cover ordering, state transitions, bounded input,
resident-session reuse and fail-closed behavior. The native transition fixture
exercises ATTRACT → CONFIG → YES → SESSION, GAME OVER → CONFIG, and overlay
acquisition failure. It assembled on Oryx as a 680-byte binary with SHA-256
`e3144b13565b867230d912542914f0998e15efd07f124e5f781d0d115f081d08`.
The isolated validation directory was removed after hashing; no emulator or
deployment state changed.

A later exit-path audit found that Escape during ATTRACT or GAME OVER would
otherwise reach `main_end` before the overlay feature transaction restored VDP
variable 2. The off-production flow layer now supplies an idempotent process
shutdown: detach/demote renderer sprites, restore an ordinary or orphaned
feature transaction, clear overlay/input ownership, and only then permit the
normal sprite reset and global buffer deletion. The revised native transition
fixture observes this ordering and assembles to 785 bytes, SHA-256
`f36143adabb62fa93258ec878f800ab1418bc614fb6b57990cf34188378f6cb8`.
The prior 680-byte fixture hash remains historical evidence for the transition
logic before the exit-restoration case was added.

The complete device-free Python regression pass now covers 188 tests with 15
expected skips. Three assembler-driven test methods were excluded because the
Pi's documented `~/.local/bin/ez80asm` is absent; the current production graph,
new input/choice fixtures and the presentation-flow transition fixture have
already been assembled separately with Oryx's native toolchain. No device-free
test failed.

## 41. Isolated full-integration assembly proof

The hardware visual gate still prevents changing the production include graph,
but the exact post-gate integration was exercised in a disposable source
mirror. That mirror enabled every real presentation module in dependency order,
added ATTRACT and CONFIG to the validated pointer whitelist and telemetry map,
replaced blocking startup with `game_attract_begin`, replaced blocking GAME
OVER with `game_over_bounded`, initialized presentation RAM at process entry,
and called presentation shutdown before the normal VDP resource teardown.

Native Oryx assembly of that complete candidate passed and emitted the expected
state/teardown symbols. The integrated binary was 45,816 bytes with SHA-256
`db19a0766351ab31a03145787d4eda7dce05d06a5fcd83c011013751bbeb645a`.
Both the local integration mirror and its Oryx copy/build were validated and
removed after hashing, so the canonical `nurples.asm` remains outside the
hardware gate and no profile or deployment changed.

This integration audit also tightened process ownership: overlay feature
process initialization is now RAM-only. It no longer clears control buffer IDs
before the application has acquired and globally reset VDP resources. Each
actual begin transaction still clears and reconstructs all three controls before
snapshotting variable 2, preserving repeated-entry determinism without touching
potential caller-owned buffers during early process setup.

## 42. Deferred math review boundary

A separate wide-ranging VDPlet code review reported a potential defect in the
production 16.8 trigonometry implementation: the report says `sin168` may
overwrite the fractional angle byte while constructing a three-byte sample
index, leaving the documented interpolation path ineffective. This campaign
does not validate that report and makes no math or consumer change in response.

The independent review is now frozen as `MATH-001` in `TODO.md`. It requires a
baseline, complete consumer inventory, deterministic numeric and ABI evidence,
historical comparison, and a reviewed repair proposal before any production
math routine changes. `GAME-001` continues against the current frozen math
behavior so the possible defect cannot silently expand the presentation-flow
scope.

## 43. Completion-gate audit before overlay review

The active campaign is not complete merely because its dormant modules compile.
Current evidence separates into the following gates:

1. **Visually qualified foundation:** the corrected player swoop and one-second
   grace period were accepted in R02. The fast-starfield seam guard has native
   OBS evidence identifying the missing scanline and R04 is byte-identical to
   the previously clean R02 renderer. The fireball direction/frame correction,
   explosion edge deactivation, retained death trail and terminal explosion
   also have explicit Author acceptance.
2. **Integrated and deterministically qualified, awaiting complete-game
   exercise:** safe mid-level respawn placement, physical turret slew/search,
   exact-pad seeker retreat, victory flight and the terrain-drain/hyperwarp
   transition are in the production source graph. Their focused host models,
   assembly contracts and native integration build pass, but that evidence does
   not substitute for the final complete-game play-through.
3. **Hardware foundation qualified:** the standalone two-hardware-sprite
   renderer, exact VDP-variable transaction and teardown pass the pinned Alexe
   R07 qualifier for absent, present-zero and present-BEEF incoming states.
   The working production source now contains the qualified integration; its
   full-game native build and Author review remain open.
4. **Post-gate integration in review:** bounded ATTRACT, CONFIG and GAME OVER lifecycles,
   fresh-press input, resident-session restart and process shutdown pass their
   source, host and isolated native integration proofs. Gate 3 has now admitted
   them to the canonical production include graph. The integrated full game
   still requires a fresh native build and Author runtime review before any
   commit.

This matrix is the completion rule for the current goal. Passing the standalone
overlay review authorizes the prepared production integration; it does not by
itself close the campaign.

## 44. Hardware-overlay crash and exact-state qualifier

The first generation-417 runtime review rendered a green/scanline-corrupted
frame and then panicked with `CORRUPT HEAP: Bad tail ... got 0xc4c4c4c4`.
Pinned VDPlet source confirmed that the scanline compositor reads active sprite
objects asynchronously. The renderer and fixture now deactivate the global
sprite engine before mutating frame vectors or hardware/software flags, then
reactivate only after the two overlay slots are complete. The same ordering is
used during teardown. R02 and every later run completed without that panic.

R02 also exposed a fixture-only stale-register defect: `vdu_clear_buffer`
destroys HL, but the PASS/FAIL program writer reused HL without reloading the
target buffer ID. The status programs were therefore written to buffer 163 and
the intended buffers stayed empty. Reloading HL after each clear made the
restore verdict visible. R03 then showed ABSENT FAIL, ZERO PASS and BEEF PASS.
R04 disproved a buffered tail-call hypothesis. R05 and R06 added temporary
diagnostics and established all of the following independently:

- variable 2 was absent before asset loading;
- it remained absent after all four overlay bitmaps were created;
- neither marker-zero nor marker-one comparison succeeded;
- the ordinary restore left variable 2 present;
- a direct top-level `F9` clear restored absence successfully.

The MOS→VDP capture contained the exact one-byte marker write, ruling out an
assembly or transport omission. Inspection of the pinned VDPlet source then
identified the defect in `VDUStreamProcessor::bufferConditional`: its 32-bit
`sourceValue` starts at `-1`, and `readBufferBytes` overwrites only one byte for
an ordinary 8-bit buffer comparison. Marker zero becomes `0xFFFFFF00`, marker
one becomes `0xFFFFFF01`, and zero is consequently treated as nonzero. This is
a firmware buffered-conditional defect, not a missing Nurples buffer.

The exact references inspected were the official
`/home/smith/Agon/agon-docs/docs/vdp/Buffered-Commands-API.md` specification and
the pinned-release `video/vdu_buffered.h`, `video/buffers.h` and
`video/vdp_variables.h` sources beneath
`/home/smith/Agon/mystuff/agon-vdplet/build/vdpl041/release-012/firmware-src`.

The final transaction avoids the defective operation and does not depend on
its future repair. Buffer 63985 begins as a seven-byte CLEAR program with two
NUL padding bytes. The true VDP-variable existence check in buffer 63988 calls
63987 only when variable 2 exists; that program patches byte 2 from `F9` to
`F8` and copies the original 16-bit value into bytes 5–6. Teardown calls 63985
unconditionally, so the same program restores either exact absence or exact
presence/value without any buffer-valued conditional. Only buffers 63985,
63987 and 63988 are now transactional controls.

The clean R07 fixture assembled to 2,665 bytes, SHA-256
`5f58d3e020102f7f1d4994f7ab92405f155b593c7b8ef4086cae823ecc0a4728`.
It used pinned VDPlet `dc43ecb8…` and Eirene MOS RC3 `d7a51147…`. Its OBS
recording is SHA-256
`66f299648bb61a139a5aa1a981b9509b8fdf8b386084bed8b6955cf5ae2da75b`.
Visual review shows PASS for ABSENT, ZERO and BEEF restoration, both title
frames and both prompt frames animate over the moving background, and the run
returns to MOS without a heap panic. The corrected capture launcher also tracks
the actual emulator binary rather than merely its shell wrapper; post-run Oryx
inspection found no owned emulator, OBS, private X server or collector process.

The generic firmware owner independently reconfirmed the same current-release
partial-copy behavior as the existing `VDPL-020` root cause. A private firmware
candidate already corrects and qualifies that behavior across 545 cases, but it
has a different feature composition and is not the pinned shared default.
Nurples therefore retains its R07-qualified application workaround; no firmware
source, flash, release or default changed during this investigation.

## 45. Qualified presentation flow enters the production graph

After R07 passed, the previously isolated integration was applied to the real
working source without changing its established state-pointer ABI:

1. The input gate, bounded presentation sampler, joystick-choice controller,
   overlay controller, exact feature transaction, hardware renderer, ATTRACT,
   GAME OVER and flow coordinator are included in dependency order.
2. Process entry initializes presentation ownership in RAM only. Process exit
   shuts it down before the ordinary sprite reset and global buffer deletion.
3. The one-time loader now enters bounded ATTRACT directly after all resident
   assets load. The old blocking start prompt and blocking joystick choice are
   no longer on the successful startup path.
4. `game_over` retains its stable pointer identity but tail-jumps to the bounded
   `game_over_bounded` lifecycle. ATTRACT and CONFIG are admitted to the exact
   dispatcher whitelist and telemetry map as TITLE phases.
5. Both ATTRACT and GAME OVER dismiss through the same release-gated CONFIG
   state, redraw the resident playfield and start a clean session without
   clearing or reloading assets.

The focused production-integration suite passes 44 tests with seven expected
assembler skips on the Pi host; the added dispatch/telemetry integration test
also passes. Ruff and the hand-authored-source `git diff --check` pass. The full
Pi discovery run now distinguishes that documented missing native assembler
from a source failure: 201 tests pass with 17 expected assembler skips and no
error. The already-qualified R07 fixture remains the hardware foundation. A
fresh native production build, stopped Alexe profile and Author full-flow
review are still required before these changes may be committed.

Oryx generation 417 was released at 2026-11-09 18:49 EST after a fresh
whole-host snapshot showed no emulator process. The queued `agon-6502` request
was notified immediately; Nurples did not retain or preempt the machine for the
pending production build.

## 46. Zero-safe pad and turret launch countdown

The explicit `ASM-010` underflow is repaired in the working candidate without
changing either actor's record schema, pool limit or established post-launch
range. Both launchers now call one generic active-tile countdown helper. A timer
already at zero reports ready without decrementing; one decrements to zero and
reports ready; larger values decrement once and continue waiting. Thus the
initial random zero retains its intended immediate-launch meaning instead of
wrapping to 255 frames.

The pad's later 64–95-frame range and the turret's later 24–55-frame range are
unchanged. Four new deterministic tests cover zero, one, ordinary nonzero
values, check-before-decrement assembly ordering, both consumers and both reset
ranges. They pass together with the 46 source-level assembly-safety tests; the
only focused skip is the unavailable Pi-native assembler. `ASM-010` remains
open until the queued native production assembly confirms the integrated code.

## 47. Separate stopped-profile production staging

`tools/stage_production_oryx.py` prepares the forthcoming production review as
a deployment-only operation. It refuses an existing destination, verifies the
pinned Flexe/VDPlet/MOS template, requires a complete nonempty regular-file game
bundle, creates a fresh FAT image, reads every payload and load-only autoexec
back by hash, verifies both runtime manifests and leaves the profile stopped.
It cannot assemble or launch the game. The autoexec selects `nurples.bin` but
does not run it.

Three host tests cover recursive source/hash completeness, font-symlink refusal
and the exact load-only autoexec. The full device-free suite now passes 201
tests with 17 expected native-assembler skips. Actual FAT-image staging remains
part of the queued Oryx operation because this Pi host lacks `mcopy`; the tool's
first real use must retain its output and readback evidence before the profile
is offered for Author review.
