# Game-only SD deployment

The Author identified this checkout as the intended modern AGNB version after
an older loose-file build from the separate nurples checkout was deployed.
Selected the existing 2026-08-03 15:27 compiled nurples.bin on dev at a4dd2ad,
with its two AGNB containers and sole runtime font. No rebuild or source change.

The nurples-runtime manifest bundle places these four files under
/mystuff/nurples, with the executable at the root and the font in fonts/.
It excludes symbols, test programs, source and loose bitmap assets. Existing
development bundles are preserved. The canonical deployer verifies staging
before replacement and preserves autoexec. No emulator run or commit.

Runtime SHA256 values:

```json
{
  "nurples.bin": "e56f725dbf5a7c32c9496560bce6a6ab925e66cfbd4cb6c4dfa69da435245ccd",
  "game.agnb": "b5e27f3f36ee9f25fe69fb35b3294c1268af1610f53327386fac6ae0055af823",
  "ui.agnb": "d16b74171130fd72eff4dffd8485408aee1a1d356ce6df6e00bd9bb5742a0780",
  "fonts/Lat38-VGA8_8x8.font": "23321abb10437db6b8bc8f21907268b46615d409b38975f6a195eb9643065a69"
}
```

This records the initial AGNB deployment. The subsequent joystick-choice
build, its replacement binary hash, and the Author's emulator/hardware
acceptance are recorded in [the joystick-choice log](2026-09-09-joystick-choice.md).
