# Purple Nurples project handoff

Read `/home/smith/Agon/mystuff/agon-dev-env/codex/AGENTS.md` first.

## Purpose and current status

Purple Nurples is an eZ80 assembly game for the Agon platform. Development
resumed on 2026-08-02 after a dormant period. The current binary entry point is
`src/asm/nurples.asm`; generated application data lives under `tgt/`.
The canonical active workspace is `/home/smith/Agon/mystuff/nurples`.
The former `nurples-repair` checkout was moved here with its branches and
development history intact. Older task records may still cite the former path
for historical builds or Oryx profiles; it is not a second development checkout.
Leave existing Oryx review profiles where they are. Create all new Oryx review
profiles under `/home/smith/Agon/mystuff/nurples/.emulator/`.

The Python 3.14 build environment is current. Image tooling uses the canonical
native `/home/smith/Agon/mystuff/agon-utils` checkout plus the thin
project-local Pillow adapters in `nurples_tools/images.py`. Do not recreate or
vendor `agonutils` in this repository.

## Canonical branch and important external dependencies

- Integration branch: `main`.
- Development branch: `dev`.
- Python: `/home/smith/.local/bin/python3.14`, currently 3.14.6.
- Native image extension: `/home/smith/Agon/mystuff/agon-utils`.
- Assembler: `~/.local/bin/ez80asm`.
- Official Agon documentation: `/home/smith/Agon/agon-docs/docs`.
- Isolated emulator profile: `.emulator/nurples`.
- Experimental async MOS target: tagged Eirene fork release
  `3.1.0_uart0_rc_3` at commit
  `97311c437093e416342a6c836cf9914f2ced027d`.

Only repositories below `/home/smith/Agon/mystuff` are user-owned. Treat the
other checkouts directly below `/home/smith/Agon` as read-only references.

## Branch discipline

All development work belongs on `dev`, never directly on `main`. At the start
of every working session, verify that `HEAD` is attached to `dev` before making
project changes or commits. If the worktree is on `main`, switch to `dev`
before editing.

`main` is the integration/release line. Update it only through a deliberate
merge or other integration operation explicitly requested by the Author after
the work on `dev` has been reviewed and accepted. Do not create ad-hoc feature
branches unless the Author changes this policy.

## Session-start reading order

1. Read the canonical `agon-dev-env/codex/AGENTS.md` instructions.
2. Read this handoff.
3. Read `docs/python-tooling.md` for Python or asset-pipeline work.
4. Read `TODO.md`, the only authoritative unfinished-work list.
5. Read the latest file under `devlog/` pertinent to the task.
6. Confirm the current branch is `dev`.
7. Inspect Git status and the relevant source before editing.

## Python environment and verification

Create or rebuild the environment from the repository root:

```bash
python3.14 -m venv .venv
.venv/bin/python -m pip install --upgrade pip setuptools wheel
.venv/bin/python -m pip install -r requirements-dev.txt
.venv/bin/python -m pip install --no-build-isolation --no-deps \
  -e /home/smith/Agon/mystuff/agon-utils
.venv/bin/python -m pip install --no-build-isolation --no-deps -e .
```

Never substitute system Python for `.venv/bin/python`. Verify with:

```bash
.venv/bin/python -m pip check
.venv/bin/python /home/smith/Agon/mystuff/agon-utils/tests/test_agonutils.py
.venv/bin/python -m unittest discover -s tests -v
.venv/bin/ruff check .
```

## Build, emulator, and hardware deployment

Assemble without deploying or launching an emulator:

```bash
.venv/bin/python build/scripts/build_00_all_the_things.py --assemble
```

Production assembly requires the private PT3s listed in
`docs/tasks/MUS-001/samples.json` (see `docs/python-tooling.md`). The local
`tgt/nurples.bin` embeds them; never commit or publish that executable, a
listing containing their bytes, or the tunes. The build writes a matching
local `tgt/nurples.symbols` alongside the binary.
The stable runtime observation contract is documented in
`docs/runtime-telemetry.md`.

