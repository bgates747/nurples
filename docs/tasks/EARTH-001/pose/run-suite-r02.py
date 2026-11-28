"""Earth R02 production-only comparison. Preflight is offline; run requires live FIFO custody."""
import argparse
import fcntl
import re
import subprocess
import sys
from pathlib import Path
from hashlib import sha256

ROOT = Path("/Agon/mystuff/nurples")
BENCH = ROOT.parent / "olimex-bench"
EXT = ROOT.parent / "agon-extender"
JOB = Path(__file__).with_name("olimex-suite-r02.json").resolve()
PIN = "6afb61ccd2da22ee1f448fbd809aa652acbca731dc1a0908745a6cb314f40e62"
CONFIG = EXT / "agents/upstream-002/operator-r01/bench.local.json"
REV = "e141613c1d5989dcc049da43ebe59b0f32555021"
OWNER = "019fc0a7-f33a-7160-a663-427af9394e01"
FIFO = EXT / "agents/olimex-scheduling/rehearsal-r02-fifo.json"
OUT = ROOT / ".build/earth-pose-r02/bench-uart-01"
RUNNER = BENCH / "scripts/olimex_bench.py"
sys.path[:0] = [str(BENCH / "scripts"), str(ROOT)]
import bench_process
import olimex_bench as bench
from reset_agon_usb import pulse
from nurples_tools.pose_compare import describe
from nurples_tools.pose_cost import decode_cost, CAP, SIZE


def require(ok, msg):
    if not ok:
        raise RuntimeError(msg)


def production(config, kit):
    inventory = bench_process.read(config["inventory"])
    required = dict(native_vdp_sha256="1698d5ce5ddb441279feb07198753ce99b242552c0d1245c71a00f67336422b2",
                    emos_rom_sha256="9b90b5d94ec6888d85ef1fa1acf23a82b2e0eb1f179b75f549863fbd72343673",
                    p4_app_sha256="539dc7176f079bc6c298c48352b7e94c54db654fb6fdbb9c42897370fb19834b",
                    startup_sha256=kit["preserve"]["/autoexec.txt"],
                    listener_sha256=kit["preserve"]["/emos/sdserve.bin"])
    require(not inventory["pending_operation"] and inventory["ready_claim"]
            and all(inventory["components"].get(k) == v for k, v in required.items()),
            "Production handoff not established; do not stage or flash")
    return required


def custody():
    require(bench_process.read(FIFO)["active"] == OWNER, "No live Nurples reservation")


def reset(arm, config, kit):
    with Path(config["operation_lock"]).open("a") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        custody()
        production(config, kit)
        folder = OUT / ("reset-" + arm)
        folder.mkdir()
        target = bench.Bench(config, kit, folder)
        target.lock = lock
        target.validate_ports()
        target.idle()
        before = target.keyboard()["boot"]
        bench_process.record(folder / "before.json", {"boot": before})
        pulse(target.reset_config)
        bench_process.record(folder / "after.json", bench.wait_boot(target.url, before, 60))


def validate(folder, arm):
    results = folder / "results"
    cost = arm.startswith("cost-")
    name = arm.removeprefix("cost-")
    raw = (results / ("POSECOST.BIN" if cost else "POSE.BIN")).read_bytes()
    transfer = bench_process.read(folder / "sd-collect-01.receipt.json")
    require(transfer["bytes"] == len(raw) and transfer["payload_carrier"] == "uart",
            "Report does not match UART collection receipt")
    scopes = decode_cost(raw, name) if cost else None
    eps = b"EPS1" + raw[4:33] + raw[38+CAP*SIZE:] if cost else raw
    stats = describe(eps, name)
    serial = (results / "vdp-usb.bin").read_bytes()
    for pattern, expected in [
        (rb"Skipped / VBlanks:\s*(\d+)\s*/\s*(\d+)", (stats["skipped"], stats["vblanks"])),
        (rb"Updates / zero-tick waits:\s*(\d+)\s*/\s*(\d+)",
         (stats["updates"], int.from_bytes(eps[13:16], "little"))),
    ]:
        require(re.findall(pattern, serial) == [tuple(str(x).encode() for x in expected)],
                "Report counters and this invocation's serial witness disagree")
    require(b"Native pose token/kind/value" not in serial, "Native rejection in serial capture")
    for key in ("poses", "pose_sends", "pose_wire_bytes"):
        stats[key+"_per_update"] = stats[key]/stats["updates"]
        stats[key+"_per_second_60hz"] = stats[key]*60/stats["vblanks"]
    return dict(stats=stats, scopes=scopes, source_sha256=sha256(raw).hexdigest())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("preflight", "run"))
    action = parser.parse_args().action
    require(Path.cwd().resolve() == BENCH.resolve(), "Run from the commissioned checkout")
    require(subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip() == REV,
            "Unexpected commissioned runner revision")
    require(sha256(JOB.read_bytes()).hexdigest() == PIN, "Changed R02 packet")
    kit, config = bench_process.read(JOB), bench_process.read(CONFIG)
    argv = [sys.executable, str(RUNNER), "--job", str(JOB), "--sha256", PIN, "--config", str(CONFIG)]
    subprocess.run([*argv, "preflight"], check=True)
    if action == "preflight":
        return
    ready = bench_process.read(OUT / "operator-ready.json")
    require(ready["ready"] and ready["operator_thread"] == OWNER and ready["revision"] == REV,
            "Missing commissioned operator readiness")
    custody()
    pins = production(config, kit)
    bench_process.record(OUT / "r02-production.json", pins)
    phases = ["stage", *("run-"+j["id"] for j in kit["fixtures"]), "handback"]
    for phase in phases:
        custody()
        production(config, kit)
        folder = OUT / phase
        require(not folder.exists(), f"Refusing to repeat phase {phase}")
        if phase.startswith("run-"):
            reset(phase[4:], config, kit)
        subprocess.run([*argv, "--out", str(OUT), "--operator", OWNER, phase], check=True)
        proof = bench_process.read(folder / "complete.json")["proof"]
        require(proof["all_joined"] and proof["actual_returncode"] == 0, f"Unjoined {phase}")
        if phase.startswith("run-"):
            result = validate(folder, phase[4:])
            bench_process.record(folder / "validated.json", result)
            print(phase, result["stats"]["skipped"], "/", result["stats"]["vblanks"], flush=True)
        elif phase == "handback":
            require(bench_process.read(folder/"handback.json")["working_legacy_cli"], "MOS handback failed")


if __name__ == "__main__":
    main()
