"""Build three private arms in fresh directories; never publish to tgt/."""
from hashlib import sha256
from pathlib import Path
import shutil
import subprocess
from tempfile import TemporaryDirectory

from nurples_tools.campaign_audio import prepare_audio, verify_audio

ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / ".build/mus001-prt-r01"
PRODUCTION = "a607169c4431b54b0c34b28cb6e4ba827fc9aa7198d4cb21e9af1fc1c1d0457e"


def edit(src, filename, old, new):
    file = src / filename
    data = file.read_text()
    if data.count(old) != 1:
        raise ValueError(f"{filename}: fixture anchor changed: {old!r}")
    file.write_text(data.replace(old, new))


def configure(src, candidate, small_reads=False, whole=False):
    shutil.copyfile(ROOT / "docs/tasks/MUS-001/prt-loader.inc", src / "prt-loader.inc")
    with (src / "mus_payload.inc").open("a") as f:
        f.write(f'lp_music: equ {int(candidate)}\nlp_whole: equ {int(whole)}\n'
                '    include "prt-loader.inc"\n')
    if small_reads:
        edit(src, "agnb_api.inc", "agnb_io_buffer_size:       equ 8192",
             "agnb_io_buffer_size:       equ 1024")
        edit(src, "prt-loader.inc", '"LOADPRT.BIN"', '"LOAD1KW.BIN"' if whole else '"LOAD1K.BIN"')
        edit(src, "prt-loader.inc", '"LPR1"', '"LPR4"' if whole else '"LPR3"')
    init = "state_game_init.inc"
    edit(src, init, "    call load_ui_images", "    call lp_begin\n    call load_ui_images")
    edit(src, init, "    call game_start_config_begin",
         "    call lp_finish\n    call game_start_config_begin")
    edit(src, "nurples.asm", "    call playfield_display_report",
         "    call playfield_display_report\n    call lp_save")
    if candidate:
        edit(src, "agnb_api.inc", "    call vdu_load_buffer", "    call lp_append")
        edit(src, "audio/agonAMP/libsupport.asm", "\trst.lil\t18h",
             "\tcall lp_flush ; Nurples experiment: RAM capture during preparation only")
        edit(src, "campaign_audio.inc", "mus_tick:\n    call pt3_play\n    call ay_update",
             "mus_tick:\n    call lp_frame")
        edit(src, init, "@agnb_load_error:\n", "@agnb_load_error:\n    push af\n"
             "    call lp_stop_timer\n    call mus_stop\n    pop af\n")
        edit(src, "nurples.asm", "main_end:\n", "main_end:\n    call lp_stop_timer\n")


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
