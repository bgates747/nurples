# Actor-record schema and lifecycle audit

## Executive summary

This audit is pinned to `dev` commit `359d9b5` (`checkpoint: pin pre-audit
game and UART work`). It reviews the current sprite and active-tile framework
without changing game code.

The framework should be repaired and extended, not replaced. Its central idea
is sound: fixed 64-byte records, stable physical identities, actor-specific
19-byte defaults, generic allocation, and fail-closed validation immediately
before indirect behavior dispatch. The recent bounds and callback checks are a
strong base for the planned actor-local state machines.

The principal weakness is not the allocator. It is ownership at lifecycle
boundaries. Bytes 0 through 40 have a shared layout, but bytes 41 through 63
have no declared owner, name, initialization rule, or transformation rule.
Deactivation deliberately leaves records intact, and every allocator reuses
them. This is harmless today only because no current actor reads the tail. It
will become an immediate stale-state bug as soon as seeker origins, generation
tokens, grace periods, turret search state, or other planned local state is
put there without a formal schema and initializer.

The other important boundary is behavior transition. Dispatch itself is
validated, but `change_sprite_group_move_program` and the seeker orbit setup
write callback addresses directly and do not establish the target behavior's
state. The current death hook works by accident because the orbit routine is
also its own one-frame initializer. New work must use actor-owned transition
routines that validate the requested state, initialize all state owned by it,
and publish the new behavior only after initialization is complete.

Two current gameplay defects are confirmed while following these paths:

1. `move_explosion` tests `A` after `move_sprite`, although a top or bottom
   collision calls `neg_hlu`, which destroys `A`. The record is clamped and its
   Y velocity reversed, so explosion sprites bounce instead of leaving the
   screen. Other movers correctly reload `sprite_screen_edge`.
2. A new fireball retains default frame zero for its first eight updates. Its
   first heading-derived frame is not selected until the animation timer
   expires. This independently explains the conspicuous wrong initial artwork,
   regardless of the separate question of whether artwork heading should be
   derived from `Bearing_t` or the final velocity vector.

No broad rewrite is recommended. The staged remediation in section 11 first
freezes the schema and tests, then repairs construction and transition
boundaries, and only then adds the new gameplay states.

## 1. Scope and evidence

The audit covers:

- the sprite record and its five physical pools;
- the active-tile record, physical table, and dense stack;
- all current actor default blocks and their spawn paths;
- record initialization, reuse, transformation, behavior changes, dispatch,
  and deactivation;
- carry, index-register, and published-pointer contracts; and
- existing static tests in `tests/test_asm_safety.py`.

The relevant implementation is primarily in:

- `src/asm/sprites.inc`;
- `src/asm/tile_table.inc`;
- `src/asm/tiles_active.inc`;
- `src/asm/tables.inc`; and
- each actor's own include (`player_state.inc`, `player_laser.inc`,
  `enemy_seeker.inc`, `enemy_fireball.inc`, `explosion.inc`, and the five
  active-tile includes).

This document distinguishes current defects from hazards that are dormant only
because the custom tail is not yet consumed.

Audit-time verification on 2026-11-09 ran `tests.test_asm_safety`: 39 tests
passed. The remaining assembly smoke test did not run because the configured
assembler path `/home/smith/.local/bin/ez80asm` is absent on this host; Python
raised `FileNotFoundError` before any project code was assembled or executed.
That is an environment limitation, not a test assertion failure. No emulator
or hardware run was requested or performed for this read-only audit.

## 2. Canonical 64-byte record schema

Sprite and active-tile records deliberately share the same physical layout.
The two sets of symbolic names are aliases for the same offsets.

