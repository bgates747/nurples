# Rotation frames: upload, generation and lifetime

**Design discussion, 2026-11-24. Not implemented or selected for a trial.**

Today the build uploads 32 headings apiece for the seeker, turret barrel and two fireball animation phases. That is 128 of the 165 sprite-art bitmaps, although the turret is plotted as a ground bitmap. At 16×16 RGBA2222 those frames total 32 KiB: many records, but only about 6.4% of the Earth JIT fixture’s resident game-bank pixel payload. One starfield texture alone is 64 KiB.

## What stock firmware and the two other games actually do

- Stock Bistromathics v2.16.0 offers two paths. A drawing-context affine matrix affects a plot, not its source. Buffered command 0x28, with the affine feature enabled, instead inverse-samples the source into a destination buffer and creates an RGBA2222 bitmap there. That bitmap can be plotted or attached by ID as a sprite frame (sprite command 0x26). Replacing a bitmap already in use can clear sprite frames: generate before attaching, detach before freeing. See the tagged [transform implementation](https://github.com/AgonPlatform/agon-vdp/blob/v2.16.0/video/vdu_buffered.h#L2048) and [sprite frame handling](https://github.com/AgonPlatform/agon-vdp/blob/v2.16.0/video/sprites.h#L80).
- [Rally traffic](/Agon/mystuff/AgonArcade/rally-production/src/main.cpp) normally scales or mirrors at plot time. Its [custom scaler](/Agon/mystuff/AgonArcade/docs/tasks/RALLY-28/custom-vdp/scaler-comparison/src/production-sampler.inc) reuses a horizontal sample map, writes a scratch bitmap, then plots it. The [comparison](/Agon/mystuff/AgonArcade/docs/tasks/RALLY-28/custom-vdp/scaler-comparison/STATUS.md) found a 1.98–3.60× speedup over cached affine plots in 30 Pi-host reduction cases, not an ESP32 measurement or rotation result.
- [Wolf3dOrig](/Agon/mystuff/Wolf3dOrig/agonport/vdplet/production/wolf3d_renderer.cpp) uses incremental fixed-point coordinates and writes opaque pixels directly into the back buffer, checking wall occlusion for actors. Neither game’s scaler demonstrates on-the-fly rotation of a live stock VDP sprite.

## Proposed lifetime

Keep each enemy’s north-facing master resident. Let a level declare which rotation sets it uses; prepare those before they are needed, not on first spawn or during a combat tick. Gameplay would still select a ready bitmap by heading. That keeps the runtime workload predictable and allows different levels to use different rotatable enemies without retaining every level’s frames.

Each set costs 256 bytes for the master plus 7.75 KiB of derived pixels while resident. Baking all four existing sets would avoid uploading 124 frames (31 KiB of pixel transfers, before command overhead). Baking alone does **not** reduce resident pixel memory if all sets stay live, nor does it reduce scanout cost. The RAM saving comes from releasing sets no longer needed. Reuse a set across adjacent levels that both need it.

Warp remains playable: a live projectile can outlast the level that fired it. Do not recycle its bitmap IDs on a level-complete event alone. Keeping shared projectile art resident and scoping only level-specific hulls is one simple option, not a decision.

Open questions: when can preparation fit without a visible transition stall or excess peak RAM? Does stock rotation reproduce the existing 16×16 pivot, crop, transparency and heading convention? Is baking faster than uploading on physical hardware? The current pre-rendered art is the comparison control; no change to the loader, firmware or game is implied.
