"""Refresh only the four timing-fixture artifacts on the mounted Olimex card."""
import hashlib
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]
HOST = "oryx-linuxlite"
MOUNT = "/run/media/smith/AGONEX"
FILES = ("earth.agnb", "game.agnb", "nurples.bin", "nurples.symbols")


def remote(command):
    return subprocess.check_output(["ssh", "-o", "BatchMode=yes", HOST, command], text=True)


def stage():
    backup = Path(tempfile.mkdtemp(prefix="aligned-card-before-", dir=ROOT / ".build"))
    jobs = []
    for name, package in (("aligned", "control"), ("bombing", "bombing")):
        src = ROOT / ".build" / f"aligned-pace-{package}"
        dst = f"{MOUNT}/mystuff/nurples/{name}"
        guard = (f'test "$(findmnt -nr -o LABEL,FSTYPE --mountpoint {MOUNT})" = "AGONEX vfat"'
                 f' && test "$(realpath -e {dst})" = {dst}')
        for file in FILES:
            if not (src/file).is_file() or (src/file).is_symlink():
                raise ValueError(src/file)
            guard += f" && test -f {dst}/{file} && test ! -L {dst}/{file}"
        remote(guard)
        saved = backup / name
        saved.mkdir()
        for file in (*FILES, "MUSIC50.BIN"):
            if file in FILES or remote(f"test ! -f {dst}/{file} || echo present").strip():
                subprocess.run(["rsync", f"{HOST}:{dst}/{file}", saved], check=True)
        jobs.append((src, dst, guard))
    for src, dst, guard in jobs:
        subprocess.run(["rsync", "--checksum", "--rsync-path", guard + " && rsync",
                        *(str(src/f) for f in FILES), f"{HOST}:{dst}/"], check=True)
        checks = "".join(f"{hashlib.sha256((src/f).read_bytes()).hexdigest()}  {f}\n" for f in FILES)
        remote(f"{guard} && sync -f {dst} && cd {dst} && printf %s {shlex.quote(checks)} | sha256sum -c -")
        print("Verified", dst)
    print("Previous files:", backup)


if __name__ == "__main__":
    stage()
