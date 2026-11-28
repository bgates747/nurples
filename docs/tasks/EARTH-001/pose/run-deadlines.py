"""One R03-matched direct deadline run. Invoke from the commissioned bench checkout."""
import importlib.util
from pathlib import Path
import re
import subprocess
import sys

here = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("r03", here / "run-r03.py")
suite = importlib.util.module_from_spec(spec)
spec.loader.exec_module(suite)
r = suite.r
from nurples_tools.earth_deadlines import summarize_deadlines

r.OUT = r.ROOT / ".build/earth-deadlines-r01/bench-01"
job = here / "olimex-deadlines-r01.json"
pin = "902794dae2cded398af630020f847ad676d34ea0724b435d73c6f60464b8cfc8"
reference = r.ROOT / ".build/earth-pose-r03/bench-uart-01"
read, record, need = suite.read, suite.record, suite.need
need(Path.cwd().resolve() == r.BENCH.resolve() and
     subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip() == r.REV, "Checkout changed")
ready = read(r.OUT / "operator-ready.json")
need(ready["ready"] and ready["operator_thread"] == r.OWNER and ready["revision"] == r.REV, "Not ready")
resume = sys.argv[1:] == ["--after-smith-reset"]
need(resume or len(sys.argv) == 1, "Unknown argument")
if resume:
    suite.joined("stage")
    need(read(r.OUT / "suite.log.terminal.json")["returncode"] == 1
         and (r.OUT / "reset-direct/before.json").is_file()
         and not (r.OUT / "reset-direct/after.json").exists()
         and not (r.OUT / "run-direct").exists(), "Not the recorded pre-launch reset failure")
for phase in (("run-direct", "handback") if resume else ("stage", "run-direct", "handback")):
    r.custody()
    kit, config = read(job), read(r.CONFIG)
    need(r.production(config, kit) == read(reference / "r03-production.json"), "Control firmware mismatch")
    dest = r.OUT / phase
    need(not dest.exists(), "Refusing replay: " + phase)
    if phase == "run-direct" and not resume:
        r.reset("direct", config, kit)
    subprocess.run([sys.executable, str(r.RUNNER), "--job", str(job), "--sha256", pin,
                    "--config", str(r.CONFIG), "--out", str(r.OUT), "--operator", r.OWNER, phase], check=True)
    suite.joined(phase)
    if phase == "run-direct":
        raw = (dest / "results/DEADLINE.BIN").read_bytes()
        receipt = read(dest / "sd-collect-01.receipt.json")
        need(receipt["bytes"] == len(raw) and receipt["payload_carrier"] == "uart", "Collection mismatch")
        stats = summarize_deadlines(raw, (reference / "run-native-direct/results/POSE.BIN").read_bytes(),
                                    list_dense_misses=True)
        serial = (dest / "results/vdp-usb.bin").read_bytes()
        need(re.findall(rb"Skipped / VBlanks:\s*(\d+)\s*/\s*(\d+)", serial) ==
             [tuple(str(stats["all"][k]).encode() for k in ("skipped", "vblanks"))], "Serial mismatch")
        record(dest / "validated.json", stats)
    elif phase == "handback":
        need(read(dest / "handback.json")["working_legacy_cli"], "Handback failed")
