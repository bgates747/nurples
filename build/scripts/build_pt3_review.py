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
import tomllib

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


LEVEL_MUSIC = {
    "../tiles/dg/dg0_03.tmx": "terminator.pt3",
    "../tiles/dg/nurples_z80_main.tmx": "bugs2_2.pt3",
}


def campaign_tracks(directory, manifest):
    levels = tomllib.loads(manifest.read_text())["levels"]
    return [directory / LEVEL_MUSIC[level["source"]] for level in levels]


def build(assembler, amp, bank, song, output, *, campaign=False):
    pins = json.loads((ROOT / "docs/tasks/MUS-001/samples.json").read_text())
    tracks = campaign_tracks(song, ROOT / "maps/campaign.toml") if campaign else [song]
    modules = {}
    title = song / "01_loadtune.pt3" if campaign else None
    for track in [*tracks, *([title] if title else [])]:
        pin, = (p for p in pins if p["filename"] == track.name)
        module = track.read_bytes()
        if len(module) != pin["bytes"] or sha256(module).hexdigest() != pin["sha256"]:
            raise ValueError(f"wrong audition module: {track.name}")
        modules[track.name] = module
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
        # Emit one private copy per distinct track, in campaign manifest order.
        labels = {name: f"mus_track_{i}" for i, name in enumerate(modules)}
        for name, data in modules.items():
            (src / name).write_bytes(data)
        table = ("mus_tracks:\n" + "".join(f"    dl {labels[t.name]}\n" for t in tracks)
                 if campaign else "")
        title_alias = f"mus_loadtune: equ {labels[title.name]}\n" if title else ""
        (src / "mus_tracks.inc").write_text(table + "mus_song:\n" + "".join(
            f'{labels[name]}:\n    incbin "{name}"\n' for name in modules) + "mus_song_end:\n" + title_alias)
        (src / "mus_config.inc").write_text(f"mus_campaign: equ {int(campaign)}\n"
                                            f"mus_level_count: equ {len(tracks)}\n")
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
        blast = "    call mus_queue_blast\n" if campaign else "    ld a,1\n    ld (mus_blast),a\n"
        inject(src / "player_state.inc", "    ld a,player_dying_phase_finale\n",
               blast + "    ld a,player_dying_phase_finale\n")
        if campaign:
            # Destructive air collisions and ground kills converge here.
            # Initial crater placement, retreat and decorative debris do not.
            for filename, label in (("explosion.inc", "spawn_explosion"),
                                    ("tile_crater.inc", "transform_active_tile_to_crater")):
                inject(src / filename, f"\n{label}:\n", f"\n{label}:\n" + blast)
            # Shared accepted Start, before the (potentially slow) redraw.
            inject(src / "presentation_flow_states.inc", "\ngame_start_session:\n",
                   "\ngame_start_session:\n    call mus_stop\n")
        fixture = assemble()
        if any(fixture.count(data) != 1 for data in modules.values()):
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
    for flag in ("assembler", "amp", "bank", "output"):
        parser.add_argument("--" + flag, type=Path, required=True)
    tracks = parser.add_mutually_exclusive_group(required=True)
    tracks.add_argument("--song", type=Path, help="R02 single-track audition")
    tracks.add_argument("--campaign-songs", type=Path, help="private directory of verified modules")
    args = parser.parse_args()
    build(*(p.resolve() for p in (args.assembler, args.amp, args.bank,
                                  args.song or args.campaign_songs, args.output)),
          campaign=args.campaign_songs is not None)
