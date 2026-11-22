"""Build three private arms in fresh directories; never publish to tgt/."""
from hashlib import sha256
from pathlib import Path
import shutil
import subprocess
from tempfile import TemporaryDirectory

from nurples_tools.campaign_audio import prepare_audio, verify_audio
from nurples_tools.loading_music import configure

ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / ".build/mus001-prt-r01"
PRODUCTION = "a607169c4431b54b0c34b28cb6e4ba827fc9aa7198d4cb21e9af1fc1c1d0457e"




def build():
    for arm in ("ordinary", "control", "candidate", "small-reads", "whole"):
        with TemporaryDirectory() as temp:
            src = Path(temp) / "asm"
            shutil.copytree(ROOT / "src/asm", src)
            modules = prepare_audio(ROOT, src)
            if arm != "ordinary":
                configure(src, arm != "control", arm in ("small-reads", "whole"), arm == "whole")
            assembler = shutil.which("ez80asm") or ROOT / ".build/game001jk-olimex-r01/ez80asm"
            subprocess.run([assembler, "-l", "-s",
                            "nurples.asm", "nurples.bin"], cwd=src, check=True, timeout=60)
            binary = (src / "nurples.bin").read_bytes()
            verify_audio(binary, modules)
            if arm == "ordinary" and sha256(binary).hexdigest() != PRODUCTION:
                raise ValueError("ordinary binary no longer matches the rollback")
            if arm == "candidate" and sha256(binary).hexdigest() != (
                    "e24b6e9dd15a9568c21989195732be8f7a8448c939489caaafbf16295cf3920c"):
                raise ValueError("R01 timing control changed")
            dst = (ROOT / ".build/mus001-prt-r03" if arm == "small-reads" else OUTPUT) / arm
            if arm == "small-reads" and sha256(binary).hexdigest() != (
                    "519cd653e7c5555d170169f573b4a2873e18b7866b7bf2979ed140372a9597c6"):
                raise ValueError("R03 timing control changed")
            if arm == "whole":
                dst = ROOT / ".build/mus001-prt-r04/whole"
            dst.mkdir(parents=True, exist_ok=True)
            files = ("nurples.bin", "nurples.symbols", "ui.agnb", "game.agnb",
                     "voice.agnb", "fonts/Lat38-VGA8_8x8.font")
            for name in files:
                dest = dst / name
                dest.parent.mkdir(exist_ok=True)
                shutil.copy2((src if name.startswith("nurples.") else ROOT / "tgt") / name, dest)
            (dst / "SHA256SUMS").write_text("".join(
                f"{sha256((dst / name).read_bytes()).hexdigest()}  {name}\n" for name in files))


if __name__ == "__main__":
    build()
