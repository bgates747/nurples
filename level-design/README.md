# Nurples level design conventions

Use one Tiled project per story level. Keep map placements local and share
the artwork they reference.

    level-design/
      template/                    Starting project and map; copy, don't link
      assets/
        tilesets/tiny-city/        External TSX, atlas PNG, license and credits
        tilesets/dg/
        backgrounds/earth/
      levels/
        01/
          01-earth.tiled-project
          level.tmx                Campaign version
          staggered.tmx            Alternate encounter, same level
        02/
          02-outbound.tiled-project
          level.tmx
      experiments/                 Studies and maps without a story slot

## Starting or varying a level

1. Copy the contents of `template/level/` into a two-digit folder under
   `levels/`, named for the **1-based story
   stage**. Earth is `01`. Do not assign a story number just because a proving
   ground happens to be next in a test build.
2. Name the project for its stage and give its campaign map the name
   `level.tmx`. Keep alternate layouts in the same project with meaningful
   names. Each variant owns its placements; editing staggered spawns must not
   change the campaign map.
3. Add the playable map to the ordered campaign manifest. The manifest,
   not the directory name, determines the **zero-based runtime index**:
   its first entry is index 0. An alternate is not an extra campaign entry
   unless explicitly selected.

## Reference boundaries

- Use relative paths. A map or project may reference files in its own
  level folder and in `level-design/assets/`, never in another level folder.
- Store reusable art once. Maps reference an external TSX; the TSX references
  its atlas. Keep provenance and licenses with the shared art. Shared assets
  must not depend on anything inside a level folder.
- If another level needs something currently local, move it into shared
  assets and update both references rather than copying it or linking into
  the original level. For example, the return to Earth can reuse the airfield
  art without depending on Stage 1's map.
- The campaign manifest deliberately references levels from outside their
  folders. The template is a starting copy, not a live dependency.

## Map layers

Use exactly three layers, bottom to top:

1. **Background** — Earth's locked terrain image, or an empty tile layer for a
   runtime starfield. The empty layer costs no bitmap upload.
2. **Tiles** — visible scenery and fixed active tiles. In DG, put functional
   seeker pads, turrets, electrodes and discharges here. A seeker pad already
   owns its launches and is a retreat destination: don't also place a seeker
   marker unless you want a second, independent ship.
3. **Spawns** — place a sprite marker on any cell, even on top of a fixed
   active tile. The marker draws nothing. To show an ornamental DG pad under
   a ship, paint Tiled tile 120 on **Tiles**; to show a road, paint the road.

The compiler keeps the terrain and emits row events in left-to-right order.
If two events share a cell, the ground activates first, then the sprite.
Both use the existing fully-scrolled-in timing. Hiding either drawing pass
in a diagnostic build does not disable spawns.

## Migration and checks

Move files and repair references first; verify that compiled map bytes and
artwork are unchanged. Migrate layer conventions separately. The
`test_design_refs` check resolves references and reports both ends of a
missing or forbidden reference.