| Bytes | Width | Shared meaning | Ownership and lifecycle |
|---:|---:|---|---|
| 0 | 1 | Stable physical record ID | Table initializer; allocator repairs tile ID before publication |
| 1-3 | 3 | X position, 16.8 | Constructor, then movement behavior |
| 4-6 | 3 | Y position, 16.8 | Constructor, then movement behavior |
| 7-9 | 3 | X velocity, 16.8 | Constructor clears; actor initializer/behavior owns |
| 10-12 | 3 | Y velocity, 16.8 | Sprite constructor clears; tile constructor sets one pixel/frame |
| 13-15 | 3 | Scalar velocity, 16.8 | Constructor clears; actor initializer/behavior owns |
| 16-18 | 3 | Heading, degrees/256 | Constructor clears; actor behavior owns |
| 19-21 | 3 | Orientation/scratch | Constructor clears; currently overloaded by seeker orbit |
| 22 | 1 | Type plus transient bit 7 | Defaults own low 7 bits; constructor/traversal own bit 7 |
| 23-25 | 3 | Base VDP buffer ID | Actor defaults; behavior may change it |
| 26-28 | 3 | Behavior callback | Actor defaults or reviewed transition routine |
| 29 | 1 | Collision/lifecycle flags | Defaults plus generic lifecycle routines |
| 30 | 1 | Width | Actor defaults |
| 31 | 1 | Height | Actor defaults |
| 32 | 1 | Orientation count | Actor defaults |
| 33 | 1 | Animation count | Actor defaults |
| 34 | 1 | Animation index/count | Actor defaults, then actor behavior |
| 35 | 1 | Animation timer | Actor defaults, then actor behavior |
| 36 | 1 | Movement timer | Actor defaults, then actor behavior |
| 37 | 1 | Movement step/scratch | Actor defaults, then actor behavior |
| 38 | 1 | Points | Actor defaults |
| 39 | 1 | Shield damage | Actor defaults |
| 40 | 1 | Selected frame | Actor defaults, then actor behavior |
| 41-63 | 23 | Actor-specific tail | **Unspecified and never cleared on reuse** |

The 19-byte definition block is exactly bytes 22 through 40. All ten current
`*_defaults` blocks are exactly 19 bytes, and every generic constructor copies
exactly that span. This is correct; expanding the copy to the end of the
record would revive the earlier out-of-bounds-default read.

The tail must remain outside the common defaults block. Each actor header
should instead declare its own tail symbols and provide an initializer that
writes every declared byte on every fresh spawn and relevant transformation.

## 3. Physical pool inventory

### 3.1 Sprite table

The sprite table has 20 statically typed physical records:

| IDs | Count | Pool | Default behavior |
|---:|---:|---|---|
| 0-3 | 4 | Player lasers | `move_player_laser` |
| 4 | 1 | Player | `do_player` (dispatched separately) |
| 5-7 | 3 | Seekers | `en_nav_seek_zag` |
| 8-11 | 4 | Enemy fireballs | `move_enemy_fireball` |
| 12-19 | 8 | Explosions | `move_explosion` |

The pool/type/callback combination is checked immediately before indirect
dispatch. The player slot is intentionally excluded because `do_player` is
entered through the player controller rather than `move_sprite_group`.

### 3.2 Active-tile table

The active-tile table contains 256 physical 64-byte records. A dense stack of
24-bit pointers determines which records are active and in what traversal
order. The byte-sized active count deliberately caps the live set at 255.

Five current tile types are admitted by callback validation:

| Type | Constructor/transform | Behavior |
|---|---|---|
| Small launch pad | Fresh map activation | `move_pad_small` |
| Fireball turret | Fresh map activation | `move_turret_fireball` |
| Electrode | Fresh map activation | `move_electrode` |
| Lightning | Fresh map activation | `move_lightning` |
| Crater | Fresh activation or in-place transform | `move_crater` |

The dense stack and stable physical records are a useful design. They allow
cheap iteration and preserve stable IDs while records move within traversal
order. The price is a stricter deactivation context, described below.

## 4. Lifecycle model as implemented

### 4.1 Process/table initialization

`table_init` and `tile_table_init` clear their complete physical tables and
restore stable IDs. Tile initialization also clears the dense stack, count,
cursor, and allocation hint. This is the only lifecycle operation that clears
bytes 41 through 63 today.

### 4.2 Allocation

Sprite allocation scans only the caller-supplied static pool (`IX` base, `B`
count) and returns the first record whose alive bit is clear. It does not
publish a separate list entry.

Tile allocation searches all 256 physical records from a rotating hint. On
success it repairs the stable ID, increments the active count, and publishes
the record pointer at the top of the dense stack before the constructor copies
defaults. Execution is single-threaded and non-interrupting at this boundary,
so no current observer can see the partially constructed record. This ordering
must be revisited if construction ever becomes interruptible.

