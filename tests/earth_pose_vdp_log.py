"""Compare what stock FAE actually did to formation sprites, not just game counters."""
import argparse
import hashlib
import re
import struct
from itertools import zip_longest
from pathlib import Path

EVENT = re.compile(rb"vdu_sys_sprites: sprite (\d+) (?:set to frame (\d+)|- move to \((-?\d+),(-?\d+)\))")


def actions(log):
    with log.open("rb") as stream:
        for line in stream:
            m = EVENT.search(line)
            if not m:
                continue
            actor = int(m[1])
            if 12 <= actor < 44:
                if m[2] is not None:
                    yield struct.pack("<BBH", 1, actor, int(m[2]))
                else:
                    yield struct.pack("<BBhh", 2, actor, int(m[3]), int(m[4]))


def compare(reference, candidate):
    count, md = 0, hashlib.sha256()
    for count, (a, b) in enumerate(zip_longest(actions(reference), actions(candidate)), start=1):
        if a != b:
            raise ValueError(f"stock VDP action {count} differs: {a!r} != {b!r}")
        md.update(a)
    if count < 10000:
        raise ValueError(f"not a full formation run: {count} actual frame/move events")
    return count, md.hexdigest()


if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("logs", nargs=2, type=Path)
    print(*compare(*p.parse_args().logs))
