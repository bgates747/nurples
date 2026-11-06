# RC3 asynchronous UART qualification

Purple Nurples will be the first full game used to evaluate the startup-fixed
RC3 implementation of Tom Morton's interrupt-driven UART0 sender. The immediate
goal is a narrow, deterministic integration proof; it is not yet a performance
claim or hardware-deployment approval.

## Canonical workspace correction

The older `/home/smith/Agon/mystuff/nurples` checkout was accidentally active.
Its divergent `dev` branch and all dirty work were preserved as
`deprecated/legacy-dev-pre-repair-20261105`. This repaired AGNB checkout is now
the canonical workspace on `dev`. Its three local development commits were
merged with the one remote `dev` documentation commit at `78fb307`; nothing was
pushed.

The authorized `hardware-sprites` branch remains at `f81e60d`. Its nine dirty
and untracked files were preserved verbatim in the named stash
`hardware-sprites WIP preserved before canonical dev checkout 2026-11-05`
before this worktree returned to `dev`.

## Firmware pin

The qualification target is the locally tagged and published fork release:

1. Tag: `3.1.0_uart0_rc_3`.
2. Commit: `97311c437093e416342a6c836cf9914f2ced027d`.
3. ROM: 100879 bytes, CRC32 `B08C80B8`.
4. SHA256: `d7a51147f3381cd68d56f3fbe4605434a897dfae1b049269afeba09ec2b7eb50`.
5. ABI: foreground ADL/MBASE0/EI single producer; syscall `25h` takes
   HL=immutable source, BC=positive 24-bit length and DE=non-null assembly
   callback. There is no queue, admission status, cancel or finish syscall.
6. Completion: the callback releases source RAM after the final FIFO fill;
   UART TEMT and VDP command execution remain separate boundaries.

RC3 contains the startup-table correction plus the exact RFC25 sender. The
original `ffab9c9` RFC image and its emulator evidence remain useful historical
controls but are no longer the Nurples deployment candidate.

## First game seam

The initial fixture will packetize one actual `move_sprite_group` cohort whose
behavior remains output-silent for the whole controlled interval. Lasers,
fireballs, seekers, the player and collision/spawn paths are excluded from that
first cohort because their behavior routines can emit VDU commands outside the
common presentation tail.

Three controls are required:

1. Existing fragmented synchronous commands.
2. One persistent packet sent synchronously.
3. The byte-identical packet sent with RC3 syscall `25h`.

Existing VDU helpers rewrite static `@cmd` storage and therefore cannot be lent
to MOS. The fixture will use dedicated static packet buffers and permit only one
outstanding immutable borrow. Its ADL callback will set one completion byte and
return without MOS/VDP calls, EI/DI, index registers or stack manipulation.
Ordinary output and exit must serialize after source release and a separately
identified drain/receiver boundary.

After the narrow proof passes, the performance case may compute/build frame N,
submit N, and overlap only independent N+1 work. Packetized synchronous and
asynchronous controls must retain the same logical input, update and 60 Hz
pacing schedule. Stock VDP firmware is required; the affine/hardware-sprite
experiment is unrelated.

No emulator profile, firmware image, mounted card or hardware was changed by
this planning step.

## Initial fixture implementation and build

The first controlled fixture is now implemented without changing production
`nurples.asm`. `build/scripts/build_uart001_fixture.py` copies the assembly tree
to an isolated staging directory, adds `tests/asm/uart001_fixture.inc`, and
redirects only the staged main-loop `do_game` call. The published pair is:

1. `tgt/nurples-uart001.bin` — 37,983 bytes, SHA256
   `9ef2f64e54c118d33715c77b87c552531b7dd00850eb6131cc47d3814b5dcd94`.
2. `tgt/nurples-uart001.symbols` — 50,899 bytes.

