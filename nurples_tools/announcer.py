"""Pack the three approved announcer WAVs using the canonical AGNB 0.2 writer."""
from hashlib import sha256
import os
from pathlib import Path
import runpy
import wave


PINS = {
    "game_over": "e11bf968d0ffee84127a7a5b7108b53b953024e01effd910b6aea71420c8d43f",
    "shields_depleted": "6055a05cb7c4353b2fe1084b1e147f25b43330aa2ca41ffd6f7b0b7cb35bf29a",
    "shields_critical": "da6b70badc183a45a18f7331bf1320db9afc112d857b4bea71aa9bcb16241743",
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
