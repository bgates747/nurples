# Designing Nurples levels on Oryx

From a terminal on the Oryx desktop, open the modern campaign map:

```sh
cd ~/Agon/mystuff/nurples-level-design
tiled level-design/levels/modern-dg/level.tmx
```

The other campaign map is [the recovered Z80 level](../level-design/levels/recovered-z80/level.tmx).
[campaign.toml](campaign.toml) lists the two in play order. For a small map
that shows how to place an active tile, open the [seeker nursery](../tests/maps/dg/pn_map_01_seeker_nursery.tmx)
or [fire-control range](../tests/maps/dg/pn_map_02_fire_control_range.tmx).
Use **Save As** beside the original TMX to start a sketch without modifying
the campaign. A new TMX isn't added to the campaign just by saving it.

## Art to review

| Open | What to look for |
| --- | --- |
| [Gameplay atlas](../level-design/assets/tilesets/dg/dg0.png) and [Tiled tileset](../level-design/assets/tilesets/dg/dg0.tsx) | The brushes the game maps use today. |
| [Source sprites](../assets/img/orig/sprites/) | Compare `seeker.png`, `turret.png`, `pad.png` and the `tile_pad_*` animation frames. |
| [Processed sprites](../assets/img/proc/sprites/) | Compare the prepared game artwork with the sources before picking a marker. |
| [Classic art](../tiles/nurples_classic/) and [Xevious sprite sheet](../tiles/xevious/xevious_sprites.png) | Historical and visual references, not new runtime spawn types. |

For maps that must build today, keep the existing format: 16 columns of
16×16 tiles, one CSV tile layer, one external `dg0.tsx` tileset. The bottom
of the Tiled map enters play first. Enemy-sprite brushes that mean “spawn
this type here, but draw the usual launch pad” are a design proposal. There
is no such compiler mapping yet; don't add a second layer or tileset and
expect the current build to accept it.
