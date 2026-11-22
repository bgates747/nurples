"""Decode and cross-check the two guest reports against the deployed containers."""
import sys
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


def check(report, payload):
    data = report.read_bytes()
    async_assets = data[:4] == b"LPR2"
    assert (len(data), data[:4]) in ((27, b"LPR1"), (30, b"LPR2"), (27, b"LPR3"))
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
        assert sent > 0 and sent + missed <= irqs <= sent + missed + 1
        assert total == sum(counts) and pieces == sum((n + 511) // 512 for n in counts)
        if async_assets:
            callbacks = int.from_bytes(data[27:30], "little")
            print(f"async completions: {callbacks}/{pieces}")
            assert callbacks == pieces
    return elapsed


if __name__ == "__main__":
    check(Path(sys.argv[1]), Path(sys.argv[2]))
