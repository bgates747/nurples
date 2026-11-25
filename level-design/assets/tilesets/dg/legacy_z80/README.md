# Original Z80 Nurples artwork

These five passive 16-by-16 tiles are copied unchanged from the recovered
original Z80 Nurples project. They are the source artwork for modern runtime
tile IDs 17 through 21. The current asset build converts them through the
canonical Agon64/`agonutils` path, writes the corresponding numbered DG tiles
and RGBA2222 payloads, and updates their cells in `../dg0.png`.

The old landing-pad and laser-turret images are intentionally not imported.
Their old level IDs map to the modern `tile_pad_small` and
`tile_turret_fireball` IDs so the current animations, launch/fire behavior,
destruction handling, and crater transformation remain authoritative.

Recovered source location at import time:

```text
/home/smith/gatesx/Agon/Z80/img/production/
```

Original SHA-256 values:

```text
2c8cf59f77a61d993690b640f12fbaf5f7f5282508c4eb394bbba8097393ae07  tile_blank.png
3a0a2bef4dd29facfda9b1d812c61690ca30e1d5ecfeb2e76aaaf5e48da67657  tile_circle.png
e4e67e14c31e77a1d20d132f22c324baa77e61a82fc4b92ceb883f7f7a512222  tile_conduit_cross.png
e740356104031e4d085949ed02084ad17a6da2afbc2984901a0500d23e513d18  tile_conduit_straight.png
ec532cdbb4d4a39e65b589ac084e57a6b03d2b596527d5c0eec180055b6d0146  tile_conduit_straight_rot.png
```
