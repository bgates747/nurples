# EARTH-001 — Earth City opening level

Open the campaign on Earth, defending the city before following the attackers to their worlds. This silo owns the Earth level, from its editable map to Smith's first in-motion review.

## Design history

The Xevious source is 1024×2048. The first two columns were cut into sixteen 256×256 strips, bottom to top in each column, left column first. In Tiled, strip 00 is at the foot of the 16×256-cell level. The assembled background is a locked image layer. The editable Structures layer uses Kenney's CC0 Tiny City pack: 132 original 16×16 tiles repacked sixteen across, converted to Agon64 by nearest RGB without dithering. IDs 0–15 are transparent and reserved for spawn markers. The 135 existing structure placements were shifted with their art and the rendered map checked pixel-for-pixel. Clockwise 90/180/270 variants of IDs 81, 92 and 93 occupy IDs 148–156. Rotations are baked; runtime tiles have no transform flags. Separate Civ terrain studies and picker are documented in MAP-003; they are not another runtime layer.

## Approved integration tranche (Nurples)

- EARTH-001-001 [ ] Earth City first, then dg0_00, dg0_03 and the Z80 level with their existing music and appearance. Render Earth terrain 00 through 15 in flight order with the authored structures.

- EARTH-001-002 [ ] Earth initial ship at Tiled cell (6,253); use ZXArt tune 79469 privately, record provenance; retain stock MOS/VDP compatibility.

- EARTH-001-003 [ ] Verify the campaign, build and stage ordinary nurples.bin for an Oryx FAE review; leave GUI launch to Smith. No production asset redistribution or game implementation commit pending review. Rollback: game/assets at 7bc87f5 and unchanged editable Tiled sources.
