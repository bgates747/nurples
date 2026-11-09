# Pinned firmware-default compatibility audit

`FW-001` audited the current Purple Nurples production and test candidates
against the Author-selected firmware defaults. This was a source, artifact,
profile and retained-evidence audit only: it did not build, deploy, flash,
launch an emulator or access a target. On 2026-11-09 this record was refreshed
against the Author-directed async-enabled amendment published under the same
VDPlet release tag. The static compatibility conclusions did not change, but
the current VDP binary, source identity and enabled-feature set did.

## Selected defaults and verified release artifacts

The selected VDP on Alexe, Olimex and Console8 is
[VDPlet v0.1.2-alpha](https://github.com/bgates747/agon-vdplet/releases/tag/v0.1.2-alpha),
selection `0.1.2-alpha-async-r1`, source commit
`60e5b4ebafa3e1c8bb740e45260d78cdb6125e30`. The 13 official release assets
were downloaded afresh; all 12 payloads listed by the published `SHA256SUMS`
passed. The files material to this audit are:

| Artifact | Bytes | SHA-256 |
|---|---:|---|
| `SHA256SUMS` | 1,081 | `52438920e70480cd1c0f0c9543fbf2e0f7771ce937b92f422ef2f6ea394f79ba` |
| `vdp-vdplet-0.1.2-alpha.bin` | 1,115,568 | `dc43ecb8b1c324ffd1a032e6cc0c56d8c05dd33a355ce170c23adb35ffb533ab` |
| `release-manifest.json` | 5,584 | `23036cfdd5d12c9ed54d197ab5184f2227dd635e4c09647a5c9efda95f129f5d` |
| `firmware-release.json` | 450 | `1599d0dbb515a9e59dee2d97ebe2d11b341ebdb6200bdab1e96a7d94678ee651` |
| `firmware-uart-provider.json` | 1,529 | `8e6c95c2a6fa221f54863687da7a07ad8d5b956421bd8e95b33b1a20067ab1d5` |

The release manifest states that the public ABI and wire versions are
unchanged. Pixels, the `text96` profile, borrowed colour rows, bounded
`READ_BYTES`, the UART receive-full recovery repair, and the ordinary async
OPEN/SUBMIT/PUBLISH/STATUS/STOP/CLOSE worker are enabled. The async table is
ABI 1.7/capability 799 and permits exact 512- and 513-byte OPEN operations.
Counted return, PCM wire support, the private overlap witness, quiet polling
and admission diagnostics remain disabled. The release records 24 release,
49 UART and 118 provider checks plus host/native inspection, but explicitly
records no execution of these exact versioned bits on hardware or an emulator
at publication.

The amendment supersedes, but deliberately identifies, the pre-async image
previously audited under this tag: source
`b6c7c58331bf4cbbd54dbc4a1301c124e5963622`, 1,100,304-byte binary SHA-256
`b004570b607b45bbd6e87a3c704b65b23d3221981db3aac6cf2578aa22d766e3`,
and `SHA256SUMS` SHA-256
`6469f6d6717ede6acb68d9b7ce519db07a1d5a08a0ef7c765739dfe86658aa9d`.
That identity is historical evidence only and is not the current deployment
or qualification pin.

Alexe and Console8 use
[Eirene MOS 3.1.0_uart0_rc_3](https://github.com/bgates747/eirene-mos/releases/tag/3.1.0_uart0_rc_3),
tagged commit `97311c437093e416342a6c836cf9914f2ced027d`. The retained published ZIP is
1,887,937 bytes with SHA-256
`9c8a9a28b0f597e6a9294d806491ffca3902c16cf1876b092ae8944f65368061`.
All 360 entries covered by its internal `SHA256SUMS` passed. Its required
`mos-rc3.bin` is exactly 100,879 bytes with SHA-256
`d7a51147f3381cd68d56f3fbe4605434a897dfae1b049269afeba09ec2b7eb50`.

Olimex instead uses Extender's selected integrated EMOS baseline:
`agon-emos-eirene-r04-b2026-11-06-09-02-21Z`, source
`2c79c6006d6bcb0c2b3f9d8d6aae3ef4b36c730a`, 131,072-byte `ROM.bin` SHA-256
`a25f1612f4aaa542c5488cf3266d968ad3d1d61e9df3b5ce2c680650f5d7a2ca`,
map SHA-256
`11a296f09e1d2b7125e1a40ce0aea62042f299e93ea85afc6a6459a351866d20`.
Its selected profile has `EMOS_UART0_RFC25=1` and retains synchronous
`RST 10h`/`RST 18h`. This is the exact current default, not a moving branch
head or bare RC3 ROM.

## Application-interface review

The production game and map suite use ordinary synchronous `RST 10h` and
`RST 18h`, MOS system variables, keyboard-map and file calls, software-sprite
VDU commands, ordinary bitmap/buffer commands, and the existing VDP reply
surface. Production source contains no syscall `25h` call. It does not use
affine or hardware sprites, the VDPlet async worker, counted return, PCM sink,
or the dormant AGNB audio entry point. The failed affine-sprite experiment is
therefore not part of this candidate set.

No source-level conflict was found between those production dependencies and
the published VDPlet/RC3 or VDPlet/integrated-EMOS interfaces. This is static
compatibility, not runtime qualification. In particular, the new VDP image has
not yet been paired with any Nurples candidate on Alexe, Olimex or Console8.

UART-001 is different. Its isolated overlay invokes syscall `25h` with the
bare-RC3 borrowed-buffer/callback contract and deliberately never enters the
production build. It is statically compatible with the pinned RC3 ROM on
Alexe and Console8. The Olimex EMOS default includes a guarded RC3-style `25h`
but gives real context/admission result codes. The current fixture sets its
in-flight flag before the call and ignores the returned admission result; a
rejection becomes a bounded callback timeout. That is useful fail-closed
behavior, but it is not an audited EMOS adapter. UART-001 must therefore remain
an RC3-specific Alexe/Console8 fixture unless a separately reviewed Olimex
variant handles the selected EMOS admission contract.

## Current candidate inventory

| Candidate | Exact identity | Compatibility disposition |
|---|---|---|
| Production game | `tgt/nurples.bin`, 36,439 bytes, `63c89d2676f6645631b2240f23002b8b68d45069979a4804ca7d14f34e720088` | **Statically compatible; runtime blocked.** No incompatible API use was found. It needs first qualification with the pinned VDP and target-appropriate MOS/EMOS. |
| Production data | `game.agnb` `e06f127f6239fa435ccdf19f97a2cccfad9924f70b22a9035750c320388cdf98`, `ui.agnb` `655b3476bea1304b4ca8ea486d25e7e1e53cb8bdd9846a4139555d4b9947717b`, font `23321abb10437db6b8bc8f21907268b46615d409b38975f6a195eb9643065a69` | **Compatible inputs; runtime blocked with the executable.** They exercise existing synchronous buffer/bitmap/font paths and do not depend on the amended async feature. |
| Historical two-VBlank bench game | commit `2f0f095` `tgt/nurples.bin`, 36,443 bytes, `e7850b78db8c3c5ae1a232836f58f68dcbd56af3066049be8ee335325909b3b0` | **Obsolete historical timing candidate.** It was separated under `/test/nurples`, is absent from current deployment bundles and must not replace the single-VBlank production game. Preserve prior evidence; do not use it for pinned-default qualification. |
| Standalone map suite | `tgt/nurples-map-tests.bin`, 37,672 bytes, `129f5552d975f1446562a901946ebfca0dbfbc5eb30d6e940fcbc07fc4d0012a` | **Statically compatible; stale and blocked.** It shares the production interface surface but predates later production changes and has no pinned-default run. Rebuild only under its existing test gate before use. |
| UART-001 narrow R01 | 37,983-byte historical artifact `9ef2f64e54c118d33715c77b87c552531b7dd00850eb6131cc47d3814b5dcd94` in retained profile | **Obsolete historical failure.** Preserve evidence; do not rerun. |
| UART-001 narrow R02 | 38,041-byte historical artifact `aed9d7ddb7fcf1e61161069ccae8fa655187ed669d62513994e4944c5097bb7a` | **Historical RC3 pass, superseded.** Its rendering and repeated-`RUN` result remains valid only for the old identified VDP runtime. |
| UART-001 full-frame R02 | current dirty `tgt/nurples-uart001.bin`, 55,198 bytes, `cb2f56cad1248be0b81560b37809351d5bd8650d013c85d99623120356d6015c` | **Current RC3 test candidate; blocked.** The source/output are deliberately uncommitted continuation work. It requires a new release-matched Alexe profile before another RC3 run. It is not an Olimex candidate without an EMOS admission adapter. |
| State-core and player-input headless fixtures | source-only host/headless fixtures | **Firmware-independent.** They neither deploy to a VDP target nor qualify a firmware pairing. |
| Dormant assembly experiments | no current bundle or candidate artifact | **Not candidates.** `test_collisions.asm`, `test_viewports.asm`, joystick and other dormant programs are outside current build/deployment manifests. |

The three deployment bundles inherit the status of their executable:
`nurples` and `nurples-runtime` carry the compatible-but-unqualified production
game; `map-tests` carries the stale compatible-but-unqualified map suite. The
bundle machinery does not install or select MOS/VDP firmware and therefore
cannot establish firmware compatibility by itself.

## Profile and target matrix

### Alexe

The project-local ordinary profile `.emulator/nurples` is **obsolete for new
qualification**. It identifies Arthur MOS v3.0.2 (`mos_platform.bin`
`d564243283972690933a4554296ad6202ca4ef54572279533a942960846bebae`)
and launches Fab's stock platform firmware. Its SD tree does
contain an exact copy of the current production bundle, but that does not make
the runtime conform to either pin.

The retained Oryx UART profiles `uart001-rc3`, `uart001-rc3-r02` and
`uart001-rc3-full-r01` correctly contain RC3 MOS SHA-256
`d7a51147f3381cd68d56f3fbe4605434a897dfae1b049269afeba09ec2b7eb50`,
but all three contain the older 1,080,608-byte ESP firmware SHA-256
`201ad8ab4af807c261cb92634286b2fbaedcf3879f140e820b60ab54cf707e2a`
and Flexe VDP module SHA-256
`0866265bd6f3d73ad572f31dc1bb954f308ed9a5d857efa9414d37e1ade13374`.
They are frozen historical evidence,
not candidates under VDPlet v0.1.2-alpha. No release-matched Alexe profile was
found. A future profile must bind the VDPlet release identity and a matching
Alexe runtime representation rather than simply relabeling the old module.

### Olimex

The selected standard is exact and locally retained: VDPlet
`dc43ecb8b1c324ffd1a032e6cc0c56d8c05dd33a355ce170c23adb35ffb533ab`
plus integrated EMOS
`a25f1612f4aaa542c5488cf3266d968ad3d1d61e9df3b5ce2c680650f5d7a2ca`.
The last physical handback still recorded the older VDP application SHA-256
`5d65797e53a70db8716cc3bc31dc936e431aea975ffad8efb76d5d7e8c1e3d07`
with the same EMOS. Thus the new VDP
standard was selected but not yet installed or exercised. Production and map
suite candidates are statically compatible but physically blocked pending the
scheduler's normal standard preparation and a separately authorized Nurples
run. No firmware exception is required for those two applications.

UART-001 is not approved for that pairing for the admission-contract reason
above. Running bare RC3 on Olimex would be a firmware exception, but it is not
justified by the current target policy: RC3 qualification belongs on Alexe and
Console8. If an Olimex test is later wanted, adapt and freeze the fixture for
the selected EMOS rather than silently substituting RC3.

### Console8

The correct proposed pair is VDPlet
`dc43ecb8b1c324ffd1a032e6cc0c56d8c05dd33a355ce170c23adb35ffb533ab`
and RC3 MOS
`d7a51147f3381cd68d56f3fbe4605434a897dfae1b049269afeba09ec2b7eb50`.
No current installed-state receipt or Nurples execution under that exact pair
was found in this project. The production game, map suite and RC3-specific
UART-001 interface are statically compatible, but every Console8 runtime claim
is blocked until the exact installed images are verified and the applicable
human-gated run is performed. Earlier physical acceptance did not record this
new pair and is superseded for firmware-compatibility purposes.

## Exceptions and required follow-up

There is no production-game firmware exception. Production and map testing
must use the selected defaults.

The only inherent different-firmware case is UART-001's deliberate bare-RC3
qualification on Alexe or Console8. That is not an exception to the stated
default because RC3 is the selected MOS on those targets. It is target-excluded
from Olimex until adapted to the integrated EMOS contract.

No pinned-default runtime pass exists yet. Follow-up is therefore discrete:

1. Replace or retire the obsolete ordinary and UART Alexe profiles through the
   existing emulator-change review gate; preserve frozen historical profiles.
2. Produce a release-matched Alexe representation of VDPlet v0.1.2-alpha and
   verify its provenance before invoking production, map or UART candidates.
3. Route any Olimex run through Extender's exact standard selector and physical
   lease; do not assume selection means the VDP is already installed.
4. Verify the exact VDP/MOS images on Console8 before its separately authorized
   human run.
5. Keep UART-001 off Olimex unless a reviewed fixture variant checks the EMOS
   `25h` admission result and preserves its source-lifetime/drain fences.

These are qualification/remediation tasks. The audit itself is complete and
found no reason to modify the production game binary or its assets.
