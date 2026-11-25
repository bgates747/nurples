"""Prepare private PT3 inputs in the assembler's scratch directory."""
from hashlib import sha256
import json
import os
from pathlib import Path
import tomllib


LEVEL_MUSIC = {
    "../level-design/levels/01/level.tmx": "break2.pt3",
    "../level-design/levels/proving-ground/level.tmx": "acidflsh.pt3",
    "../level-design/levels/modern-dg/level.tmx": "terminator.pt3",
    "../level-design/levels/recovered-z80/level.tmx": "bugs2_2.pt3",
}


def prepare_audio(root: Path, staged: Path, *, directory: Path | None = None,
                  single: Path | None = None) -> list[bytes]:
    """The ordinary game follows campaign order; single is the historical audition."""
    pins = {p["filename"]: p for p in
            json.loads((root / "docs/tasks/MUS-001/samples.json").read_text())}
    campaign = single is None
    if single is None:
        directory = directory or Path(os.environ.get("NURPLES_MUSIC_DIR",
                                                     root / ".build/mus001/audition"))
        levels = tomllib.loads((root / "maps/campaign.toml").read_text())["levels"]
        tracks = [directory / LEVEL_MUSIC[level["source"]] for level in levels]
        title = directory / "01_loadtune.pt3"
    else:
        tracks, title = [single], None
    labels = {}
    modules = []
    for track in dict.fromkeys([*tracks, *([title] if title else [])]):
        pin = pins[track.name]
        if not track.is_file():
            raise FileNotFoundError(f"Missing private music: {track}. Set NURPLES_MUSIC_DIR "
                                    "to the directory containing the pinned PT3 files.")
        data = track.read_bytes()
        if len(data) != pin["bytes"] or sha256(data).hexdigest() != pin["sha256"]:
            raise ValueError(f"Private music does not match the approved recording: {track.name}")
        labels[track.name] = f"mus_track_{len(modules)}"
        modules.append(data)
        (staged / track.name).write_bytes(data)
    table = ("mus_tracks:\n" + "".join(f"    dl {labels[t.name]}\n" for t in tracks)
             if campaign else "")
    (staged / "mus_tracks.inc").write_text(table + "mus_song:\n" + "".join(
        f'{label}:\n    incbin "{name}"\n' for name, label in labels.items()) +
        "mus_song_end:\n" + (f"mus_loadtune: equ {labels[title.name]}\n" if title else ""))
    (staged / "mus_config.inc").write_text(
        f"mus_enabled: equ 1\nmus_campaign: equ {int(campaign)}\n"
        f"mus_single: equ {int(not campaign)}\nmus_level_count: equ {len(tracks)}\n"
        "mus_all50: equ 0\nmus_combat_cues: equ 0\n")
    (staged / "mus_payload.inc").write_text('    include "campaign_audio.inc"\n')
    return modules


def verify_audio(binary: bytes, modules: list[bytes]) -> None:
    if any(binary.count(module) != 1 for module in modules):
        raise ValueError("Each distinct pinned PT3 must appear exactly once in the executable")