Both allocators return carry set and a valid `IX` on success. On exhaustion
they clear carry and explicitly declare `IX` invalid. All current call sites
that subsequently use `IX` test carry first.

### 4.3 Construction

There are five closely related generic constructors:

1. sprite at explicit coordinates;
2. sprite at another sprite's position;
3. sprite at an active tile's position;
4. active tile at map coordinates; and
5. active tile at a sprite's position.

They copy bytes 22 through 40 from a 19-byte defaults block, establish
position, initialize common motion fields, set type bit 7 (`just spawned`),
set the alive collision bit, and return carry set.

They do **not** clear or initialize bytes 41 through 63. The source-copy
variants obtain their source from the global `table_pointer` or
`tile_table_pointer`; they do not validate that pointer locally.

Actor-specific setup follows construction. Examples include seeker speed,
laser muzzle offset and velocity, fireball targeting vectors, and randomized
pad/turret timers.

### 4.4 First-update publication

Type bit 7 is a transient construction tag. Sprite traversal skips the actor
for one update, then clears the bit, moves the VDP sprite to its initial
position, and shows it. Tile traversal skips the first update and clears the
bit in place.

During that interval the byte at offset 22 is not a valid unmasked actor type.
Any new code that examines a just-spawned record must either respect the
lifecycle barrier or mask bit 7.

### 4.5 Dispatch

Both traversal loops validate the physical record before reading record data,
then validate the callback against an exact type/pool whitelist before
`CALL_HL`. Invalid callbacks fail closed: a diagnostic bit is latched and the
record is deactivated rather than called.

This is an important safety boundary and should remain. The current central
whitelists are manual manifests; each new actor or legal behavior requires a
coordinated update.

### 4.6 Behavior transition

There is no general validated behavior-transition API. Current mechanisms are:

- `change_sprite_group_move_program`, which writes the same raw callback to
  every live record in a caller-supplied range and forcibly clears the
  just-spawned tag; and
- direct actor writes, notably the seeker orbit initializer replacing its own
  callback with `move_enemy_seeker_orbit_stage1`.

Neither mechanism validates the target before publication or guarantees that
the target behavior's owned fields have been initialized. Dispatch validation
catches an illegal callback on the next traversal, but it cannot detect a
legal callback paired with stale or semantically wrong state.

### 4.7 In-place tile transformation

`spawn_active_tile_from_tile` is a transform, not a spawn. It copies only
bytes 22 through 40 over an existing record. ID, position, velocities,
heading, orientation, and bytes 41 through 63 are preserved. This is currently
used to turn pads, turrets, and electrodes into craters.

Preserving position and scroll velocity is intentional. Preserving all other
state is merely implicit. Once actor-specific tail fields exist, a transform
must declare which source fields survive and initialize every destination-owned
field; otherwise a crater can inherit pad/turret state.

### 4.8 Deactivation and reuse

Sprite deactivation validates the record, hides its VDP sprite, and clears the
alive and just-died collision bits. Tile deactivation additionally requires
that the global dense-stack cursor identify the same record as `IY`; it clears
the lifecycle bits and compacts the dense stack.

Neither operation clears the record. This is a reasonable fast release policy
provided construction owns complete initialization before reuse. Today that
contract is true for bytes 1 through 40 and false for bytes 41 through 63.

## 5. Register, carry, and pointer contracts

| Boundary | Required input | Success | Failure/return risk |
|---|---|---|---|
| `table_get_next_id` | `IX` pool base, `B` nonzero pool count | carry, valid `IX`, ID in `A` | carry clear; `IX` invalid |
| `tile_table_get_next_id` | allocator globals initialized | carry, valid `IX`, dense entry published | carry clear; `IX` invalid |
| sprite constructors | valid defaults; pool; optional published source pointer | carry, constructed `IX` | carry clear; no `IX` use allowed |
| tile constructors | valid defaults; optional published source pointer | carry, constructed `IX` | carry clear; no `IX` use allowed |
| `move_sprite_group` | aligned pool `IY`, correct `B` | callbacks receive current record through published `table_pointer`; traversal restores `IY` | bad record ends group; bad callback deactivates record |
| `move_tiles` | valid dense stack | publishes both stack cursor and physical record | corrupt entry is quarantined; record may be leaked alive rather than guessed |
| `table_deactivate_sprite` | physical sprite record in `IY` | hidden and reusable | invalid record latches fault |
| `table_deactivate_tile` | physical tile in `IY`; matching global stack cursor | record reusable; dense stack compacted | mismatch latches fault and leaves record active |
| source-copy constructors | global source pointer already published and valid | copy source position | no local source validation |