The fixture lets the real startup load both AGNB containers and establish the
production sprite definitions. It then hides ordinary actors and runs eight
stationary explosion records for 120 frames in each of the three controls.
Every frame builds the same 120-byte select/frame/move stream and checks it
against the first fragmented-control packet. The async path uses two 128-byte
static buffers, one outstanding RC3 submission, a primary-A-only callback,
bounded foreground release/TEMT polls, and a fresh text-colour reply as its
ordered receiver token before ordinary output or exit. Faults, callback count,
mode, frame and elapsed values are named in the symbol file for inspection.

The Pi host has no `ez80asm`; policy therefore routed these finite compiles to
Oryx. Lease generations 324 and 325 assembled the isolated trees successfully
in 0.02 seconds each and were released with fresh empty-host evidence and no
successor. The second build adds explicit fixture-state initialization for
repeated MOS `RUN`. No emulator process was started.

`tools/stage_uart001_oryx.py` is the separate deployment step for human review.
It must run on Oryx and creates a new profile rather than modifying any accepted
Eirene profile. The tool verifies the RC3 MOS, map, Flexe host, VDP and ESP32
firmware hashes; copies those inputs into the project-local profile; creates a
fresh private FAT16 card image; installs only the fixture, symbols, current AGNB
containers and fonts; and records a complete identity receipt. Its CRLF
autoexec selects the keyboard, changes to the fixture directory and loads the
binary without running it. It never assembles or launches an emulator.

The Oryx deployment is staged and stopped at
`/home/smith/Agon/mystuff/nurples-repair/.emulator/uart001-rc3`. The first
staging attempt stopped before publication because Oryx mtools 4.0.49 does not
implement `mmd -s`; the temporary directory was removed. The tool now creates
each FAT directory explicitly and the second attempt passed both manifest
checks, launcher syntax validation and a direct card listing. The card SHA256
is `8b4edca875ed34b1b33a4de6cda74a5a28aea1517d33840013f1d237b59d178b`.

No emulator was started. Final host inspection found the separately owned
Eirene `RFC001-LAT001` interpreter run active, so Nurples did not attempt a
checkout. Human review request `1cc79fdde111` (`UART-001-RC3-HUMAN-R01`) is
queued behind it under the shared Oryx FIFO. The profile may be launched only
after the active owner releases and Nurples obtains a fresh idle checkout.

The Eirene owner subsequently released generation 326 with an empty host.
Nurples obtained its own fresh empty-host inspection and checked out generation
327 for the Author's review. The checkout was asserted immediately before the
launch handoff. The Author, rather than the agent, remains responsible for the
graphical launch; the checkout stays with Nurples until the Author closes the
emulator and reports the review result.

## Related receive-loss evidence

Eirene's immediately preceding `LAT001` model campaign used the same unchanged
RC3 ROM and the same identified actual-firmware Flexe VDP family as this review
profile. The JIT cell transferred 622,592 checked bytes, emitted and latched all
32 key packets, and reported no RX overrun. The interpreter cell transferred
491,520 checked bytes and completed all guest byte/callback/restoration checks,
but the host reported 16 RX overruns and only 30 of 32 key packets latched inside
the measurement windows. Both cells matched all 136 outbound packets and saw
all 68 expected asynchronous callbacks. The interpreter host failure is real
for that model even though the guest printed PASS; the guest cannot read the
host overrun counter.

Source inspection supplies a plausible mechanism: RC3's `WAIT_CTS` spins while
the UART ISR is under DI, which can delay receive, timer and VBlank service.
Neither Flexe engine is a calibrated physical timing model, so the JIT success
does not prove Console8 input retention and the interpreter failure does not
prove a hardware defect. This Nurples visual cohort uses the JIT engine and is
only a transfer/render integration check. It cannot qualify input safety or
replace the separately gated Console8 confirmation required before RC3 is
treated as production firmware.

## R01 visual failure and presentation-boundary repair

The Author ran the first JIT review. Normal assets loaded, the GUI and static
starfield appeared, no controlled sprite cohort became visible, and the fixture
exited automatically at its expected end. This is a failed visual result.

