# 2026-08-03 — Linear targeting restoration

## Provenance

The targeting computer implements the compact law-of-sines linear targeting
method described by Miked0801 on the old RoboWiki:

- `https://old.robowiki.net/cgi-bin/robowiki?LinearTargeting`
- Archived mirror:
  `https://robowiki.net/wiki/Linear_Targeting/Archived_Talk_20071111`

For target speed `Vt`, projectile speed `Vp`, target heading `Ht`, and bearing
to the target `Bt`, the exact lead angle is:

```text
A = asin(Vt * sin(Ht - Bt) / Vp)
```

The Z80 implementation follows the NanoLinearTargeting size optimization and
drops `asin`, using its argument as the small-angle radian approximation. It
then multiplies by `256/(2*pi)` to convert radians to the project's angle256
representation. That 8.8 implementation is the behavioral reference for the
eZ80 restoration.

## Failure trace

1. The initial 16.8 port reconstructed target speed and heading, evaluated the
   law-of-sines numerator, divided by projectile speed, and converted the lead
   angle just as the Z80 implementation did.
2. The first port incorrectly consumed `HL` after `sdiv168`. The 24-bit signed
   division routine returns the quotient in `DE` and remainder in `HL`. Commit
   `f10b7eb` corrected that by exchanging `DE` and `HL` after the call.
3. Results were still directionally wrong because `sin168` mishandled negative
   angles. It took the absolute value, computed `abs(angle) - 256`, and then
   indexed the table using only the integer byte. For example, `-64` became
   table index `64`, producing `+1`; the correct modulo-256 index is `192`,
   producing `-1`. Consequently the targeting computer could not produce a
   negative lead angle.
4. Commit `cab67d5` deliberately commented out the failed lead calculation and
   retained head-on `Bearing_t` aiming so turret projectiles remained usable.
   The fallback contained the defect but did not cause it.

## Repair

`sin168` now uses the integer byte of the incoming two's-complement 16.8 angle
directly. That byte is already the correct modulo-256 lookup index for positive,
negative, and wrapped angles. The repair retains the sandbox's separate ADL
pointer-safety fix, which zero-extends the 16-bit `MLT` result before adding it
to the 24-bit sine-table base. `cos168` continues to add 64 angle256 units and
inherits the same modulo behavior.

The consumer audit found one additional implementation at
`fixed168/fixed168.inc`, used only by the standalone `fixed168.asm` and
`calc168.asm` utilities. It retained both the negative-angle defect and the old
unsafe ADL lookup-address construction. Its `sin168` routine is synchronized
with the runtime implementation, and the assembly-safety test now enforces
sign-preserving modulo lookup plus zero-extended pointer arithmetic in both
copies.

Runtime consumers are limited to the targeting computer and
`polar_to_cartesian`. The latter feeds seeker heading movement, seeker orbit
coordinates, and fireball projectile vectors. Existing seeker inputs are
ordinary angle256 values from 0 through 255 (or the constants 96 and 160), so
their sine results are unchanged. `cos168` remains correct across its
64-angle offset and wrap. Only negative lead-angle and wrapped projectile
headings change behavior, and those were previously incorrect.

The targeting computer's original 16.8 law-of-sines path is active again. It
retains the original one-pixel-per-frame ground-scroll compensation, the
NanoLinearTargeting small-angle approximation, and the established component
velocity output contract. Both the original and archived provenance URLs now
accompany the active implementation.

## Qualification

`nurples.asm` assembled successfully in the `nurples-repair` sandbox. The
resulting binary was run through the dedicated `nurples-repair` emulator
profile, and the Author confirmed that the restored targeting behavior works.

`ASM-009` is accepted and removed from the authoritative TODO. The changes
remain uncommitted pending an explicit commit request, in accordance with the
emulator-validation gate.