The global published pointers are traversal context, not durable actor
references. They must not be stored as long-lived destinations. A seeker that
remembers a launch pad needs a stable ID plus a generation/token that can be
revalidated, not a bare physical pointer whose record may have been reused.

## 6. Per-actor initialization proof

The following table records the current construction result. “Complete” means
bytes 1 through 40 reach a defined value before the actor becomes dispatchable;
it does not claim anything about the undefined tail.

| Actor | Path and actor-owned setup | Exhaustion behavior | Result |
|---|---|---|---|
| Player | Explicit-coordinate constructor; player init then sets lifecycle/resources | Returns before using invalid `IX` | Complete 1-40 |
| Player laser | Copy from known player; muzzle offset and upward velocity | Returns before `IX` use | Complete 1-40 |
| Seeker | Copy from current tile; scalar speed | Returns before `IX` use | Complete 1-40 |
| Fireball | Copy from current tile; heading and final X/Y targeting velocities; one-frame position advance | Returns before `IX` use | Complete 1-40; initial frame defect |
| Explosion from sprite | Copy from current sprite; downward velocity; parent released | On exhaustion parent is still released using caller's valid `IY` | Complete 1-40 |
| Explosion from player | Copy from known player | Returns carry; failure hides/releases player safely | Complete 1-40 |
| Explosion from tile | Copy from current tile; downward velocity; parent released | On exhaustion parent tile is still released in valid traversal context | Complete 1-40 |
| Small pad | Fresh tile; randomized timer | Returns before `IX` use | Complete 1-40; zero may underflow on first update (`ASM-010`) |
| Fireball turret | Fresh tile; randomized timer | Returns before `IX` use | Complete 1-40; zero may underflow on first update (`ASM-010`) |
| Electrode | Fresh tile; no extra setup | Carry is ignored, but `IX` is never read after failure | Complete 1-40 or silent no-spawn |
| Lightning | Fresh tile; no extra setup | Returns before `IX` use | Complete 1-40 |
| Crater | Fresh tile or in-place definition transform | Fresh path propagates carry; transform cannot exhaust | Complete 1-40; transform preservation policy incomplete |

Constructors trust the callback in each static defaults block, but no callback
is executed before traversal validates it. The unsafe cases are therefore
legal behavior pointers combined with uninitialized behavior-specific state,
and future use of the stale tail.

## 7. Findings

### AR-001 — High: the custom tail has no schema or reuse contract

Bytes 41 through 63 are unnamed and cleared only by whole-table
initialization. Deactivation retains them and all allocators reuse them.

**Current impact:** none observed because no current actor reads the tail.

**Near-term impact:** every planned per-actor mini state machine would be
nondeterministic on record reuse unless its complete tail is initialized.

**Required disposition:** reserve a small common lifecycle prefix if needed
(for example generation and local state ID), then declare every remaining
actor-specific field in that actor's own include with span assertions and a
complete initializer.

### AR-002 — High: behavior publication is not a state transition

Raw callback writes do not validate the requested target and do not initialize
the target behavior. `kill_player` currently broadcasts the orbit callback to
seekers, and the orbit routine repairs its own state on the next update. This
does not scale to return-to-pad, player dying, respawn grace, turret search, or
game-over behavior.

**Required disposition:** actor-owned transition entry points accept a numeric
local state or a fixed legal target, initialize all state for that target, and
publish its validated behavior last. Global lifecycle code publishes policy;
it does not rewrite individual actor callbacks.

### AR-003 — High, current defect: explosion edge exit reads clobbered `A`

`move_explosion` branches on `A` after `move_sprite`. Top/bottom collision
handling calls `neg_hlu`, whose documented contract destroys `A`; the generic
mover also reverses velocity. This confirms the reported explosion bounce.