The retained host evidence rules out a transport or emulator crash: 680,437
MOS-to-VDP bytes and 1,199 delivered VDP-to-MOS bytes were captured; Flexe
reported no backend failure, unhandled/unregistered/unmapped operation, TX
drop, or RX overrun. The command stream contains 2,985 sprite selects, 2,905
sprite moves and 25 sprite shows, but zero `VDU 23,27,15` sprite-update
commands.

That missing presentation boundary is the fixture defect. Production sprite
movement relies on a later HUD drawing operation to commit sprite changes, but
the isolated fixture deliberately omits all unrelated drawing. Its setup
show/move commands and its per-frame move commands therefore never reached the
screen. This is not evidence against RC3 asynchronous transfer.

The corrected fixture explicitly updates after setup. Each measured frame now
constructs a 123-byte canonical stream: eight 15-byte select/frame/move records
followed by the three-byte sprite-update command. The fragmented control emits
that same tail through `vdu_sprite_update`; the synchronous and asynchronous
packet controls carry the identical bytes in their packet. The established
static sprite frames remain intentional so packet bytes stay identical across
all 120 frames and all three modes. Targeted overlay/packet/profile tests pass.
Oryx generation 329 assembled the corrected isolated source in 0.02 seconds
because the Pi has no `ez80asm`; the checkout was released with fresh empty-host
evidence and no successor. The corrected outputs are:

1. `tgt/nurples-uart001.bin` — 38,041 bytes, SHA256
   `aed9d7ddb7fcf1e61161069ccae8fa655187ed669d62513994e4944c5097bb7a`.
2. `tgt/nurples-uart001.symbols` — 51,109 bytes, SHA256
   `4b95180a775234dedfa4a5e1465271e76f59292e1d9f8c3233730ac1134608e2`.

The new stopped profile is
`/home/smith/Agon/mystuff/nurples-repair/.emulator/uart001-rc3-r02`; its card
SHA256 is `0aa64841c154262fbd1ba0edc6051c80b1fb54f6eb227fcc2a97f70558096748`.
All RC3 runtime/card identity checks pass, and autoexec loads without running.
The failed R01 profile and evidence remain unchanged. Nurples obtained a fresh
empty-host checkout at Oryx generation 330 for the Author's R02 review.

Targeted Python tests pass. The complete suite could not be cleanly executed on
this Pi checkout because its existing Pillow extension and `ruff` executable
are foreign-architecture binaries, while assembly-dependent tests expect the
missing local `ez80asm`. Fifty-six tests passed in that run, including both new
fixture tests; one existing headless-input test skipped. These environment failures
predate UART-001 and were not repaired as part of this fixture.

## R02 visual pass

The Author's corrected R02 run displayed the complete intended cohort of eight
small orange explosion sprites over the GUI and static starfield. All three
transfer modes deliberately use identical fixed-frame bytes, so no visible
animation or phase transition is expected. The sprites disappeared during the
fixture's normal cleanup immediately before the application exited.

The retained JIT evidence confirms that all three 120-frame modes completed and
were presented. The VDU stream contains exactly 363 sprite-update commands:
three setup commits plus 360 measured-frame commits. It also contains 2,905
sprite moves and 25 sprite-show commands. Flexe retired 5,346,228,684
instructions over 1,683 frames and reported no backend failure, unhandled,
unregistered or unmapped operation, TX drop, or RX overrun. UART totals were
680,869 MOS-to-VDP bytes, 615 VDP-to-MOS bytes, 960 TX interrupts and 559 RX
interrupts. This is a pass for the narrow RC3 asynchronous-transfer/render
integration check; it does not expand the performance or physical-hardware
claims described above.

The Author subsequently confirmed that a repeated `RUN` in the same emulator
and MOS session also passes. This closes the fixture's restart-initialization
acceptance item. After a fresh empty-host inspection, Nurples released Oryx
generation 330 at 2026-11-06T05:15:56Z; no successor was queued.
