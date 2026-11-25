# Air-launched bombs — first audition

**Heavy missile accepted on FAE, 2026-11-25:** Smith approved the 10×8 bomb for large ships. Its two frames are separate layers of `missile_10x8.xcf`. It uses the same 180° orientation adjustment, screen-relative speed, random reload, animation cadence, damage and projectile pool as the other missiles.

**Accepted on FAE, 2026-11-25:** Smith selected the final brighter colors for both missiles. The editable XCFs and their four one-layer PNG exports are retained; the 3×6 frames below describe the earlier audition, not the accepted assets.

**Art revision for the next FAE review:** each layer of the two Oryx 6×8 missile XCFs is exported separately, untrimmed. The single is 6×8 and the paired artwork is 16×8. The build rotates them bulb-first as before; muzzle centering and collision bounds use the new dimensions. Drop speed, reload and animation timing are unchanged.

Small ships drop one 3×6 missile; medium ships drop one projectile drawn as a double. Large ships do not shoot. Turn the two frames of each type 180° so the bulb leads. Drop straight down at 3 pixels per gameplay tick relative to the screen, with no terrain correction or inherited sideways motion. Alternate frames in flight. Each visible shooter has an independent randomized countdown; no targeting computer yet.

Stage the current Earth JIT build in the existing Oryx FAE profile. Review silhouettes, cadence, animation, player contact, bottom exit and turret coexistence before tuning.

Staged for human review: random delay 48–175 visible live-player ticks per shooter, six ticks per animation frame. Both types share the existing four-slot hostile projectile pool with turrets; a full pool skips the shot rather than retrying every tick. Spawn point is the center of the ship’s lower edge. The existing diagnostic invulnerability is unchanged. Ten AGNB checks and the Earth JIT reconstruction check passed; six guest files verified after FAT staging. Executable SHA-256: `29b7f43eb61c29775a77dcb8b95d164da53442033635bc2c8f8c3ce77703a483`. Visual playtest pending.
