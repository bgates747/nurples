"""Compare immutable EPS1 files offline without changing the bench parser or game."""
import argparse
from collections import Counter
from hashlib import sha256
from pathlib import Path
from struct import iter_unpack

from nurples_tools.earth_pose import POSE_ARMS, pose_summary


def describe(raw, arm):
    summary = pose_summary(raw, arm)
    rows = list(iter_unpack("<HBBBB", raw[33:]))
    # Fingerprint workload, never timing. Same totals may hide quite different
    # trajectories. Different outcomes remain valid observations, not matched timings.
    work = bytes(value for _, _, flags, p, _ in rows for value in (p, flags & 1))
    return dict(summary, workload=sha256(work).hexdigest(), report_sha256=sha256(raw).hexdigest(),
                max_poses_per_update=max(p for _, _, _, p, _ in rows),
                updates_by_sends=dict(sorted(Counter(s for _, _, _, _, s in rows).items())))


def tables(reports):
    # Never combine a mirror-off baseline with mirror-on measures. Firmware
    # identity, reset and guest hashes come from the physical runner receipts.
    arms = set(reports)
    if arms & {"native", "native-direct"} and arms - {"native", "native-direct"}:
        raise ValueError("compare the two mirror-off arms separately")
    out = ["| Arm | Skipped / VBlanks | Dense: skipped / VBlanks | Poses | Sends | Pose bytes |",
           "| --- | ---: | ---: | ---: | ---: | ---: |"]
    for name, s in reports.items():
        d = s["at_least_eight_poses"]
        out.append(f"| {name} | {s['skipped']:,} / {s['vblanks']:,} | {d['skipped']:,} / {d['vblanks']:,} "
                   f"| {s['poses']:,} | {s['pose_sends']:,} | {s['pose_wire_bytes']:,} |")
    out += ["", "1 KiB SD refill times in ms (sample SD; nearest-rank percentiles).", "",
            "| Arm | Valid | Omitted | Min | Mean | SD | Median | p95 | p99 | Max |",
            "| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |"]
    for name, s in reports.items():
        r = s["read_ms"]
        cells = " | ".join(f"{r[k]:.3f}" for k in ("min", "mean", "sd", "median", "p95", "p99", "max"))
        out.append(f"| {name} | {r['n']} | {s['refill']['invalid_or_clipped']} | {cells} |")
    if len(reports) > 1:
        same = len({s["workload"] for s in reports.values()}) == 1
        out += ["", ("Eligible-pose counts and SD-refill positions matched on every update."
                     if same else "Workload differs by update; do not attribute the entire difference to transport.")]
    return "\n".join(out)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reports", nargs="+", metavar="ARM=EPS1", help="same firmware; fresh reset per arm")
    args = parser.parse_args()
    results = {}
    for arg in args.reports:
        name, sep, path = arg.partition("=")
        if not sep or name not in POSE_ARMS or name in results:
            parser.error(f"expected unique ARM=EPS1 path, got {arg}")
        results[name] = describe(Path(path).read_bytes(), name)
    print(tables(results))
