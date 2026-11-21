"""Render the isolated VOX-001 audition, with no desktop or game playback."""

from hashlib import sha256
import io
import json
from pathlib import Path
import subprocess
import sys
import wave
import zipfile


RATE = 16384
CUES = {
    "welcome": "Welcome to Attack of the Purple Nurples.",
    "game_over": "Game over.",
    "shields_depleted": "Shields depleted.",
    "shields_critical": "Shields critical.",
}

# The reference voice's existing interpreter synthesizes to stdout only.
# Pass text on stdin, separately from the remote shell command.
VOICE = '''/home/smith/.local/share/oryx-voice-alert/venv/bin/python -c '
import sys
from importlib.metadata import version
from gtts import gTTS
if version("gTTS") != "2.5.4": raise RuntimeError("unexpected voice client")
gTTS(text=sys.stdin.read(), lang="en", tld="co.uk", slow=False,
     timeout=(5, 20)).write_to_fp(sys.stdout.buffer)
' '''


def render(out: Path) -> None:
    out.mkdir(parents=True, exist_ok=False)
    metadata = {"encoding": "signed 8-bit PCM, mono, headerless", "rate_hz": RATE,
                "voice": "gTTS 2.5.4 en/co.uk, normal speed", "gain_db": -2, "clips": []}
    flip_sign = bytes(i ^ 128 for i in range(256))
    for cue, words in CUES.items():
        mp3 = subprocess.run(
            ["ssh", "-T", "-o", "BatchMode=yes", "-o", "ConnectTimeout=5", "oryx-linuxlite", VOICE],
            input=words.encode(), stdout=subprocess.PIPE, check=True, timeout=50,
        ).stdout
        pcm = subprocess.run(
            ["ffmpeg", "-v", "error", "-f", "mp3", "-i", "pipe:0", "-af", "volume=-2dB",
             "-ac", "1", "-ar", str(RATE), "-c:a", "pcm_s8", "-f", "s8", "pipe:1"],
            input=mp3, stdout=subprocess.PIPE, check=True, timeout=15,
        ).stdout
        if not RATE // 4 <= len(pcm) <= RATE * 8 or len(set(pcm)) < 24:
            raise ValueError(f"{cue}: empty, implausible or silent output")
        preview = io.BytesIO()
        with wave.open(preview, "wb") as w:
            w.setparams((1, 1, RATE, 0, "NONE", "not compressed"))
            w.writeframes(pcm.translate(flip_sign))  # WAV uses unsigned 8-bit PCM
        wav = preview.getvalue()
        with wave.open(io.BytesIO(wav), "rb") as w:
            if (w.getnchannels(), w.getsampwidth(), w.getframerate(), w.getnframes()) != (
                1, 1, RATE, len(pcm)
            ) or w.readframes(len(pcm)).translate(flip_sign) != pcm:
                raise ValueError(f"{cue}: WAV did not round-trip")
        (out / f"{cue}.raw").write_bytes(pcm)
        (out / f"{cue}.wav").write_bytes(wav)
        metadata["clips"].append({"cue": cue, "text": words, "bytes": len(pcm),
                                  "raw_sha256": sha256(pcm).hexdigest(),
                                  "wav_sha256": sha256(wav).hexdigest()})
        print(f"{cue}: {len(pcm)} bytes, {len(pcm)/RATE:.2f} s", flush=True)
    (out / "samples.json").write_text(json.dumps(metadata, indent=2) + "\n")
    with zipfile.ZipFile(out / "agon-announcer-pcm.zip", "x", zipfile.ZIP_DEFLATED) as z:
        for name in [*(f"{cue}.raw" for cue in CUES), "samples.json"]:
            z.write(out / name, name)


if __name__ == "__main__":
    render(Path(sys.argv[1]))
