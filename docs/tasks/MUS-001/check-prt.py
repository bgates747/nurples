"""Decode a loader or whole-game timing report against its deployed containers."""
import argparse
from pathlib import Path

from nurples_tools.agnb_verify import _chunk


def sizes(path):
    data = path.read_bytes()
    assert data[:4] == b"RIFF" and data[8:12] == b"AGNB"
    pos = 12
    while pos < len(data):
        kind, start, end, pos = _chunk(data, pos, len(data))
        if kind != b"LIST":
            continue
        assert data[start:start + 4] == b"BUFR"
        inner = start + 4
        while inner < end:
            kind, start, stop, inner = _chunk(data, inner, end)
            if kind == b"DATA":
                yield stop - start


def check(report, payload, expect=None):
    data = report.read_bytes()
    async_assets = data[:4] == b"LPR2"
    whole_game = data[:4] == b"LPR5"
    if expect and not whole_game:
        raise ValueError("Phase checks require the whole-game LPR5 report")
    assert (len(data), data[:4]) in ((27, b"LPR1"), (30, b"LPR2"), (27, b"LPR3"),
                                     (27, b"LPR4"), (57, b"LPR5"))
    arm, status = data[4:6]
    elapsed, irqs, sent, missed, late, pieces, total = (
        int.from_bytes(data[i:i + 3], "little") for i in range(6, 27, 3))
    print(f"{report.name}: mode={arm} status={status:#x} MOS-clock={elapsed} "
          f"IRQ={irqs} serviced={sent} missed={missed} "
          f"worst-delay={late}/72 ms ({late / 72:.2f} ms nominal) "
          f"appends={pieces} payload={total}")
    assert elapsed > 0
    if arm == 0:
        assert status == 1 and not any((irqs, sent, missed, late, pieces, total))
    else:
        counts = [n for name in ("ui.agnb", "game.agnb", "voice.agnb")
                  for n in sizes(payload / name)]
        assert arm == 1 and status == 9
        assert sent > 0 and sent + missed <= irqs
        if whole_game:
            load_time, load_missed, load_late, *phases, paused = (
                int.from_bytes(data[i:i + 3], "little") for i in range(27, 57, 3))
            assert sum(phases) == sent and phases[0] > 0
            assert 0 < load_time <= elapsed and load_missed <= missed and load_late <= late
            assert paused <= irqs - sent - missed
            print(f"Loading: MOS-clock={load_time} missed={load_missed} peak={load_late}/72 ms; "
                  f"ticks [load, prompt, attract, level 1, level 2, Game Over]={phases}; "
                  f"paused periods={paused}; other unplayed={irqs-sent-missed-paused}")
            if expect:
                # MOS sysvar_time counts 1/120 s; compare the whole invocation,
                # not just the sounding periods, to its independent PRT count.
                assert 49 <= irqs * 120 / elapsed <= 51, "Unexpected PRT cadence"
            if expect == "play-pause":
                assert all(phases[:4]) and paused > 0, "Did not reach play and resume"
            if expect == "resident-prompt":
                assert phases[1] > 0 and not any(phases[2:]) and not paused
            if expect == "campaign":
                assert all(phases), "Did not deliver music in every requested phase"
        else:
            assert irqs <= sent + missed + 1
        cap = 1024 if data[:4] in (b"LPR4", b"LPR5") else 512
        assert total == sum(counts) and pieces == sum((n + cap - 1) // cap for n in counts)
        if async_assets:
            callbacks = int.from_bytes(data[27:30], "little")
            print(f"async completions: {callbacks}/{pieces}")
            assert callbacks == pieces
    return elapsed


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("report", type=Path)
    parser.add_argument("payload", type=Path)
    parser.add_argument("--expect", choices=("play-pause", "resident-prompt", "campaign"))
    args = parser.parse_args()
    check(args.report, args.payload, args.expect)
