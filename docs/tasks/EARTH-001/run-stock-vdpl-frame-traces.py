"""Continue the cold-start matrix, without restaging or rebuilding the game."""
import fcntl
from hashlib import sha256
from pathlib import Path
import runpy
import subprocess
import sys
import time

shared = runpy.run_path(str(Path(__file__).with_name("run-r30-frame-traces.py")))
a, b, p, ab, fw, prod, check, summarize = (
    shared[k] for k in ("a","b","p","ab","fw","prod","check","summarize"))
prior = shared["out"]
out = prior.parent/"bench-frame-stock-vdpl-r01"
parent_done = prior.parent/"frame-traces-direct.log.terminal.json"
worker = [sys.executable, str(Path(__file__).resolve()), "worker"]


def main():
    check(Path.cwd() == a.BENCH and Path(sys.executable) == a.BENCH/".venv/bin/python", "runner path")
    check(subprocess.check_output(["git","rev-parse","HEAD"], text=True).strip() == fw.REVISION
          and a.digest(a.JOB) == a.JOB_SHA and a.digest(a.CONFIG) == a.CONFIG_SHA, "runner/kit pins")
    cfg, kit = p.read(a.CONFIG), p.read(a.JOB)
    for job in kit["fixtures"]:
        job.update(agon=shared["binary"], results=[job["cwd"]+"/EARTHFR.BIN"])
    stock = b.pinned(fw.APP)
    kit["firmware"] = p.read("/Agon/mystuff/agon-vdplet/.worktrees/sprite-scanout/build/vdpl061/r06/job.json")["firmware"]
    spec = kit["firmware"]["mainboard"]
    check(spec["candidate_app"] == fw.APP and spec["restore_app"] == prod.APP
          and spec["restore_write"] == prod.WRITE, "firmware spec differs")
    b.preflight(kit)
    if sys.argv[1:] == ["preflight"]:
        check(not out.exists(), "follow-up already started")
        print("Pinned unchanged fixture and both firmware arms passed host preflight.")
        return 0
    if sys.argv[1:] == ["after-r30"]:
        # Wait for the host supervisor receipt only. No bench operation here.
        deadline = time.monotonic()+2400
        while not parent_done.is_file() and time.monotonic()<deadline:
            time.sleep(2)
    check(p.read(parent_done)["returncode"] == 0 and p.read(prior/"complete.json")["proof"]["all_joined"]
          and p.read(prior/"supervisor-started.json")["lease"] == a.LEASE and not (a.OUT/"handback").exists(),
          "R30 pair not joined under our reservation")
    previous = p.read(prior/"matrix.json")
    check(len(previous)==4 and all(v["updates"]==4544 for v in previous.values()), "missing R30 run")
    inventory = p.read(cfg["inventory"])
    check(not inventory.get("pending_operation") and inventory["native_vdp"]["slot"]==1
          and inventory["components"]["native_vdp_sha256"]==shared["A"]
          and all(inventory["components"][k]==v for k,v in prod.OTHER_COMPONENTS.items()), "entry firmware")
    if sys.argv[1:] == ["after-r30"]:
        return p.run(worker,out,2400,owner=a.OWNER,lease=a.LEASE,phase="stock-vdpl-frame-traces",
                     next_action="Stop at first failure and retain custody.")
    check(sys.argv == worker[1:] and p.read(out/"supervisor-started.json")["lease"]==a.LEASE,"supervision")
    target=b.Bench(cfg,kit,out)
    def phase(name):
        return ab.phase(target,name)
    reports={}
    with Path(cfg["operation_lock"]).open("a") as lock:
        fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
        target.lock=lock
        check(p.read(cfg["inventory"])==inventory,"inventory changed")
        for j,job in enumerate(kit["fixtures"]):
            old=p.read(prior/"stage-trace-binaries"/f"stage-file-{j:02d}.receipt.json")
            check(old["target"]==job["sd_program"] and old["result"]["error"]==0
                  and old["result"]["bytes"]==shared["binary"]["bytes"],"trace staging receipt mismatch")
        p.record(out/"inventory-before.json",inventory)
        for label in ("stock","vdplet"):
            def archive(c):
                moves=[]
                for job in kit["fixtures"]:
                    path=job["results"][0]
                    last=(prior/("control-"+job["id"]) if label=="stock" else out/("stock-"+job["id"]))
                    local=(last/"results/EARTHFR.BIN").read_bytes()
                    dest=job["cwd"]+("/SETAFR.BIN" if label=="stock" else "/STOCKFR.BIN")
                    check(ab.read_stat(c,dest) is None and c.stat_entry(path)[0]==len(local),
                          "prior trace/archive path mismatch")
                    moves.append((path,dest,len(local)))
                for src,dest,size in moves:
                    c.move(src,dest)
                    check(ab.read_stat(c,src) is None and c.stat_entry(dest)[0]==size,"archive mismatch")
            phase(label+"-archive").listener("archive",b.LISTENER,archive)
            if label=="stock":
                fw.check_flash(phase("check-updater"))
                staged=p.read(fw.ROOT/"operation/copy-stock-app-to-sd/stage-file-00.receipt.json")
                check(staged["target"]==fw.SD_IMAGE and staged["result"]["bytes"]==len(stock)
                      and staged["result"]["error"]==0, "stock firmware staging receipt")
                ota=phase("stock-ota")
                inventory.update(pending_operation=dict(component="native_vdp",selection="candidate",
                                 operation=str(ota.out),write_sha256=fw.APP["sha256"],recorded_epoch=time.time()),
                                 ready_claim=False)
                target.update_inventory(inventory)
                boot=fw.run_ota(ota)
                r=phase("ota-mos"); r.validate_ports(); r.idle()
                b.olimex_p4_app.reboot_pair(r,boot); r.cli()
                verify=phase("stock-readback")
                boot=fw.verify_board(verify,stock)
                sel=p.read(verify.out/"selected-image.json")
                check(sel["slot"]==0 and sel["sha256"]==fw.APP["sha256"],"stock app0 verification")
            else:
                # Stock now occupies confirmed app0: normal commissioned app0
                # restore writes and independently verifies the production extent.
                restore=phase("restore-mainboard")
                restore.firmware("mainboard","restore")
                rec=p.read(restore.out/"independent-verification.json")
                check(rec["sha256"]==prod.APP["sha256"] and rec["verified_extent_sha256"]==prod.WRITE["sha256"]
                      and rec["boot_cli_ready"],"production restoration not verified")
            for name,offset in (("control",72),("aligned",512)):
                r=phase(f"{label}-{name}-reset"); r.validate_ports(); r.idle()
                first=(label,name)==("stock","control")
                if not first:
                    boot=r.keyboard()["boot"]
                fw.pulse(r.reset_config)
                p.record(r.out/"admitted.json",b.wait_boot(r.url,boot,60)); r.cli()
                if first:
                    receipt=str(verify.out/"independent-verification.json")
                    sel.update(component="native_vdp",selection="candidate",status="verified-exception",
                               verified_extent_sha256=fw.APP["sha256"],verified_extent_bytes=len(stock),
                               boot_cli_ready=True,verified_epoch=time.time(),verification_receipt=receipt)
                    p.record(receipt,sel)
                    check(p.read(cfg["inventory"])==inventory,"inventory changed")
                    inventory.update(native_vdp=dict(sel,prior=inventory["native_vdp"]),pending_operation=None,
                                     recorded_epoch=time.time(),source="Nurples stock timing trace",
                                     next_preparation="Restore production VDPlet before handback.")
                    inventory["components"]["native_vdp_sha256"]=fw.APP["sha256"]
                    target.update_inventory(inventory)
                pin=fw.APP["sha256"] if label=="stock" else prod.APP["sha256"]
                check(p.read(cfg["inventory"])["components"]["native_vdp_sha256"]==pin,"firmware drift")
                subprocess.run(["ssh","-o","BatchMode=yes","-o","ConnectTimeout=8","oryx-linuxlite",
                                f"~/.local/bin/oryx-voice-alert 'Purple Nurples' '{label}, {name} Earth trace starting.'"],
                               check=True,timeout=30)
                arm=phase(f"{label}-{name}")
                arm.invoke("run-"+name)
                data=(arm.out/"results/EARTHFR.BIN").read_bytes()
                result=summarize(data,offset)
                check(result["updates"]==4544 and result["refill"]["updates"]==895,"incomplete Earth trace")
                result.update(firmware_sha256=pin,report_sha256=sha256(data).hexdigest())
                p.record(arm.out/"summary.json",result)
                reports[f"{label}-{name}"]=result
        p.record(out/"matrix.json",reports)
    return 0


if __name__=="__main__":
    raise SystemExit(main())
