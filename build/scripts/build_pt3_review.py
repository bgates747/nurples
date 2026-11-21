"""Build the private MUS-001 FAE payload; never deploy, publish or launch it."""
import argparse
from dataclasses import replace
from hashlib import sha256
from pathlib import Path
import runpy
import shutil
import subprocess
from tempfile import TemporaryDirectory
from nurples_tools.campaign_audio import prepare_audio, verify_audio

from build_game_over_review import ASSETS, ROOT

AMP = "91ef401bcb7f636e7593ad1ba176508344deebb9"
BANK = "962c1868b3a5485c3021af1ad908181fb92f34fcb95004b081a76fb2d31759c1"


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


def build(assembler, amp, bank, song, output, *, campaign=False):
    with TemporaryDirectory(prefix="mus001-") as temporary:
        src = Path(temporary) / "asm"
        shutil.copytree(ROOT / "src/asm", src)
        # Keep the audition's explicit donor provenance checks.
        for name in ("libsupport.asm", "pt3play.asm", "pt3tab.inc", "ayvdp.asm"):
            upstream = subprocess.check_output(["git", "-C", amp, "show", f"{AMP}:lib/{name}"])
            if upstream != (src / "audio/agonAMP" / name).read_bytes():
                raise ValueError(f"unexpected player source: {name}")
        if effect_programs(bank) != (src / "audio/mus_fx.inc").read_text().split("\n", 1)[1]:
            raise ValueError("soundboard programs differ from the pinned donor")
        modules = prepare_audio(ROOT, src, directory=song if campaign else None,
                                single=None if campaign else song)
        subprocess.run([assembler, "-s", "nurples.asm", "n.bin"], cwd=src, check=True)
        fixture = (src / "n.bin").read_bytes()
        verify_audio(fixture, modules)
        output.mkdir(parents=True, exist_ok=False)
        (output / "nurples.bin").write_bytes(fixture)
        shutil.copyfile(src / "nurples.symbols", output / "nurples.symbols")
        assets = (*ASSETS, "voice.agnb") if campaign else ASSETS
        for name in assets:
            (output / name).parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / "tgt" / name, output / name)
        (output / "SHA256SUMS").write_text("".join(
            f"{sha256((output / n).read_bytes()).hexdigest()}  {n}\n"
            for n in ("nurples.bin", "nurples.symbols", *assets)))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    for flag in ("assembler", "amp", "bank", "output"):
        parser.add_argument("--" + flag, type=Path, required=True)
    tracks = parser.add_mutually_exclusive_group(required=True)
    tracks.add_argument("--song", type=Path, help="R02 single-track audition")
    tracks.add_argument("--campaign-songs", type=Path, help="private directory of verified modules")
    args = parser.parse_args()
    build(*(p.resolve() for p in (args.assembler, args.amp, args.bank,
                                  args.song or args.campaign_songs, args.output)),
          campaign=args.campaign_songs is not None)
