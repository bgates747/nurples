# Purple Nurples runtime telemetry ABI

Status: ABI 1.0 implemented; baseline traces pending

## Purpose and ownership

`telemetry_begin` is a stable, contiguous RAM record for deterministic tests,
debuggers, and the future autonomous qualification player. It publishes
numeric semantic state rather than writable legacy callback pointers. Purple
Nurples owns the record and its meanings; Agon Headless Test reads it through
the symbol artifact built with the exact binary under test.

The global, event, policy, player, and enemy-domain IDs are defined once in
`src/asm/state_ids.inc`. Legacy pointer-dispatched states are translated at the
telemetry boundary during migration. Bit 0 of `telemetry_flags` remains set
while that bridge is active.

Producer-only event, lifecycle, outcome, phase, and frame-accounting latches
are deliberately stored after `telemetry_end`. They are implementation state,
not part of the public ABI.

## Snapshot protocol

Resolve `telemetry_begin`, `telemetry_size`, and `telemetry_write_seq` from the
`.symbols` file shipped with the binary. All multi-byte fields are unsigned
little-endian values.

1. Read `telemetry_write_seq` and retry while it is odd.
2. Copy the number of bytes recorded at `telemetry_size`, beginning at
   `telemetry_begin`.
3. Read `telemetry_write_seq` again.
4. Accept the copy only if both sequence values are equal and even.

The sequence is an eight-bit wrapping seqlock, not a frame counter. A valid
record begins with ASCII `PNST`, has ABI version 1.0, and currently has a size
of 80 (`0x50`) bytes. Readers must validate the in-record version and size and
must not hard-code a build-specific address.

The assembled sequence byte is deliberately odd. `telemetry_init` is the first
call in `main` and publishes an even BOOT record only after rebuilding every
public field. Because repeated MOS `RUN` does not reload resident RAM, a runner
starting another invocation must still wait for that invocation's new LOADING
transition instead of accepting a prior terminal record sampled before the
entry code has executed.

`telemetry_total_frames` advances only after a completed outer-loop iteration
that reached VBlank. Startup phase publications and the pre-VBlank orderly-exit
publication do not invent scheduler frames or age a stable state. Escape is
sampled after VBlank and therefore publishes the completed final frame.

## ABI 1.0 layout

| Offset | Size | Symbol | Meaning |
| ---: | ---: | --- | --- |
| `00` | 4 | `telemetry_magic` | ASCII `PNST`. |
| `04` | 1 | `telemetry_version_major` | ABI major version, currently 1. |
| `05` | 1 | `telemetry_version_minor` | ABI minor version, currently 0. |
| `06` | 2 | `telemetry_size` | Total public record size, currently 80. |
| `08` | 1 | `telemetry_write_seq` | Odd while the producer is writing. |
| `09` | 1 | `telemetry_flags` | Validity and migration flags described below. |
| `0A` | 6 | `telemetry_header_reserved` | Zero; reserved for compatible header growth. |
| `10` | 1 | `telemetry_global_current` | Current global numeric ID. |
| `11` | 1 | `telemetry_global_previous` | Previous observed global ID. |
| `12` | 1 | `telemetry_global_pending` | Reserved coordinator target; `255` in the bridge. |
| `13` | 1 | `telemetry_global_policy` | Published `STATE_NEW_POLICY_*` bits. |
| `14` | 1 | `telemetry_dispatcher_phase` | Coordinator dispatch phase; IDLE in the bridge. |
| `15` | 1 | `telemetry_outcome` | High-level outcome described below. |
| `16` | 1 | `telemetry_player_current` | Current `STATE_NEW_PLAYER_*` ID. |
| `17` | 1 | `telemetry_player_previous` | Previous observed player ID. |
| `18` | 1 | `telemetry_player_pending` | Reserved controller target; `255` in the bridge. |
| `19` | 1 | `telemetry_player_dying_phase` | Cooperative death subphase. |
| `1A` | 1 | `telemetry_player_dying_timer` | Death-subphase frame timer. |
| `1B` | 1 | `telemetry_player_dying_spawns` | Death-subphase spawn attempts. |
| `1C` | 1 | `telemetry_enemies_current` | Current `STATE_TELEMETRY_ENEMY_*` ID. |
| `1D` | 1 | `telemetry_enemies_previous` | Previous enemy-domain ID. |
| `1E` | 1 | `telemetry_enemies_pending` | Reserved controller target; `255` in the bridge. |
| `1F` | 1 | `telemetry_last_lifecycle_events` | Most recently published lifecycle marker bits. |
| `20` | 3 | `telemetry_total_frames` | Completed outer scheduler frames since launch. |
| `23` | 3 | `telemetry_global_state_frames` | Completed frames in the current global ID. |
| `26` | 3 | `telemetry_global_transition_serial` | Incremented on every observed global-ID change. |
| `29` | 3 | `telemetry_player_state_frames` | Completed frames in the current player ID. |
| `2C` | 3 | `telemetry_player_transition_serial` | Incremented on every observed player-ID change. |
| `2F` | 3 | `telemetry_enemies_state_frames` | Completed frames in the current enemy ID. |
| `32` | 3 | `telemetry_enemies_transition_serial` | Incremented on every observed enemy-ID change. |
| `35` | 1 | `telemetry_pending_events` | Events still pending after the latest publication; zero in the bridge. |
| `36` | 1 | `telemetry_frame_events` | Events attributed to the latest publication. |
| `37` | 1 | `telemetry_last_transition_events` | Events accompanying the last global transition. |
| `38` | 1 | `telemetry_events_seen` | Process-lifetime OR of all published event bits. |
| `39` | 3 | `telemetry_lifecycle_serial` | Incremented whenever lifecycle markers are published. |
| `3C` | 2 | `telemetry_fault_domains` | Bit mask of domains with nonzero fault bytes. |
| `3E` | 1 | `telemetry_fault_game` | Legacy global-dispatch faults. |
| `3F` | 1 | `telemetry_fault_player` | Legacy player-dispatch faults. |
| `40` | 1 | `telemetry_fault_enemies` | Legacy enemy-dispatch faults. |
| `41` | 1 | `telemetry_fault_sprite` | Sprite/table faults. |
| `42` | 1 | `telemetry_fault_tile` | Active-tile/table faults. |
| `43` | 1 | `telemetry_fault_timer` | Timer faults. |
| `44` | 1 | `telemetry_fault_vdu` | VDU timeout/query faults. |
| `45` | 1 | `telemetry_fault_coordinator` | Reserved replacement-coordinator faults. |
| `46` | 1 | `telemetry_fault_player_controller` | Reserved player-controller faults. |
| `47` | 1 | `telemetry_fault_enemy_controller` | Reserved enemy-controller faults. |
| `48` | 1 | `telemetry_fault_telemetry` | Reserved telemetry-producer faults. |
| `49` | 1 | `telemetry_player_ships` | Current lives/ships value. |
| `4A` | 1 | `telemetry_player_shields` | Current shield value. |
| `4B` | 1 | `telemetry_tileset` | Current tileset index. |
| `4C` | 1 | `telemetry_level` | Current level index. |
| `4D` | 3 | `telemetry_rows_remaining` | Source tile rows not yet inserted. |

