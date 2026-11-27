"""One private Eirene RC3 TX-IRQ compatibility run. Not an Olimex benchmark."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import signal
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[4]
sys.path.insert(0, str(ROOT))
from nurples_tools.earth_pose import pose_summary

BUILD = ROOT / ".build/earth-pose-r01"
PROFILE = BUILD / "pi-flexe-async-r01"
REF = ROOT.parent / "AgonJukebox/.emulator/uart001-profiles/candidate"
TOOLS = ROOT.parent / "AgonJukebox/.emulator/uart001-tools/mtools/usr/bin"
RELEASE = ROOT.parent / "eirene-mos/build/rc003/packages/3.1.0_uart0_rc_3"
ENV = ROOT.parent / "agon-dev-env"
PINS = {
    "flexe-agon-emulator": "41cd8f70afd473f18d65b25403a52f0bf6013a93e8b32c4ec283acff947511aa",
    "flexe-agon-emulator.bin": "ffa4e66ac661896952c020cc1e5cad3de719688375adde8889cd82217d5fec98",
    "flexe-vdp.so": "c1acfd2201ead6fed425d4affe3d396885e16485c33092ee176019d471eecbe4",
    "esp32-firmware.bin": "201ad8ab4af807c261cb92634286b2fbaedcf3879f140e820b60ab54cf707e2a",
    "esp32-firmware.elf": "14715f9f3d88d891dfddbf77c1423defffdb2351412f44fdce140b6d28f59470",
    "mos_platform.bin": "d7a51147f3381cd68d56f3fbe4605434a897dfae1b049269afeba09ec2b7eb50",
    "mos_platform.map": "48dae5890af7b49fe6bcb535a20e9b6432069c8d84666e3d0e8e7e1070282be9",
}
ASSETS = {
    "earth.agnb": "abf3e8355be3e90eca77c8dd22606d26626e8f64c6e0204743325c9a2cae35ee",
    "game.agnb": "30edadee99aafb65487657f5eaee6c4b6b4d7bb3bf047b8fd9e5fb677063318f",
    "ui.agnb": "d0024003369c8674a3e7989efa6d45c264ab2a455e6da628b16b2afd9594d41a",
    "voice.agnb": "3ba577d5053ce9ef5579ea6c2807f68a5e4523e8ded06684a4005e82707715fe",
    "fonts/Lat38-VGA8_8x8.font": "23321abb10437db6b8bc8f21907268b46615d409b38975f6a195eb9643065a69",
}
FAT = PROFILE / "sdcard.img"


def verify(path, expected):
    if hashlib.sha256(path.read_bytes()).hexdigest() != expected:
        raise ValueError(f"pin mismatch: {path}")
    return path


def mtool(name, *args):
    return subprocess.run([TOOLS / name, "-i", FAT, *args], check=True, timeout=20)


def prepare():
    PROFILE.mkdir(mode=0o700, exist_ok=False)
    (PROFILE / "sdcard").mkdir()
    for name, pin in PINS.items():
        src = (RELEASE / name.replace("mos_platform", "mos-rc3")
               if name.startswith("mos_platform") else REF / name)
        shutil.copy2(verify(src, pin), PROFILE / name)
    (PROFILE / ".mos-release").write_text("Eirene RC3 (97311c4)\n")
    (PROFILE / ".bespoke-vdp-profile").write_text("flexe-vdp.so\n")
    lines = {k: f"{v}  {k}\n" for k, v in PINS.items()}
    (PROFILE / ".mos-release.sha256").write_text(lines["mos_platform.bin"] + lines["mos_platform.map"])
    (PROFILE / ".runtime.sha256").write_text("".join(lines.values()))
    # -C creates a new image in the just-created private directory (never a device).
    if FAT.exists() or FAT.is_symlink() or not PROFILE.is_dir() or PROFILE.is_symlink():
        raise ValueError("fresh private FAT path required")
    subprocess.run(["/usr/sbin/mkfs.fat", "-C", "-F", "16", FAT, "32768"], check=True, timeout=20)
    for folder in ("test", "test/pose", "test/earth-ab-vl1", "test/earth-ab-vl1/control",
                   "test/earth-ab-vl1/control/fonts"):
        mtool("mmd", "::/" + folder)
    folder = "test/earth-ab-vl1/control/"
    for name, pin in ASSETS.items():
        src = BUILD / "pi-stock-r01/direct/sdcard" / folder / name
        mtool("mcopy", verify(src, pin), "::/" + folder + name)
    pin = next(f["agon"]["sha256"] for f in json.loads(
        Path(__file__).with_name("olimex-pose-r01.json").read_text())["fixtures"] if f["id"] == "async")
    mtool("mcopy", verify(BUILD / "async/nurples.bin", pin), "::/test/pose/nurples.bin")
    startup = PROFILE / "autoexec.txt"
    startup.write_bytes(b"SET KEYBOARD 1\r\nCD /test/pose\r\nLOAD nurples.bin &040000\r\nRUN &040000\r\n")
    mtool("mcopy", startup, "::/autoexec.txt")


def run(lease):
    subprocess.run([ENV / ".venv/bin/python", ENV / "cross-agent/pi5-workloads/pi5_lease.py",
                    "assert-owner", "--owner", "nurples", "--lease", lease], check=True, timeout=15,
                   stdout=subprocess.DEVNULL)
    if (PROFILE / "last-run").exists():
        raise RuntimeError("fresh trial required")
    start = time.monotonic()
    seen = b""
    pos = 0
    wire = PROFILE / "last-run/mos-to-vdp.bin"
    log = PROFILE / "frontend.log"
    with log.open("xb") as out:
        guest = subprocess.Popen(["./flexe-agon-emulator", "--precise-interrupts"], cwd=PROFILE,
                                 env=dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy",
                                          AGON_FLEXE_ENGINE="jit"), stdout=out, stderr=subprocess.STDOUT,
                                 start_new_session=True)
        try:
            while time.monotonic() - start < 600:
                if guest.poll() is not None:
                    raise RuntimeError(f"frontend exited before report: {guest.returncode}")
                available = int(Path("/proc/meminfo").read_text().split("MemAvailable:")[1].split()[0])
                temp = int(Path("/sys/class/thermal/thermal_zone0/temp").read_text())
                if available < 512 * 1024 or temp >= 80000 or log.stat().st_size > 64 * 1024**2:
                    raise RuntimeError("resource limit")
                counters = PROFILE / "last-run/uart.json"
                if counters.exists():
                    try:
                        current = json.loads(counters.read_bytes())
                    except json.JSONDecodeError:
                        # The running emulator truncates and rewrites this snapshot.
                        current = {}
                    if current.get("rx_overruns", 0) or current.get("tx_drops", 0):
                        raise RuntimeError(f"UART loss before end of level: {current}")
                if wire.exists():
                    with wire.open("rb") as stream:
                        stream.seek(pos)
                        fragment = seen + stream.read()
                        pos = stream.tell()
                    if b"EARTH AB FAILED" in fragment:
                        raise RuntimeError("guest report failure")
                    if b"EARTH AB SAVED" in fragment:
                        break
                    seen = fragment[-32:]
                time.sleep(1)
            else:
                raise TimeoutError("no Earth report in 600 s")
        finally:
            if guest.poll() is None:
                # SDL handles TERM; let the frontend flush, and the wrapper do its normal cleanup.
                frontend = PROFILE / "flexe-agon-emulator.bin"
                for proc in Path("/proc").iterdir():
                    if not proc.name.isdigit():
                        continue
                    try:
                        pid = int(proc.name)
                        if os.getpgid(pid) == guest.pid and (proc / "exe").resolve() == frontend:
                            os.kill(pid, signal.SIGTERM)
                    except (FileNotFoundError, PermissionError, ProcessLookupError):
                        continue
                try:
                    guest.wait(timeout=8)
                except subprocess.TimeoutExpired:
                    os.killpg(guest.pid, signal.SIGKILL)
                    guest.wait(timeout=5)
    mtool("mcopy", "::/test/pose/POSE.BIN", PROFILE / "POSE.BIN")
    result = pose_summary((PROFILE / "POSE.BIN").read_bytes(), "async")
    uart = json.loads((PROFILE / "last-run/uart.json").read_text())
    if uart["tx_irqs"] <= 0 or uart["tx_drops"] or uart["rx_overruns"]:
        raise RuntimeError(f"TX IRQ delivery / receiver failure: {uart}")
    result.update(uart=uart, elapsed_s=round(time.monotonic()-start, 2), frontend_exit=guest.returncode)
    (PROFILE / "result.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result))


if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("action", choices=("prepare", "run"))
    p.add_argument("--lease")
    a = p.parse_args()
    if a.action == "prepare":
        prepare()
    elif not a.lease:
        p.error("Pi manager admission and START required")
    else:
        run(a.lease)