**Required disposition:** reload and test `sprite_screen_edge`, as the laser
and fireball movers already do, then apply explosion-specific exit policy.

### AR-004 — Medium: source-copy constructors trust global pointers

Cross-domain constructors dereference `table_pointer` or
`tile_table_pointer` without a local range/alignment/identity check. All
current callers establish these pointers through a validated traversal or a
known static player record, so this is not presently an observed corrupt read.

**Required disposition:** validate the source before allocation/copy, or make
the validated source pointer an explicit input with a documented register
contract. Failure must occur before publishing a destination record.

### AR-005 — Medium: tile transformation has no preserve/reset declaration

The crater transform intentionally preserves position and scroll motion but
also preserves orientation and the entire custom tail. That accidental policy
will leak source-actor state into the destination actor.

**Required disposition:** define a transformation contract: preserve ID,
position, and selected world-motion fields; copy destination defaults;
initialize the destination tail; clear any source-only lifecycle data.

### AR-006 — Medium: tile release is valid only inside cursor context

Tile deactivation requires `tile_stack_pointer` to name the exact dense entry
for `IY`. This is well defended and necessary for compaction, but it is not a
general “release this tile record” operation. The electrode teardown's lengthy
cursor rediscovery demonstrates the burden.

**Required disposition:** retain the efficient cursor-context primitive and
name/document it as such. If actor-owned code needs deferred release, add a
validated “find stable record and release” wrapper rather than weakening the
cursor equality check.

### AR-007 — Medium: pool bounds are caller authority

Sprite allocation and group behavior replacement trust caller-supplied pool
base/count. All current call sites use compile-time pool constants, but a bad
future caller can scan or write across adjacent actor pools before dispatch
validation has a chance to intervene.

**Required disposition:** expose actor-specific allocation wrappers or a
validated pool descriptor. Do not put arbitrary base/count construction in new
gameplay code.

### AR-008 — Medium, current defect: fireball initial frame is not initialized

Fireball defaults select frame zero. Spawn writes the computed heading and
velocity but does not call `sprite_frame_heading_animation`; that happens only
after the eight-update animation timer expires.

**Required disposition:** calculate and select the heading-derived initial
frame during successful fireball initialization. Separately decide whether
the visual heading comes from `Bearing_t` or is recomputed from the final
`Vp_x`/`Vp_y` velocity. Fixing the timer defect does not settle that choice.

### AR-009 — Low: five constructors duplicate common initialization

The constructors currently agree on copy bounds and motion reset, and tests
count those writes. Duplication nevertheless makes future shared fields easy
to omit from one cross-domain path.

**Required disposition:** after sentinel tests exist, factor a small internal
“initialize common destination” routine or macro while preserving the current
public spawn entry points and carry contracts. This is not permission for a
stylistic rewrite.

### AR-010 — Low: shared offsets use cross-domain names

Tile code sometimes uses `sprite_alive`, `sprite_type`, or
`sprite_animation` aliases. The numeric offsets are currently identical, so
this is not a machine bug. It obscures ownership and makes future divergence
dangerous.

**Required disposition:** define neutral shared flag constants and use the
domain-specific field symbol in actor code when that code is otherwise being
changed for a reviewed fix.

### AR-011 — Low: the transient type tag is an undocumented schema exception

Bit 7 of the type byte means “just spawned,” temporarily making the raw type
different from every declared type value. Current traversals handle it before
validation. New diagnostics, selectors, or transition code could easily
compare the unmasked byte.

**Required disposition:** document the tag in the common schema and provide a
mask/helper for any code allowed to inspect records before first publication.
Do not relocate it merely for style during stabilization.

## 8. Required per-type header pattern

Each actor include should contain, in this order:

1. pool count and pool identity (or the active-tile type identity);
2. its exact 19-byte shared defaults block;
3. named tail fields expressed as offsets from a common `actor_custom` value
   of 41;
4. an exclusive end offset and an assembler assertion that it is at most 64;
5. local state IDs and the fixed legal transition table or selector;
6. a fresh-spawn initializer that writes every declared tail byte;
7. behavior-transition entry points that initialize target-owned fields before
   publishing the behavior/state; and
8. explicit transform and release policy where the actor supports either.

Illustrative shape (names only, not a frozen implementation):

