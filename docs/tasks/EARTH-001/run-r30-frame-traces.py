"""Held Olimex job: after SET01 joins, four independently reset Earth runs."""
import fcntl
import subprocess
import sys
import time
from pathlib import Path

agents = Path("/Agon/mystuff/agon-extender/agents")
sys.path[:0] = [str(agents / "olimex-stock-recovery/20261126"), str(agents / "earth-001/recovered-stage"),
                "/Agon/mystuff/nurples"]
import recover as fw
import restore_production as prod
import recover_aligned as ab
from nurples_tools.earth_frame_trace import summarize

a, b, p, check = ab.admit, fw.bench, fw.process, fw.require
prior = a.OUT.parent / "bench-set01-r01"
out = a.OUT.parent / "bench-frame-r01"
worker = [sys.executable, str(Path(__file__).resolve()), "worker"]
binary = dict(path=str(a.OUT.parent / "frame-trace/nurples.bin"), bytes=113131,
              sha256="3f0f26d84d804fab21ff17ed94d03a0c5864bbd1ac9f6d3ca34589b91ce84be4")
A = "1f6a84e8a8721feb2d48932296326165abcd9d43e5734a9d69f1c1fbe113458e"
B = "b4cbc886c1959aca4ab692730d20f27bc01e4763226add2d1869fca74d882d90"