The layout is append-only within ABI major version 1. Existing offsets and
meanings must not move. A reader may consume the prefix it understands after
validating that the reported size covers that prefix.

## Flags and validity

`telemetry_flags` uses these bits:

| Bit | Meaning |
| ---: | --- |
| 0 | Legacy pointer-to-ID bridge active. |
| 1 | Session-owned scalar fields are valid. |
| 2 | Level-owned scalar fields are valid. |
| 3 | Actor state fields are valid. |

BOOT begins with all validity bits clear and actor IDs INACTIVE. Every startup
phase from BOOT through COUNTDOWN clears the three validity bits, including the
NEW_GAME bridge snapshot produced when a legacy handler schedules `game_init`.
They become set in the same snapshot that successfully observes PLAYING. A
consumer must ignore invalid domain payloads even though their bytes remain
well-formed.

## IDs, events, lifecycle, and outcomes

Global IDs 0 through 13 are BOOT, LOADING, TITLE, NEW_GAME, LEVEL_ENTER,
COUNTDOWN, PLAYING, PAUSED, LEVEL_TRANSITION, RESPAWN, GAME_OVER, VICTORY,
EXIT, and FAULT. Values 128 through 130 temporarily identify the legacy
CUTSCENE, INVENTORY, and MAP handlers. Value 255 means none or invalid.

The legacy blocking startup explicitly publishes LOADING, TITLE, NEW_GAME,
and LEVEL_ENTER as it crosses those boundaries. The first completed gameplay
snapshot publishes PLAYING. LOAD_COMPLETE accompanies the TITLE edge and START
accompanies NEW_GAME, including a key-confirmed restart after GAME_OVER.
COUNTDOWN remains absent while the marked legacy countdown bypass is active.

Player IDs are INACTIVE 0, READY 1, ACTIVE 2, DYING 3, and DEAD 4. Enemy-domain
IDs are INACTIVE 0, ACTIVE 1, DESPAWNING 2, DEAD 3, and RESPAWNING 4.

Event bits are START `$01`, LOAD_COMPLETE `$02`, PAUSE `$04`, LEVEL_COMPLETE
`$08`, PLAYER_DEAD `$10`, NO_LIVES `$20`, VICTORY `$40`, and QUIT `$80`.
Posting is observational in the legacy bridge; the replacement coordinator
will consume the same bit assignments as control input.

Lifecycle marker bit `$01` means RESPAWN and `$02` means PLAYER_DEAD. The
latest nonzero marker remains visible in `telemetry_last_lifecycle_events`,
while `telemetry_lifecycle_serial` distinguishes later occurrences of the same
marker.

Outcomes are RUNNING 0, LEVEL_TRANSITION 1, GAME_OVER 2, VICTORY 3, EXIT 4,
FAULT 5, RESPAWN 6, and PLAYER_DEAD 7. A nonzero fault domain takes precedence,
followed by an explicitly forced terminal outcome, phase-derived outcomes, and
one-publication lifecycle/event outcomes.

Fault-domain bits 0 through 6 correspond to game, player, enemies, sprite,
tile, timer, and VDU. Bits 7 through 10 are reserved for coordinator, player
controller, enemy controller, and telemetry faults as those producers are
connected.

## Binary and symbol publication

The normal and standalone map-suite builds assemble in fresh temporary source
trees with `ez80asm -s`, then publish the binary and `.symbols` artifact as one
matched pair under `tgt/`. The campaign pair is `nurples.bin` and
`nurples.symbols`; the qualification pair is `nurples-map-tests.bin` and
`nurples-map-tests.symbols`. A source-named symbol file is renamed only after
the same successful assembler invocation has produced both artifacts.

Implementation and symbol publication do not close `ASM-001A`. Its remaining
acceptance evidence is the repeatable startup, ordinary-play, level-completion,
player-death, respawn, and game-over traces named in `TODO.md`.
