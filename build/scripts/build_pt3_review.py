"""Build the private MUS-001 FAE payload; never deploy, publish or launch it."""
import argparse
from dataclasses import replace
from hashlib import sha256
import json
from pathlib import Path
import runpy
import shutil
import subprocess
from tempfile import TemporaryDirectory

from build_game_over_review import ASSETS, ROOT

AMP = "91ef401bcb7f636e7593ad1ba176508344deebb9"
BANK = "962c1868b3a5485c3021af1ad908181fb92f34fcb95004b081a76fb2d31759c1"


def inject(path, anchor, replacement):
    text = path.read_text()
    if text.count(anchor) != 1:
        raise ValueError(f"missing or ambiguous fixture hook: {path.name}: {anchor}")
    path.write_text(text.replace(anchor, replacement))


def effect_programs(path):
    if sha256(path.read_bytes()).hexdigest() != BANK:
        raise ValueError("unexpected AgonArcade patch generator")
    bank = runpy.run_path(str(path))
    patches = bank["patches"](False, False)
    laser, = patches[2]
    noise, bass = patches[5]  # soundboard key 6: explosion v3
    voices = [replace(v, channel=c) for v, c in zip((laser, noise, bass), (5, 6, 7), strict=True)]
    cmd = bank["audio"]

    def retrigger(cue):
        # Selecting the waveform aborts a busy note on stock VDP. Restore
        # its envelopes before play; prepare both blast voices before onset.
        return b"".join(v.setup() for v in cue) + b"".join(v.play() for v in cue)

    programs = {
        "setup": b"".join(cmd(v.channel, 8) for v in voices),
        "laser": retrigger(voices[:1]),
        "blast": retrigger(voices[1:]),
        "stop": b"".join(cmd(v.channel, 9) for v in voices),
    }
    return "".join(
        f"mus_fx_{name}:\n" + "".join("    db " + ",".join(map(str, body[i:i+16])) + "\n"
                                     for i in range(0, len(body), 16)) +
        f"mus_fx_{name}_end:\n" for name, body in programs.items()
    )


def build(assembler, amp, bank, song, output):
    pins = json.loads((ROOT / "docs/tasks/MUS-001/samples.json").read_text())
    pin, = (p for p in pins if p["filename"] == song.name)
    module = song.read_bytes()
    if len(module) != pin["bytes"] or sha256(module).hexdigest() != pin["sha256"]:
        raise ValueError("wrong audition module")
    with TemporaryDirectory(prefix="mus001-") as temporary:
        src = Path(temporary) / "asm"
        shutil.copytree(ROOT / "src/asm", src)

        def assemble():
            subprocess.run([assembler, "-s", "nurples.asm", "n.bin"], cwd=src, check=True)
            return (src / "n.bin").read_bytes()

        if assemble() != (ROOT / "tgt/nurples.bin").read_bytes():
            raise ValueError("ordinary build differs from its control")
        for name in ("libsupport.asm", "pt3play.asm", "pt3tab.inc", "ayvdp.asm"):
            (src / name).write_bytes(subprocess.check_output(
                ["git", "-C", amp, "show", f"{AMP}:lib/{name}"]))
        (src / "terminator.pt3").write_bytes(module)
        (src / "mus_fx.inc").write_text(effect_programs(bank))
        shutil.copyfile(ROOT / "tests/asm/pt3_review.inc", src / "pt3_review.inc")
        main = src / "nurples.asm"
        for old, new in [
            ('    include "tables.inc"', '    include "pt3_review.inc"\n    include "tables.inc"'),
            ("    call runtime_process_init", "    call runtime_process_init\n    call mus_process_init"),
            ("    call game_pause_poll", "    call game_pause_poll\n    call mus_pause_edge"),
            ("    jp nz,@state_exit", "    jp nz,@state_exit\n    call mus_frame"),
            ("main_end:\n", "main_end:\n    call mus_stop\n"),
        ]:
            inject(main, old, new)
        # Use only successful laser allocations and the single finale transition.
        inject(src / "player_laser.inc", "    ld (ix+sprite_yvel),hl\n    ret\n; end player_spawn_laser",
               "    ld (ix+sprite_yvel),hl\n    ld a,1\n    ld (mus_laser),a\n    ret\n; end player_spawn_laser")
        inject(src / "player_state.inc", "    ld a,player_dying_phase_finale\n",
               "    ld a,1\n    ld (mus_blast),a\n    ld a,player_dying_phase_finale\n")
        fixture = assemble()
        if fixture.count(module) != 1:
            raise ValueError("assembled song did not match the private original exactly once")
        output.mkdir(parents=True, exist_ok=False)
        (output / "nurples.bin").write_bytes(fixture)
        shutil.copyfile(src / "nurples.symbols", output / "nurples.symbols")
        for name in ASSETS:
            (output / name).parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / "tgt" / name, output / name)
        (output / "SHA256SUMS").write_text("".join(
            f"{sha256((output / n).read_bytes()).hexdigest()}  {n}\n"
            for n in ("nurples.bin", "nurples.symbols", *ASSETS)))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    for flag in ("assembler", "amp", "bank", "song", "output"):
        parser.add_argument("--" + flag, type=Path, required=True)
    args = parser.parse_args()
    build(*(p.resolve() for p in (args.assembler, args.amp, args.bank, args.song, args.output)))
