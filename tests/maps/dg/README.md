# Purple Nurples map qualification fixtures

These nine TMX files are standalone developer fixtures for the implemented gameplay systems. They use the repository's DG external tileset at `tiles/dg/dg0.tsx`; no tileset or image is vendored here.

Runtime row `r0` is the bottom row in Tiled. Increasing runtime rows move upward through the document. The map sources contain one orthogonal 16-column CSV layer, no flip flags, and only GIDs present in the current DG asset manifest. Active placements match `docs/map-qualification-suite.md`; all other nonzero cells use loaded passive GIDs of 17 or greater.

The execution order in `suite.toml` is PN-MAP-00 through PN-MAP-05, then
PN-MAP-08, PN-MAP-06, and PN-MAP-07. Putting the dirty/clean pair last ensures
that Clean Room wraps directly to Flight Deck. Launch every other isolation
map from freshly initialized game state.