def main():
    check(Path.cwd() == a.BENCH and Path(sys.executable) == a.BENCH / ".venv/bin/python",
          "not at commissioned runner")
    check(a.digest(a.JOB) == a.JOB_SHA and a.digest(a.CONFIG) == a.CONFIG_SHA, "job/config changed")
    check(subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip() == fw.REVISION,
          "bench checkout changed")
    kit, cfg = p.read(a.JOB), p.read(a.CONFIG)
    b.preflight(kit)
    prog = b.pinned(binary)
    b.pinned(prod.WRITE)
    spec = dict(path="/Agon/mystuff/agon-vdplet/.worktrees/sprite-scanout/build/vdpl062/set01/control/firmware.bin",
                bytes=1080064, sha256=A)
    image = b.pinned(spec)
    fwpath = "/extender/install/nurples-set01-r01/control.bin"
    for job in kit["fixtures"]:
        job["agon"] = binary
        job["results"] = [job["cwd"] + "/EARTHFR.BIN"]
    b.preflight(kit)
    if sys.argv[1:] == ["preflight"]:
        check(not out.exists(), "trace job already exists")
        print("Host preflight passed: one executable, two DATA offsets, both R30 pins.")
        return 0
    check(p.read(a.OUT.parent / "set01-r01.log.terminal.json")["returncode"] == 0
          and p.read(prior / "complete.json")["proof"]["all_joined"]
          and p.read(prior / "supervisor-started.json")["lease"] == a.LEASE
          and not (a.OUT / "handback").exists(), "prior SET01 job not successfully joined under our slot")
    inv = p.read(cfg["inventory"])
    check(not inv.get("pending_operation") and inv["native_vdp"]["slot"] == 0
          and inv["components"]["native_vdp_sha256"] == B
          and all(inv["components"][k] == v for k, v in prod.OTHER_COMPONENTS.items()),
          "unexpected starting firmware")
    if sys.argv[1:] == ["run"]:
        return p.run(worker, out, 2350, owner=a.OWNER, lease=a.LEASE, phase="r30-frame-traces",
                     next_action="Stop on first failure and retain the slot.")
    check(sys.argv == worker[1:] and p.read(out/"supervisor-started.json")["lease"] == a.LEASE,
          "missing supervision")
    t = b.Bench(cfg, kit, out)
    def phase(name):
        return ab.phase(t, name)
    with Path(cfg["operation_lock"]).open("a") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        t.lock = lock
        check(p.read(cfg["inventory"]) == inv, "entry inventory changed")
        p.record(out/"inventory-before.json", inv)
        def reserve_paths(c):
            moves = []
            for job in kit["fixtures"]:
                src = job["sd_program"]
                dst = job["cwd"] + "/EABBASE.BIN"
                st = c.stat_entry(src)
                check(st[0] == 92852 and not st[1] & 16 and ab.read_stat(c, dst) is None
                      and ab.read_stat(c, job["results"][0]) is None, "unexpected existing SD file")
                moves.append((src, dst))
            for src, dst in moves:
                c.move(src, dst)
                check(ab.read_stat(c, src) is None and c.stat_entry(dst)[0] == 92852, "archive failed")
        phase("save-binaries").listener("reserve-paths", b.LISTENER, reserve_paths)
        phase("stage-trace-binaries").stage_parallel(
            {job["sd_program"]: prog for job in kit["fixtures"]}, [], fw.existing)
        results = {}
        for firmware, pin in (("candidate", B), ("control", A)):
            # OUT records prove which actual image was used, not just a caller's label.
            if firmware == "control":
                fw.APP, fw.SD_IMAGE = spec, fwpath
                x = phase("check-updater")
                fw.check_flash(x)
                staged = p.read(prior/"stage-firmware/stage-file-00.receipt.json")
                check(staged["target"] == fwpath and staged["result"]["bytes"] == len(image)
                      and staged["result"]["error"] == 0, "R30 control image not staged")
                ota = phase("install-control")
                inv.update(pending_operation=dict(component="native_vdp", selection="candidate",
                           operation=str(ota.out), write_sha256=A, recorded_epoch=time.time()), ready_claim=False)
                t.update_inventory(inv)
                boot = fw.run_ota(ota)
                r = phase("ota-mos")
                r.validate_ports(); r.idle()
                b.olimex_p4_app.reboot_pair(r, boot)
                r.cli()
                verify = phase("control-readback")
                boot = fw.verify_board(verify, image)
                sel = p.read(verify.out/"selected-image.json")
                check(sel["slot"] == 1 and sel["sha256"] == A, "wrong selected R30 firmware")
            for arm, offset in (("control", 72), ("aligned", 512)):
                r = phase(f"{firmware}-{arm}-reset")
                r.validate_ports(); r.idle()
                # Following independent ROM readback, the previously observed boot
                # is the pre-programmer epoch; no extra mainboard pulse is needed.
                if (firmware, arm) != ("control", "control"):
                    boot = r.keyboard()["boot"]
                fw.pulse(r.reset_config)
                p.record(r.out/"admitted.json", b.wait_boot(r.url, boot, 60))
                r.cli()
                if (firmware, arm) == ("control", "control"):
                    receipt = str(verify.out/"independent-verification.json")
                    sel.update(component="native_vdp", selection="candidate", status="verified-exception",
                               verified_extent_bytes=len(image), verified_extent_sha256=A, boot_cli_ready=True,
                               verified_epoch=time.time(), verification_receipt=receipt)
                    p.record(receipt, sel)
                    check(p.read(cfg["inventory"]) == inv, "inventory changed")
                    inv.update(native_vdp=dict(sel, prior=inv["native_vdp"]), pending_operation=None,
                               source="Nurples R30 control frame trace", recorded_epoch=time.time(),
                               next_preparation="Restore production VDPlet before check-in.")
                    inv["components"]["native_vdp_sha256"] = A
                    t.update_inventory(inv)
                check(p.read(cfg["inventory"])["components"]["native_vdp_sha256"] == pin, "firmware drift")
                job = b.fixture_runs(kit)["run-"+arm]
                path = job["results"][0]
                subprocess.run(["ssh","-o","BatchMode=yes","-o","ConnectTimeout=8","oryx-linuxlite",
                                f"~/.local/bin/oryx-voice-alert 'Purple Nurples' 'R30 {firmware}, {arm} Earth trace starting.'"],
                               check=True, timeout=30)
                run = phase(f"{firmware}-{arm}")
                run.invoke("run-"+arm)
                payload = (run.out/"results/EARTHFR.BIN").read_bytes()
                result = summarize(payload, offset)
                check(result["updates"] == 4544 and result["refill"]["updates"] > 0, "not a full refill trace")
                p.record(run.out/"summary.json", result)
                results[f"{firmware}-{arm}"] = result
                if firmware == "candidate":
                    def archive(c):
                        dst = job["cwd"] + "/SETBFR.BIN"
                        check(ab.read_stat(c, dst) is None and c.stat_entry(path)[0] == len(payload),
                              "unexpected trace archive")
                        c.move(path, dst)
                        check(ab.read_stat(c, path) is None and c.stat_entry(dst)[0] == len(payload),
                              "trace archive not reconciled")
                    phase("archive-"+arm).listener("move-report", b.LISTENER, archive)
        p.record(out/"matrix.json", results)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
