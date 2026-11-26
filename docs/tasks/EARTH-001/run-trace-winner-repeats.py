"""Repeat the measured winner five times. No game, asset or fixture changes."""
import fcntl
from hashlib import sha256
from pathlib import Path
import runpy
import statistics
import subprocess
import sys
import time

s = runpy.run_path(str(Path(__file__).with_name("run-r30-frame-traces.py")))
a, b, p, ab, fw, prod, check = (s[k] for k in ("a", "b", "p", "ab", "fw", "prod", "check"))
base = a.OUT.parent
last = base / "bench-frame-stock-vdpl-r01"
out = base / "bench-frame-repeat-r01"
worker = [sys.executable, str(Path(__file__).resolve()), "worker"]
pin = s["B"]
image_spec = {"path": "/Agon/mystuff/agon-vdplet/.worktrees/sprite-scanout/build/vdpl062/set01/candidate/firmware.bin",
              "bytes": 1080336, "sha256": pin}
sd_image = "/extender/install/nurples-set01-r01/candidate.bin"


def main():
    check(Path.cwd() == a.BENCH and Path(sys.executable) == a.BENCH / ".venv/bin/python", "runner path")
    check(subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip() == fw.REVISION
          and a.digest(a.JOB) == a.JOB_SHA and a.digest(a.CONFIG) == a.CONFIG_SHA, "checkout/kit changed")
    check(not (a.OUT / "handback").exists(), "original slot handed back")
    scores = {}
    for folder, log in ((s["out"], "frame-traces-direct"), (last, "frame-stock-vdpl-r01")):
        check(p.read(base / (log + ".log.terminal.json"))["returncode"] == 0
              and p.read(folder / "complete.json")["proof"]["all_joined"]
              and p.read(folder / "supervisor-started.json")["lease"] == a.LEASE, "predecessor not joined")
        for name, report in p.read(folder / "matrix.json").items():
            data = (folder / name / "results/EARTHFR.BIN").read_bytes()
            offset = 512 if name.endswith("-aligned") else 72
            verified = s["summarize"](data, offset)
            check(all(report[k] == v for k, v in verified.items()) and report["updates"] == 4544
                  and report["refill"]["updates"] == 895 and report["refill"]["invalid_or_clipped"] == 0
                  and p.read(folder / (name + "-reset") / "admitted.json")["ready"], "invalid screening run")
            scores[name] = report["skipped"]
    check(len(scores) == 8 and [k for k,v in scores.items() if v == min(scores.values())] ==
          ["candidate-aligned"], "no unique expected winner")
    cfg, kit = p.read(a.CONFIG), p.read(a.JOB)
    for j in kit["fixtures"]:
        j.update(agon=s["binary"], results=[j["cwd"] + "/EARTHFR.BIN"])
    b.preflight(kit)
    job = b.fixture_runs(kit)["run-aligned"]
    remote = job["results"][0]
    check(remote == "/test/earth-ab-vl1/aligned/EARTHFR.BIN" and
          job["inputs"][0]["source"]["sha256"] == "13be5a719c649e0852e7148a57cb653da7748752a549e652f07bfe679217292b",
          "unexpected aligned fixture")
    prog = p.read(s["out"] / "stage-trace-binaries/stage-file-01.receipt.json")
    staged = p.read(base / "bench-set01-r01/stage-firmware/stage-file-01.receipt.json")
    check(prog["target"] == job["sd_program"] and prog["result"]["error"] == 0 and
          prog["result"]["bytes"] == s["binary"]["bytes"], "binary staging mismatch")
    image = b.pinned(image_spec)
    check(staged["target"] == sd_image and staged["result"]["error"] == 0
          and staged["result"]["bytes"] == len(image), "firmware staging mismatch")
    b.pinned(prod.WRITE)
    b.runtime_preflight({"firmware": {"mainboard": {}}}, cfg)
    inv = p.read(cfg["inventory"])
    evid = last / "restore-mainboard/independent-verification.json"
    check(not inv.get("pending_operation") and inv["native_vdp"]["verification_receipt"] == str(evid)
          and p.read(evid)["verified_extent_sha256"] == prod.WRITE["sha256"] and p.read(evid)["boot_cli_ready"]
          and inv["components"]["native_vdp_sha256"] == prod.APP["sha256"]
          and all(inv["components"][k] == v for k,v in prod.OTHER_COMPONENTS.items()), "unknown entry firmware")
    if sys.argv[1:] in (["preflight"], ["run"]):
        check(not out.exists(), "repeats already started")
        if sys.argv[1:] == ["preflight"]:
            print("Eight validated traces; unique minimum: R30 direct Set / aligned.", flush=True)
            return 0
        return p.run(worker, out, 2600, owner=a.OWNER, lease=a.LEASE, phase="five-winner-repeats",
                     next_action="Retain lease; stop on any failure. No replays.")
    binding = p.read(out / "supervisor-started.json")
    check(sys.argv == worker[1:] and binding["argv"] == worker and binding["lease"] == a.LEASE
          and binding["owner"] == a.OWNER, "no supervisor binding")
    target = b.Bench(cfg, kit, out)
    def phase(name):
        return ab.phase(target, name)
    with Path(cfg["operation_lock"]).open("a") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        target.lock = lock
        check(p.read(cfg["inventory"]) == inv, "entry inventory changed")
        p.record(out/"inventory-before.json", inv)
        p.record(out/"selection.json", scores)
        def save_previous(i):
            prev = (last/"vdplet-aligned" if i == 1 else out/f"repeat-{i-1}")/"results/EARTHFR.BIN"
            size = prev.stat().st_size
            archive_name = "VDPLFR.BIN" if i == 1 else f"REP0{i-1}FR.BIN"
            dest = job["cwd"] + "/" + archive_name
            def archive(c):
                entry = c.stat_entry(remote)
                check(ab.read_stat(c, dest) is None and entry[0] == size and not entry[1] & 16,
                      "unexpected trace/archive entry")
                c.move(remote, dest)
                check(ab.read_stat(c, remote) is None and c.stat_entry(dest)[0] == size, "trace archive failed")
            phase(f"archive-{i}").listener("archive", b.LISTENER, archive)
        save_previous(1)
        # Reuse the already-staged image and the independently verified Agon Flash route.
        fw.APP, fw.SD_IMAGE = image_spec, sd_image
        fw.check_flash(phase("check-updater"))
        ota = phase("candidate-ota")
        inv.update(pending_operation=dict(component="native_vdp", selection="candidate",
                   operation=str(ota.out), write_sha256=pin, recorded_epoch=time.time()), ready_claim=False)
        target.update_inventory(inv)
        boot = fw.run_ota(ota)
        r = phase("ota-mos"); r.validate_ports(); r.idle()
        b.olimex_p4_app.reboot_pair(r, boot); r.cli()
        verify = phase("candidate-readback")
        boot = fw.verify_board(verify, image)
        sel = p.read(verify.out/"selected-image.json")
        check(sel["slot"] == 1 and sel["sha256"] == pin, "wrong selected firmware")
        reports = []
        for i in range(1, 6):
            # The first fresh reset also returns the target from the independent readback.
            if i > 1:
                save_previous(i)
            r = phase(f"repeat-{i}-reset"); r.validate_ports(); r.idle()
            if i > 1:
                boot = r.keyboard()["boot"]
            fw.pulse(r.reset_config)
            admitted = b.wait_boot(r.url, boot, 60)
            p.record(r.out/"admitted.json", admitted); r.cli()
            if i == 1:
                receipt = verify.out/"independent-verification.json"
                sel.update(component="native_vdp", selection="candidate", status="verified-exception",
                           verified_extent_sha256=pin, verified_extent_bytes=len(image),
                           boot_cli_ready=True, verified_epoch=time.time(), verification_receipt=str(receipt))
                p.record(receipt, sel)
                check(p.read(cfg["inventory"]) == inv, "inventory changed")
                inv.update(native_vdp=dict(sel, prior=inv["native_vdp"]), pending_operation=None,
                           recorded_epoch=time.time(), source="Nurples cold winner repeats",
                           next_preparation="Restore production app0 before handback.")
                inv["components"]["native_vdp_sha256"] = pin
                target.update_inventory(inv)
            check(p.read(cfg["inventory"]) == inv, "firmware drift")
            if i == 1:
                subprocess.run(["ssh", "-o", "BatchMode=yes", "-o", "ConnectTimeout=8", "oryx-linuxlite",
                                "~/.local/bin/oryx-voice-alert 'Purple Nurples' 'Starting the five Earth repeat runs.'"],
                               check=True, timeout=30)
            run = phase(f"repeat-{i}"); run.invoke("run-aligned")
            data = (run.out/"results/EARTHFR.BIN").read_bytes()
            result = s["summarize"](data,512)
            check(result["updates"] == 4544 and result["refill"]["updates"] == 895
                  and result["refill"]["invalid_or_clipped"] == 0, "invalid repeat trace")
            result.update(report_sha256=sha256(data).hexdigest(), boot=admitted["boot"], firmware_sha256=pin)
            p.record(run.out/"summary.json", result); reports.append(result)
        vals = [r["skipped"] for r in reports]
        p.record(out/"repeats.json", dict(runs=reports, skipped_mean=statistics.mean(vals),
                 skipped_sample_sd=statistics.stdev(vals), skipped_range=[min(vals),max(vals)]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
