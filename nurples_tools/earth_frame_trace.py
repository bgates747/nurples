"""Decode an EFT1 little-endian post-level timing file (no timing I/O in play)."""
from math import ceil
from struct import iter_unpack


def summarize(data, offset):
    if len(data) < 33 or data[:4] != b"EFT1" or data[32]:
        raise ValueError("missing or truncated EFT1 trace")
    actual, ticks, updates, zero, skipped = (
        int.from_bytes(data[i:i+3], "little") for i in range(4, 19, 3))
    if actual != offset or not 0 < updates <= 5000 or len(data) != 33 + 4 * updates:
        raise ValueError("wrong DATA offset or record count")
    rows = list(iter_unpack("<HBB", data[33:]))
    if any(f & ~63 or f & (8 | 32) for _, _, f in rows):
        raise ValueError("VBlank interval not exactly representable")
    if (sum(t for _, t, _ in rows), sum(t == 0 for _, t, _ in rows),
        sum(max(0, t-1) for _, t, _ in rows)) != (ticks, zero, skipped):
        raise ValueError("frame rows disagree with the independent pace counters")
    summary = dict(offset=offset, vblanks=ticks, updates=updates, skipped=skipped)
    for refill, name in ((False, "no_refill"), (True, "refill")):
        group = [(duration, t, flags) for duration, t, flags in rows if bool(flags & 1) == refill]
        part = dict(updates=len(group), updates_missing_vblank=sum(t > 1 for _, t, _ in group),
                    skipped=sum(max(0, t-1) for _, t, _ in group))
        summary[name] = part
        if refill:
            durations = sorted(d for d, _, flags in group if flags == 3)
            part["invalid_or_clipped"] = len(group) - len(durations)
            if durations:
                part["ms"] = {label: round(durations[ceil(len(durations)*q)-1] / 72, 3)
                              for label, q in (("p50", .5), ("p95", .95), ("p99", .99), ("max", 1))}
    return summary