Deploy ordinary game changes under the same guest filename, `nurples.bin`,
for emulator and hardware reviews. Use Git commits or separate isolated
profiles to preserve earlier builds. Use a different guest executable name
only when the Author specifically asks for a separate diagnostic variant.

Launch the isolated profile through the canonical environment script:

```bash
cd .emulator/nurples
./fab-agon-emulator
```

Any emulator-related change must remain uncommitted and unpushed until the
Author tests it and explicitly approves a commit. The legacy no-argument mode
of `build/scripts/build_99_asm_assemble.py` also deploys and launches; do not
use it as a normal build command.

Hardware deployment requires a narrowly validated project script before it is
considered supported. Do not replace the mounted application directory with an
ad-hoc recursive shell command.

## Architecture and project-specific conventions

- `src/asm/` contains the game and assembly includes.
- `assets/` and `tiles/` contain source art and tile experiments.
- `build/scripts/` contains build and analysis scripts; several are dormant
  Wolf/MapMaker-era tools listed in `TODO.md`.
- `nurples_tools/` contains installed project-local Python adapters only.
- `tgt/` contains deployable generated data.
- `tools/legacy_fonts/` preserves inactive font experiments; it is not part of
  the installed package or current build.

Shared palette conversion, RGBA2222, compression, and image-format behavior
belongs in canonical `agon-utils`. A project adapter may bridge Pillow objects
to the canonical path API but must not duplicate its palette algorithms.

New indirect state dispatch uses validated numeric IDs, fixed callback tables,
and a real `call` followed by tail `jp (hl)`. The qualified generic machinery
is in `src/asm/state_core.inc`, the game graph is in `src/asm/state_graph.inc`,
and `src/asm/state_new.inc` is the still-disconnected integration scaffold.
Its deterministic qualification is documented in
`docs/state-core-fixture.md`. Do not mechanically rewrite legacy `CALL_HL`
sites or other functioning legacy code for stylistic consistency. Change
legacy code only when it causes a demonstrated unsafe state, unintended branch,
or when a reviewed migration requires replacing that specific boundary.

Once the replacement architecture has reached a proven working state, a
separately reviewed stylistic pass may standardize dispatch and related idioms
across the codebase. Do not begin that pass during stabilization or treat it as
implicit authorization to rewrite legacy code.

## Current task and unresolved checklist

See `TODO.md`. Chronological evidence and decisions belong under `devlog/`.

## Urgent attention on Oryx

For urgent attention only, use the
[canonical Oryx spoken-alert procedure](../agon-dev-env/codex/urgent-alerts.md)
instead of the OpenAI “agent has a question” dialog. Say “Purple Nurples” (or
the session name Smith recognizes) and one short, specific sentence about the
issue. Keep the full context and any question or approval in the originating
chat. Do not include sensitive details in speech, repeat an alert when Smith
is already attending to it, or use it for routine updates or emulator setup.
Normal review, workload, and approval gates are unchanged.

## Network access authority

For access between development machines, read the [canonical network instructions](https://github.com/bgates747/agon-dev-env/blob/feature/standardized-deployment/codex/environment.md#ssh-between-development-machines) in `~/Agon/mystuff/agon-dev-env/codex/environment.md` and its machine-local `.local/ssh-001/CONNECTIONS.md` guide. They own current hostnames, discovery, SSH identities, sudo, and LAN shares. Do not duplicate access recipes or revive old aliases, addresses, or share mappings in this project.

## Olimex bench procedures

The sole authoritative reference for all Olimex bench procedures is
[/Agon/mystuff/agon-extender/docs/operations/olimex-bench/README.md](/Agon/mystuff/agon-extender/docs/operations/olimex-bench/README.md)
and its linked supporting guides. Read it before preparing or operating a bench job.
Do not maintain local copies or alternative bench procedures.