```asm
seeker_custom:              equ 41
seeker_state:               equ seeker_custom       ; 1 byte
seeker_origin_id:           equ seeker_state+1      ; 1 byte
seeker_origin_generation:   equ seeker_origin_id+1  ; 1 byte
seeker_retreat_side:        equ seeker_origin_generation+1
seeker_custom_end:          equ seeker_retreat_side+1
; fail assembly unless seeker_custom_end <= table_record_size
```

The origin generation is essential. A stable physical tile ID alone cannot
distinguish the original launch pad from a later actor reusing that tile
record.

Unused bytes remain reserved, not scratch. Existing `move_timer`, `move_step`,
`orientation`, and similar shared fields may keep their current documented
legacy meanings until a reviewed migration moves them into the actor schema.

## 9. Initializer and transition contracts

### 9.1 Fresh spawn

1. Validate source and pool descriptor.
2. Allocate a destination.
3. Copy the exact 19-byte defaults block.
4. Initialize position and every common motion field.
5. Clear the full custom tail or call an actor initializer that writes its full
   declared span; clearing first is safer for reserved bytes.
6. Run actor-specific initialization.
7. Publish alive/just-spawned last.
8. Return carry set with valid `IX`; otherwise return carry clear without a
   visible live record.

If source validation occurs after allocation for code-size reasons, every
failure path must roll the allocation back before returning. Validating first
is simpler.

### 9.2 Behavior transition

1. Reconcile the actor with the published global lifecycle.
2. Validate the requested local state against the actor's fixed state table.
3. Initialize every field owned by the target state.
4. Clear source-state fields that must not survive.
5. Publish numeric local state and/or callback last.
6. Do not run the new update body in the same frame unless the transition
   explicitly promises that behavior.

### 9.3 In-place transform

1. Validate the physical record and its dense-stack context.
2. Preserve only the fields named by the transform contract.
3. Apply destination defaults.
4. Initialize the destination custom tail completely.
5. Restore preserved world fields.
6. Publish the destination type/behavior after initialization.

### 9.4 Deactivation

Deactivation may remain a fast logical release rather than a full clear, but
constructors must then own complete initialization. In debug fixtures, filling
released tails with a sentinel is useful for proving that no actor consumes
stale state.

## 10. Machine-check plan

Existing static checks already prove:

- every current defaults block is exactly 19 bytes;
- all constructors copy only the definitions span;
- common reused motion fields are reset;
- sprite and tile pointers are range/alignment/identity checked before
  dispatch;
- behavior callbacks are checked against fixed whitelists; and
- tile-stack deactivation and quarantine are bounded.

`ASM-011` still needs the following checks before gameplay states use the
tail:

1. assembler span assertions for the common header and every actor tail;
2. a manifest test that every actor defaults block, initializer, legal
   behavior, and pool/type appears exactly once;
3. a fresh-allocation sentinel fixture proving every readable field is
   initialized before first dispatch;
4. a reuse fixture that poisons bytes 1 through 63, releases the record,
   respawns each actor, and proves no poisoned byte owned by that actor is
   observed;
5. pool-exhaustion cases for every spawn wrapper, proving carry clear and no
   adjacent pool write;
6. sprite-to-sprite, tile-to-sprite, and sprite-to-tile source validation
   cases, including malformed global pointer inputs;
7. in-place pad/turret/electrode-to-crater transforms with poisoned source
   tails, proving only the declared preserve set survives;
8. legal and illegal behavior transitions proving initialization-before-
   publication and fail-closed rejection; and
9. generation-token reuse proving a seeker cannot dock with an unrelated tile
   that reused its former pad's physical record.

The headless fixture should inspect RAM records directly. VDP calls can be
stubbed because these tests qualify record ownership, not rendering.

## 11. Separately reviewable remediation sequence

### Stage R1 — Freeze schema and add sentinels

- Define `actor_custom = 41` and common end/span assertions.
- Add per-type tail declarations, initially empty for actors that need none.
- Add the allocation/reuse/exhaustion sentinel fixture without changing
  gameplay.

**Gate:** no `GAME-001` actor-tail work begins before R1 passes.

### Stage R2 — Centralize safe construction boundaries

