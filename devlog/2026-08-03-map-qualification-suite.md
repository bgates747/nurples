# 2026-08-03 — Standalone map qualification suite design

Audited the current map runtime, active-tile dispatch, actor pools, current TMX
content, and dormant conversion scripts. The currently selected map is a
16-by-256 omnibus level lasting approximately 68 seconds. It mixes implemented
and no-op low tile IDs and places some actors too near level completion to
finish their lifecycles.

Designed a separate nine-map qualification suite in
`docs/map-qualification-suite.md`. Eight maps isolate baseline flight, seekers,
predictive turrets, electrical circuits, craters, death/respawn, dirty level
transition, and clean transition state. A ninth compact integration map is the
candidate for eventual campaign promotion if it plays well.

The design distinguishes actually implemented GIDs 1, 5, 9, 10, 11, and 16
from declared but null behaviors. It records bottom-to-top Tiled orientation,
row timing, run-out requirements, pool capacities, exact behavioral row
layouts, operator procedures, and a coverage matrix.

No TMX, generated include, game source, build script, binary, or campaign level
was changed. The dormant `tiles/dg/tile.py` entry point was not run because it
couples destructive tile/image regeneration with a one-map overwriter. A
future implementation requires a safe map-only multi-level compiler and a
standalone build/profile that does not replace the campaign include.

The behavior audit also found that small pads and fireball turrets accept zero
from their initial random delay setup but decrement the timer before testing
it. Zero therefore wraps to 255 rather than activating immediately. This is
recorded as ASM-010 in the authoritative TODO and was not changed during map
design. The suite uses multiple launchers and does not require every launcher
to fire immediately.

## Implementation and build qualification

Implemented all nine fixtures under `tests/maps/dg/` and added a strict,
map-only compiler in `nurples_tools/maps.py`. The compiler accepts a deliberately
small TMX subset, checks every nonzero GID against the loaded DG catalog,
reverses Tiled rows into runtime order, and emits the ordered multi-level
include `src/asm/levels_test_maps.inc`. Unit tests pin the suite's map order,
dimensions, and every active-tile row from the operator cards.

An independent integration review caught that a purely numeric manifest order
would send Clean Room into Orbital Shipyard instead of wrapping to Flight
Deck. Runtime order is therefore 00 through 05, then 08, 06, 07. The
`--start-map` option resolves the requested PN-MAP ID rather than treating it
as a manifest offset, preserving intuitive independent selection.

`build/scripts/build_map_tests.py` assembles the real game through a temporary
copy of `src/asm`. It overlays the generated suite only inside that copy and
writes the separate `tgt/nurples-map-tests.bin`. The first implementation
passed an absolute output filename to `ez80asm`; the assembler rejected that
long path. The wrapper now assembles a short local filename, verifies the
staged result, and copies it to `tgt` only after successful assembly.

Final hardening restricts the CLI writer to the dedicated test include,
enforces the runtime's actual DG base-buffer and tile-zero assumptions, and
publishes the verified test binary atomically through a same-directory
temporary file.

The complete production build was run before the standalone build. Its
accepted outputs remained byte-for-byte unchanged:

- campaign include: `68fedba2fa2906711a8a22c0f66fd234503c7a36016c1c44a0b5798635da8b17`
- `game.agnb`: `b5e27f3f36ee9f25fe69fb35b3294c1268af1610f53327386fac6ae0055af823`
- `ui.agnb`: `d16b74171130fd72eff4dffd8485408aee1a1d356ce6df6e00bd9bb5742a0780`
- campaign binary: `b4c362aa8280d87eca182f1a90db1751926882935b090c0d583aeb1fe433261a`

The generated suite include hashes to
`c63dc6cc2a67b8ff115dffe1b517fdaae7332f83ec29a0ad1c16be197bb03fba`.
The default PN-MAP-00-first binary hashes to
`f5846977f1d403e3c800a9fdd6f12a57bd9fc6bda3131884e8c3f640584d719c`.
An independent PN-MAP-08-first build produced a different hash and the default
order then reproduced its original hash, exercising deterministic selection.

Verification passed with Python 3.14.6: `pip check`, the canonical native
`agonutils` test, all 68 project unit tests, Ruff, map drift checking, and both
production and standalone assembly.

The Author then ran the PN-MAP-00-first binary through the isolated graphical
profile and reported that it appeared to be working. This accepts the initial
emulator smoke qualification and the standalone suite as a development
fixture. It does not claim that every operator-card behavior or the broader
ASM-007 stabilization matrix has been exhaustively qualified.
