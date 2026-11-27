"""Private ARM64 stock FAE profiles. No writes to shared runtimes or Oryx."""
import argparse
import hashlib
import json
import os
import shutil
import signal
import subprocess
import sys
import time
from pathlib import Path


ROOT = Path(__file__).resolve().parents[4]
sys.path.insert(0, str(ROOT))
from nurples_tools.earth_pose import pose_summary

BUILD = ROOT / ".build/earth-pose-r01"
REF = Path("/Agon/mystuff/AgonJukebox/.emulator/stock-analyzer/stock")
ENV = Path("/Agon/mystuff/agon-dev-env")
ARMS = ("direct", "sync", "golem")  # EMOS extension deliberately not called on Arthur
PINS = {
    "fab-agon-emulator": "e822acb348e1be281dce6a80128d3924651f41157b407088b23ea73a4eba728c",
    "fab-agon-emulator.bin": "c63b5a45b9eec9b989bdde73af6d12add92887e3f4e6ac8ffe9291d98979fb66",
    "firmware/vdp_platform.so": "5f9de3ebe7953fdfda4e7668a32c60feae4277d3eb26e1b1e4ab84b4101edae0",
    "mos_platform.bin": "d564243283972690933a4554296ad6202ca4ef54572279533a942960846bebae",
    "mos_platform.map": "d69e60bbce61a7b4b3eef318ba395f11c4e1a5b585755755113992dc94edcb86",
}


def digest(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def prepare(profile, arm):
    profile.mkdir(parents=True, exist_ok=False)
    (profile/"firmware").mkdir()
    for name, sha in PINS.items():
        if digest(REF/name) != sha:
            raise RuntimeError(f"unexpected stock reference: {name}")
        shutil.copy2(REF/name, profile/name)
    for name in (".mos-release", ".mos-release.sha256"):
        shutil.copy2(REF/name, profile/name)
    program = profile/"sdcard/test/pose"
    program.mkdir(parents=True)
    manifest = "olimex-golem-r01.json" if arm == "golem" else "olimex-pose-r01.json"
    jobs = json.loads((Path(__file__).parent/manifest).read_bytes())["fixtures"]
    pin = next(j["agon"] for j in jobs if j["id"] == arm)
    if digest(BUILD/arm/"nurples.bin") != pin["sha256"]:
        raise RuntimeError(f"incorrect {arm} program")
    shutil.copy2(BUILD/arm/"nurples.bin", program/"nurples.bin")
    assets = profile/"sdcard/test/earth-ab-vl1/control"
    (assets/"fonts").mkdir(parents=True)
    for source in ("earth.agnb", "game.agnb", "ui.agnb", "voice.agnb", "fonts/Lat38-VGA8_8x8.font"):
        base = BUILD/arm if source in ("earth.agnb", "game.agnb") else ROOT/"tgt"
        shutil.copy2(base/source, assets/source)
    (profile/"sdcard/autoexec.txt").write_bytes(
        b"SET KEYBOARD 1\r\nCD /test/pose\r\nLOAD nurples.bin &040000\r\nRUN &040000\r\n")


def run(profile, arm, lease, filtered=False):
    subprocess.run([ENV/".venv/bin/python", ENV/"cross-agent/pi5-workloads/pi5_lease.py",
                    "assert-owner", "--owner", "nurples", "--lease", lease],
                   stdout=subprocess.DEVNULL, check=True, timeout=15)
    reports = profile/"sdcard/test/pose"
    def outputs():
        return [p for p in reports.iterdir() if p.name.upper() == "POSE.BIN"]
    if outputs():
        raise RuntimeError("non-fresh guest result")
    start = time.monotonic()
    appenv = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy")
    with (profile/"guest.log").open("xb") as log:
        filter_proc = None
        guest = subprocess.Popen(["./fab-agon-emulator", "--renderer", "sw", "--verbose"],
                                 cwd=profile, env=appenv, stdout=subprocess.PIPE if filtered else log, stderr=subprocess.STDOUT,
                                 start_new_session=True)
        try:
            if filtered:
                # Stock interpreter debug spew can exceed 1 GiB. Retain the
                # *VDP's* whole formation event stream and relevant errors.
                filter_proc = subprocess.Popen(
                    ["rg", "-a", "sprite [0-9]+ (set to frame|- move to)|[Ee]rror|[Ff]ail|[Pp]anic"],
                    stdin=guest.stdout, stdout=log, start_new_session=True)
                guest.stdout.close()
            while time.monotonic()-start < 220:
                result = outputs()
                if len(result) == 1 and result[0].stat().st_size == 33+6*4544:
                    time.sleep(1)  # allow the final close/return to MOS
                    data = result[0].read_bytes()
                    if data[:4] != b"EPS1":
                        raise RuntimeError("not an EPS1 output")
                    summary = pose_summary(data, arm)
                    (profile/"result.json").write_text(json.dumps(summary, indent=2)+"\n")
                    print(f"{arm}: {summary['updates']} updates, {summary['poses']} poses, stock emulator", flush=True)
                    return
                if guest.poll() is not None:
                    raise RuntimeError(f"{arm} guest exited {guest.returncode} without report")
                ram = Path("/proc/meminfo").read_text().split("MemAvailable:")[1].split()[0]
                temp = int(Path("/sys/class/thermal/thermal_zone0/temp").read_text())
                if int(ram) < 512*1024 or temp >= 80000:
                    raise RuntimeError("emulator resource stop")
                if log.tell() > 96*1024*1024:
                    raise RuntimeError("emulator log budget")
                time.sleep(1)
            raise TimeoutError(f"{arm}: 220 s without complete report; retain private SD and log")
        finally:
            if guest.poll() is None:
                os.killpg(guest.pid, signal.SIGTERM)
                try:
                    guest.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    os.killpg(guest.pid, signal.SIGKILL)
                    guest.wait(timeout=5)
            if filter_proc is not None:
                try:
                    filter_proc.wait(timeout=5)  # EOF after guest joined
                except subprocess.TimeoutExpired:
                    filter_proc.terminate()
                    filter_proc.wait(timeout=5)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("prepare", "run"))
    parser.add_argument("--lease")
    parser.add_argument("--golem-filtered", action="store_true")
    args = parser.parse_args()
    if args.action == "run" and not args.lease:
        parser.error("manager emulator admission required")
    for arm in (("golem",) if args.golem_filtered else ARMS):
        p = BUILD/("pi-stock-golem-r02" if args.golem_filtered else "pi-stock-r01")/arm
        if args.action == "prepare":
            prepare(p, arm)
        else:
            run(p, arm, args.lease, args.golem_filtered)
