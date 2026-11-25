# Designing Nurples levels on Oryx

From a terminal on the Oryx desktop, open the modern campaign map:

```sh
cd ~/Agon/mystuff/nurples-level-design
tiled level-design/levels/modern-dg/level.tmx
```

[campaign.toml](campaign.toml) lists all four levels in play order. For a small map
that shows how to place an active tile, open the [seeker nursery](../tests/maps/dg/pn_map_01_seeker_nursery.tmx)
or [fire-control range](../tests/maps/dg/pn_map_02_fire_control_range.tmx).
Start new projects from the [level design template and conventions](../level-design/README.md).
A new TMX isn't added to the campaign just by saving it.

## Art to review

| Open | What to look for |
| --- | --- |
| [Gameplay atlas](../level-design/assets/tilesets/dg/dg0.png) and [Tiled tileset](../level-design/assets/tilesets/dg/dg0.tsx) | The brushes the game maps use today. |
| [Source sprites](../assets/img/orig/sprites/) | Compare `seeker.png`, `turret.png`, `pad.png` and the `tile_pad_*` animation frames. |
| [Processed sprites](../assets/img/proc/sprites/) | Compare the prepared game artwork with the sources before picking a marker. |
| [Classic art](../tiles/nurples_classic/) and [Xevious sprite sheet](../tiles/xevious/xevious_sprites.png) | Historical and visual references, not new runtime spawn types. |

Maps use sixteen columns of 16×16 tiles. The bottom enters play first.
Earth supports any structure beneath a spawn. DG air spawners retain their
visible pad: place them in Spawns over an empty cell or their matching pad.
The compiler rejects unrelated underlying DG art rather than losing it.
Turrets, electrodes and other active ground structures belong in Structures.
