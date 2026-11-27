"""Optional sparse scope observer. Its skip counts are NOT benchmark scores."""
from collections import Counter, defaultdict
from math import ceil
from statistics import mean, median
from struct import iter_unpack

from nurples_tools.earth_pose import pose_summary
from nurples_tools.loading_music import edit


ARMS = ("direct", "sync", "golem")
CAP = 2048
SIZE = 10
FIRST = 17
STRIDE = 61


def configure_cost(src, arm):
    index = ARMS.index(arm)
    # Only eligible direct poses take copies of these three helpers. Text,
    # HUD and other sprite traffic must not get charged to the pose send.
    helpers = ("vdu_sprite_select", "vdu_sprite_select_frame", "vdu_sprite_move_abs168")
    vdu = (src / "vdu_sprites.inc").read_text()
    clones = []
    if arm == "direct":
        for name in helpers:
            body = vdu.split("\n" + name + ":\n")[1].split("@end:")[0]
            if body.count("rst.lil $18") != 1:
                raise ValueError(f"unexpected {name} body")
            clones.append("pc_" + name + ":\n" + body.replace("rst.lil $18", "call pc_send") + "@end:\n")
            op = "jp" if name.endswith("168") else "call"
            edit(src, "earth_pose.inc", f"    {op} {name}\n", f"    {op} pc_{name}\n")
    (src / "pc_helpers.inc").write_text("\n".join(clones))
    edit(src, "sprites.inc", "    call pose_publish\n", "    call pc_publish\n")
    file = "earth_pose_state.inc" if arm == "golem" else "earth_pose.inc"
    source = (src / file).read_text()
    before, after = source.split("pose_submit:\n")
    body, rest = after.split("\npose_barrier:\n")
    if body.count("    ret z") != 1 or body.count("    rst.lil 18h") != 1:
        raise ValueError("unexpected nonempty submit boundary")
    lines = body.splitlines(keepends=True)
    site = next(i for i, line in enumerate(lines) if line.startswith("    ret z"))
    lines.insert(site + 1, "    jp pc_submit\npc_submit_body:\n")
    (src / file).write_text(before + "pose_submit:\n" + "".join(lines).replace(
        "    rst.lil 18h", "    call pc_send") + "\npose_barrier:\n" + rest)
    edit(src, "frame_pace_fixture.inc", "    ld (pace_count_this),a\n    ret",
         "    ld (pace_count_this),a\n    jp pc_frame")
    name = "earth_bench_report.inc"
    edit(src, name, 'db "EPS1"', 'db "EPT1"')
    edit(src, name, 'asciz "POSE.BIN"', 'asciz "POSECOST.BIN"')
    edit(src, name, "trace_rows: ds 5000*6",
         f"pc_arm: db {index}\npc_bad: db 0\npc_used: dl 0\npc_events: ds {CAP}*{SIZE}\n"
         "pc_end:\ntrace_rows: ds 5000*6")
    edit(src, name, "bench_save:\n", "bench_save:\n    ld hl,(pc_ptr)\n    ld de,pc_events\n"
         "    or a\n    sbc hl,de\n    ld (pc_used),hl\n")
    with (src / name).open("a") as stream:
        stream.write('\n    include "earth_pose_cost.inc"\n    include "pc_helpers.inc"\n')


def decode_cost(blob, arm):
    """Reject missing/extra samples using the very same run's per-frame EPS1 rows."""
    offset = 38 + CAP * SIZE
    if len(blob) < offset or blob[:4] != b"EPT1" or blob[33:35] != bytes([ARMS.index(arm), 0]):
        raise ValueError("wrong observer, timer unavailable or capture overflow")
    used = int.from_bytes(blob[35:38], "little")
    if used % SIZE or used > CAP * SIZE:
        raise ValueError("bad observer length")
    rows = blob[offset:]
    game = pose_summary(b"EPS1" + blob[4:33] + rows, arm)
    expected, wire_by_frame, payload_by_frame = [], Counter(), Counter()
    for k, frame in enumerate(range(FIRST, game["updates"] + 1, STRIDE)):
        _, _, _, poses, sends = next(iter_unpack("<HBBBB", rows[6*(frame-1):6*frame]))
        tag = 1 + 2*(k % 3)
        count = poses if tag == 1 else (0 if tag == 3 and arm == "direct" else sends)
        expected.extend([(frame, 7), (frame, 8)] + [(frame, tag), (frame, tag+1)] * count)
        if tag == 5:
            wire_by_frame[frame] = 26*sends + 6*poses if arm == "golem" else 15*poses
        if tag == 3 and arm != "direct":
            payload_by_frame[frame] = (6 if arm == "golem" else 15)*poses
    events = list(iter_unpack("<BHBHHH", blob[38:38+used]))
    if [(f, t) for t, f, *_ in events] != expected:
        raise ValueError("events do not reconcile with actual sampled pose/send counts")
    samples = defaultdict(list)
    invalid = defaultdict(int)
    names = {1: "publish", 3: "nonempty_submit", 5: "mos_send", 7: "empty_pair"}
    for a, b in zip(events[::2], events[1::2]):
        tag, frame, lo, hi, down, wire = a
        key = (names[tag], wire if tag in (3, 5) else 0)
        irq_delta = ((b[3] << 8 | b[2]) - (hi << 8 | lo)) & 0xffffff
        ticks = irq_delta*1440 + down - b[4]
        if tag == 5 and not (wire in (4, 7) if arm == "direct" else
                              (15 <= wire <= 480 and wire % 15 == 0 if arm == "sync" else
                               32 <= wire <= 218 and (wire-26) % 6 == 0)):
            raise ValueError("impossible MOS packet length")
        if tag == 5:
            wire_by_frame[frame] -= wire
        if tag == 3:
            stride = 6 if arm == "golem" else 15
            if not stride <= wire <= stride*32 or wire % stride:
                raise ValueError("impossible nonempty batch payload")
            payload_by_frame[frame] -= wire
        if max(down, b[4]) > 1440 or ticks < 0 or irq_delta > 0x7fffff:
            invalid[key] += 1  # pending IRQ race or bad timestamp; never clamp to zero
        else:
            samples[key].append(ticks)
    if any(wire_by_frame.values()) or any(payload_by_frame.values()):
        raise ValueError("sample lengths disagree with the same frame's pose count")
    return {f"{name}:{wire}": {"n": len(vals), "invalid": invalid[name, wire],
                               "mean_counts": mean(vals) if vals else None,
                               "median_counts": median(vals) if vals else None,
                               "p95_counts": sorted(vals)[ceil(.95*len(vals))-1] if vals else None,
                               "min_counts": min(vals, default=None), "max_counts": max(vals, default=None)}
            for name, wire in sorted(samples.keys() | invalid.keys()) for vals in [samples[name, wire]]}


if __name__ == "__main__":
    import argparse
    import json
    from pathlib import Path

    parser = argparse.ArgumentParser(description="EPT1 sparse observer; 72 counts per ms")
    parser.add_argument("arm", choices=ARMS)
    parser.add_argument("report", type=Path)
    args = parser.parse_args()
    print(json.dumps(decode_cost(args.report.read_bytes(), args.arm), indent=2))
