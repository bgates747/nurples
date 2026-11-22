"""Pack the three approved announcer WAVs using the canonical AGNB 0.2 writer."""
from hashlib import sha256
import os
from pathlib import Path
import runpy
import wave


PINS = {
    "game_over": "a12beb667cbb89d5794b5d233c3611c4a89dab98ea7c6d3f7a3b484ebed15538",
    "shields_depleted": "0a40dddc6acb84c9062ff696094803494a1770b66b850558382783c556df8e10",
    "shields_critical": "481c020036d3eccd2f6bc0aca09cba85e863e1b8c61d38fa66ff98bdbdbe4217",
}


def audio_container(samples: Path) -> bytes:
    utils = Path(os.environ.get("AGON_UTILS_ROOT", Path.home() / "Agon/mystuff/agon-utils"))
    writer = runpy.run_path(str(utils / "examples/agnb/audio/scripts/build_audio_container.py"))
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
