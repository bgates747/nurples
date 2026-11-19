# ART-001 — Full-frame Mode 20 vector splash review

The current 512×384 splash uses Bayer dithering, which Smith did not like in
review. Make a full-image cel-style alternative using editable vector shapes
and solid Agon colors. Present it next to the existing versions in an offline
browser review. This tranche stops at visual review; it does not change the
application or publish a new splash as production art.

## Execution contract — 2026-11-19

Owner: Purple Nurples. Smith asked to document the Jukebox consultation,
commit and push the contract, then proceed. He rejected a crop-only pilot;
the first candidate covers the whole 4:3 frame. This extends the existing
ART-001 cel UI task ([earlier asset work](../../../devlog/2026-09-10-cel-ui-assets.md)).

- ART-001-001 [x] Consult Jukebox and record the reusable method and its limits.

- ART-001-002 [ ] Commit and push the full-frame review contract before any
  candidate generation. Announce the verified freeze commit.

- ART-001-003 [ ] Generate editable whole-frame SVG and a native 512×384
  undithered Agon64 PNG; keep an untraced same-source control for comparison.

- ART-001-004 [ ] Verify full opacity, exact palette membership, expected
  dimensions, byte-identical repeat output, and an exact round trip through
  canonical RGBA2222 packing and decoding. Inspect full composition and
  compare edges or lost detail with the raster control.

- ART-001-005 [ ] Deliver an offline review page with native size and integer
  pixel zoom, editable SVG/downloads, and the existing artwork for comparison.
  Record the observations and leave production selection to Smith.

## Research and method

Jukebox's [consultation reply](/Agon/mystuff/agon-dev-env/cross-agent/agentcoms/messages/2026-11-19_12-14-08_jukebox_nurples-splash-vectorization-methods-reply.md)
and [reproducible full-frame experiment](/Agon/mystuff/AgonJukebox/docs/tasks/SKIN-013/flat-color-full-02/README.md)
used an aligned RGB master and a hand-supplied binary mask. The mask made each
fill independently traceable; it could not recover boundaries between any
touching colors already merged in that mask. Potrace 1.16 used `alphamax=0.55`,
`opttolerance=0.2` source pixels, `turdsize=0`, and the BLACK turn policy.
Paths retained their holes. Color samples used a two-pixel inset and the
median RGB of the most populated 16-level RGB bin, matched to Agon64.
That inset only affected sampling. One per-path Inkscape Simplify at `0.0003`
reduced the pilot outlines; repeated rapid Simplify calls over-smoothed an
earlier candidate. Cairo rendered the simplified paths directly at 512×384
with `CAIRO_ANTIALIAS_NONE`; an exact palette lookup rejected fringe colors.
The [original gallery](/Agon/mystuff/AgonJukebox/docs/tasks/SKIN-013/flat-color-full-02/index.html)
used shared 1×/2×/4× zoom with `image-rendering: pixelated` and linked its
native PNG, SVG, fill table, and diagnostics.

Nurples has no aligned hand-made mask. Build full-resolution masks from the
cel master using non-dithered color labels through canonical `agonutils`;
use color masks rather than a global black/white threshold. An opaque base and
ordered coverage masks can avoid uncovered cracks at independently traced
boundaries. Preserve holes, dark outlines, and small lettering/detail where
the target grid allows them. Use the Jukebox tracing settings as the starting
point; skip optional Inkscape simplification unless it is both available and
shown to improve this art. Retain an untraced flat-color control to show what
the vector pass itself changes. The 320×240 GIMP finish is the artistic
reference, not aligned tooling input; the precise hand-edit technique is
unknown. Use registered 512×384 images for pixel differences. The Bayer
version is a separate source treatment, useful as a visual comparison only.

## Inputs and bounds

| Input | SHA-256 |
| --- | --- |
| `assets/img/orig/ui/splash_bg_single_engine_cel.png` (1448×1086 cel master) | `4ed644cab714b76adae1f2b063f271bbf8049aa70edc261dae4a596d7369effa` |
| `assets/img/orig/ui/splash_bg_single_engine_cel_320x240.xcf` (Author finish) | `562c7db659c24658f442af0ea141b56238623e04a892e2e0d63864a0dee8b414` |
| `assets/img/orig/ui/splash_sprite_faithful_v3_512x384.png` (Bayer review input; currently outside Git) | `a7166277186cc05e1806da6ac7a5b8ffe85597a43f09ac05511dacf7f20d627a` |
| `build/palettes/Agon64.gpl` | `c61dd1476415352e74add70f1ed0c81b1d141361a12b26587a6829ee66ec7f94` |

Jukebox bridge source: `AgonJukebox` commit
`d470d329bb4304a69d2f86fd8152233439b4a12e`,
`docs/tasks/SKIN-013/flat-color-full-02/`. Use the existing Potrace and
Cairo bindings for this offline experiment; do not introduce an application
build dependency. Save a hash-verified copy of the untracked Bayer comparison
with the review artifacts without editing its original. The other source
inputs are recoverable from the freeze commit.

The acceptance target is an inspectable full-frame result, not a claim of
artistic approval. Show the 320×240 reference, Bayer trial, flat raster
control, and vector candidate in the browser at native or integer zoom. No
production image replacement, asset manifest/container edits, runtime or
loading-screen changes, emulator deployment, or hardware work in this
tranche. Leave existing unrelated edits intact. Stop for artwork review.