- Validate cross-domain source pointers.
- Clear/initialize the tail on every fresh construction path.
- Add pool descriptors or actor-owned spawn wrappers.
- Preserve current public entry points and carry semantics.

**Gate:** required before `GAME-001B`, `GAME-001I`, and `GAME-001J` add local
state.

### Stage R3 — Introduce actor-owned behavior transitions

- Replace the global seeker callback broadcast with lifecycle reconciliation
  inside the seeker controller.
- Give seekers an explicit local-state initializer/transition API.
- Apply the same contract to player and turret states as they are migrated.
- Keep callback dispatch validation at the final call boundary.

**Gate:** required for `ASM-001I`, `ASM-001J`, and the new death/respawn/game-
over behavior.

### Stage R4 — Make tile transforms explicit

- Specify the crater preserve set.
- Initialize the crater tail and clear source-only state.
- Add poisoned-tail transform tests.

**Gate:** required before turrets or pads own custom tail data.

### Stage R5 — Repair confirmed local defects

- Make explosions read `sprite_screen_edge` and exit rather than bounce.
- Initialize the fireball's heading-derived frame at spawn.
- Resolve the fireball visual-heading source through visual qualification.
- Complete `ASM-010` so randomized zero delays do not wrap to 255.

These are small, independently reviewable gameplay fixes, not prerequisites
for documenting the schema but best completed before new presentation work
adds more states around them.

### Stage R6 — Migrate one actor at a time

Recommended order:

1. seeker return/orbit/exit states;
2. player dying, dead, spawning, and grace states;
3. turret tracking/search states;
4. fireball and explosion presentation fixes; and
5. later enemy classes.

Each actor must pass fresh, reuse, exhaustion, transition, and lifecycle-
reconciliation tests before the next actor establishes a second pattern.

## 12. Decision summary

1. Keep the common 64-byte record and generic allocation framework.
2. Freeze bytes 0 through 40 as the shared header for stabilization.
3. Treat bytes 41 through 63 as typed actor-owned storage, never anonymous
   scratch.
4. Preserve exact 19-byte default copies.
5. Make full actor-tail initialization mandatory on every reuse.
6. Replace raw behavior-pointer writes with validated actor-owned transitions.
7. Retain fail-closed validation immediately before indirect dispatch.
8. Keep dense active-tile traversal, but distinguish cursor-context release
   from a future general release wrapper.
9. Use stable ID plus generation for durable cross-actor references.
10. Repair the explosion edge and fireball initial-frame bugs as separate,
    visually qualified changes.

This supplies the architectural contract needed to begin the actor-state work
without turning stabilization into the deferred whole-codebase stylistic
rewrite.

## 13. Implemented remediation and acceptance

The first remediation pass was implemented on `dev` immediately after the
audit and kept within the framework described above:

- the shared header now reserves a nonzero generation byte and a cleared,
  actor-owned custom tail;
- every current actor include declares and bounds its custom schema;
- fresh construction validates cross-domain source records and exact sprite
  pool descriptors before allocation, begins a new logical lifetime and clears
  the tail;
- in-place crater transformation begins a new logical generation while
  retaining only its documented world-motion context;
- seeker orbit entry initializes its local fields before publishing the legal
  callback, replacing the raw group callback writer;
- explosion movement reloads the published edge result and deactivates at every
  playfield boundary; and
- fireballs select their directional animation frame immediately and derive
  their visual heading from `Heading_p`, the same lead-adjusted heading used to
  produce `Vp_x` and `Vp_y`.

The static suite passed 43 runnable assertions on the Raspberry Pi host; its
assembly smoke case could not execute there because that host has no eZ80
assembler. The identical tree assembled successfully with `ez80asm` on Oryx.
The Author then accepted the actor-lifecycle build in Alexe and used a separate
fixture with seekers disabled, fireball/player collision disabled and a
four-times-faster reload cadence to isolate projectile presentation. The
stationary-target case was correct before the final source change; the moving
lead-shot case visibly passed after changing the artwork source from
`Bearing_t` to `Heading_p`. The accelerated reload and harmless-collision
changes were fixture-only and are not part of the production source.

The deeper RAM poison/reuse and generation-token association cases in section
10 remain requirements for the first actors that consume those new fields;
this pass does not claim that the future seeker-to-pad protocol has already
been implemented.
