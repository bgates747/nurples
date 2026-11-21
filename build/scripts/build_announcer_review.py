"""Assemble only the isolated VOX-001 audition; never change tgt/ or launch."""
import argparse
from hashlib import sha256
from pathlib import Path
import runpy
import shutil
import subprocess
from tempfile import TemporaryDirectory
import wave

from build_game_over_review import ASSETS, ROOT
from nurples_tools.campaign_audio import prepare_audio, verify_audio

PINS = {
    "game_over": "e11bf968d0ffee84127a7a5b7108b53b953024e01effd910b6aea71420c8d43f",
    "shields_depleted": "6055a05cb7c4353b2fe1084b1e147f25b43330aa2ca41ffd6f7b0b7cb35bf29a",
    "shields_critical": "da6b70badc183a45a18f7331bf1320db9afc112d857b4bea71aa9bcb16241743",
}
HULL = "9f426e15a7540dde315a3962111515d7115a7f4257a4b653ee53fe877088b868"
CONTROL = "c18f8afb1043f46fbe2dd54faf78514cb9f15d3fa639d9ef7db49ccb28bb570c"


def audio_container(samples):
    writer = runpy.run_path(str(ROOT.parent / "agon-utils/examples/agnb/audio/scripts/build_audio_container.py"))
    records = []
    # FB00 belonged to the retired welcome clip. Preserve the other cue IDs.
    for i, (name, expected) in enumerate(PINS.items(), start=1):
        with wave.open(str(samples / (name + ".wav")), "rb") as wav:
            if (wav.getnchannels(), wav.getsampwidth(), wav.getframerate()) != (1, 1, 16384):
                raise ValueError(f"unexpected preview format: {name}")
            pcm = wav.readframes(wav.getnframes())
        if sha256(pcm.translate(bytes(i ^ 128 for i in range(256)))).hexdigest() != expected:
            raise ValueError(f"not the approved preview: {name}")
        records.append(writer["AudioRecord"](name, 0xFB00 + i, 16384, pcm))
    return writer["build_container"](records)


def hook(src, file, old, new):
    path = src / file
    s = path.read_text()
    if s.count(old) != 1:
        raise ValueError(f"missing or ambiguous hook: {file}: {old}")
    path.write_text(s.replace(old, new))


def build(assembler, samples, out):
    voice = audio_container(samples)
    hull = (ROOT.parent / "AgonArcade/.work/tasks/ARCADE-SFX-001/clang-r04/hull-ch8-10.vdu").read_bytes()
    if sha256(hull).hexdigest() != HULL:
        raise ValueError("wrong impact recipe")
    with TemporaryDirectory() as tmp:
        src = Path(tmp) / "asm"
        shutil.copytree(ROOT / "src/asm", src)
        modules = prepare_audio(ROOT, src)

        def assemble():
            subprocess.run([assembler, "-s", "nurples.asm", "n.bin"], cwd=src, check=True)
            return (src / "n.bin").read_bytes()

        if sha256(assemble()).hexdigest() != CONTROL:
            raise ValueError("ordinary campaign no longer matches the approved control")
        shutil.copyfile(ROOT / "tests/asm/announcer_review.inc", src / "vox.inc")
        (src / "hull.vdu").write_bytes(hull)
        for file, old, new in [
            ("state_game_init.inc", "    call load_game_images\n    jp nz,@agnb_load_error",
             "    call load_game_images\n    jp nz,@agnb_load_error\n"
             "    ld de,vox_filename\n    call agnb_load_audio\n    jp nz,@agnb_load_error"),
            ("campaign_audio.inc", "mus_process_init:\n", "mus_process_init:\n    call vox_reset\n"),
            ("campaign_audio.inc", "mus_audio_open:\n", "mus_audio_open:\n    call vox_open\n"),
            ("campaign_audio.inc", "    call ay_shutdown", "    call vox_close\n    call ay_shutdown"),
            ("campaign_audio.inc", "@load:\n", "@load:\n    ld hl,(game_state)\n    ld de,game_over\n"
             "    or a\n    sbc hl,de\n    call z,vox_game_over\n"),
            ("campaign_audio.inc", "mus_effects:\n", "mus_effects:\n    call vox_frame\n"),
            ("campaign_audio.inc", '    include "mus_tracks.inc"', '    include "vox.inc"\n    include "mus_tracks.inc"'),
            ("player_shields.inc", "@have_shields:\n", "@have_shields:\n    call vox_contact\n"),
            ("explosion.inc", "    call mus_queue_blast\n    .endif\n    ld hl,explosion_defaults",
             "    call vox_queue_ping\n    .endif\nvox_contact_explosion:\n    ld hl,explosion_defaults"),
            *[(f, "    call update_shields\n    jp spawn_explosion",
               "    call update_shields\n    jp vox_contact_explosion")
              for f in ("enemy_fireball.inc", "enemy_seeker.inc")],
            ("tile_electrode.inc", "    call update_shields\n    jp kill_electrode",
             "    call update_shields\n    jp vox_contact_electrode"),
            ("tile_crater.inc", "    call mus_queue_blast", "    call vox_crater_sound"),
        ]:
            hook(src, file, old, new)
        binary = assemble()
        verify_audio(binary, modules)
        if binary.count(hull) != 1:
            raise ValueError("clang missing or duplicated")
        out.mkdir(parents=True, exist_ok=False)
        (out / "voice.agnb").write_bytes(voice)
        (out / "nurples.bin").write_bytes(binary)
        shutil.copyfile(src / "nurples.symbols", out / "nurples.symbols")
        for name in ASSETS:
            (out / name).parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / "tgt" / name, out / name)
        (out / "SHA256SUMS").write_text("".join(
            f"{sha256((out / name).read_bytes()).hexdigest()}  {name}\n"
            for name in ("nurples.bin", "nurples.symbols", *ASSETS, "voice.agnb")))


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__)
    for name in ("assembler", "samples", "output"):
        ap.add_argument("--" + name, type=Path, required=True)
    args = ap.parse_args()
    build(args.assembler.resolve(strict=True), args.samples.resolve(strict=True), args.output)
