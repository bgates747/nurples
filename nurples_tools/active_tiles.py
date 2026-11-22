"""Generate the DG activation table. Actor defaults and callbacks stay handwritten."""

import argparse
from dataclasses import dataclass
from pathlib import Path
import re
import tomllib

REGISTRY = Path("maps/active_tiles.toml")
INCLUDE = Path("src/asm/active_tile_registry.inc")


@dataclass(frozen=True)
class Slot:
    name: str
    actor: str

    @property
    def handler(self):
        return "activate_null_tile" if self.actor == "none" else f"activate_{self.actor}"


def parse(text):
    doc = tomllib.loads(text)
    if doc.keys() != {"slots"} or not isinstance(doc["slots"], dict):
        raise ValueError("expected [slots] in active tile registry")
    table = doc["slots"]
    if set(table) != {str(i) for i in range(len(table))} or not 1 < len(table) <= 128:
        raise ValueError("expected contiguous active IDs from 0; actor bit 7 is reserved")
    slots = []
    for i in range(len(table)):
        pair = table[str(i)]
        if not isinstance(pair, list) or len(pair) != 2 or not all(
            isinstance(v, str) and re.fullmatch("[a-z0-9_]+", v) for v in pair
        ):
            raise ValueError(f"invalid active tile entry {i}")
        slots.append(Slot(*pair))
    if slots[0] != Slot("blank", "none") or len({s.name for s in slots}) != len(slots):
        raise ValueError("blank must stay empty and slot names must be unique")
    canonical = {s.name for s in slots if s.name == s.actor}
    if any(s.actor != "none" and s.actor not in canonical for s in slots):
        raise ValueError("actor must have a canonical active tile entry")
    return tuple(slots)


def render(slots):
    lines = ["; Generated from maps/active_tiles.toml; do not edit."]
    lines.extend(f"tile_{s.name}: equ {i}" for i, s in enumerate(slots) if i)
    lines.extend(["", "tile_activation_routines:"])
    lines.extend(f"    dl {s.handler} ; {i}: {s.name}" for i, s in enumerate(slots))
    lines.extend(["tile_activation_routines_end:",
                  "tile_activation_routines_count: equ [tile_activation_routines_end-tile_activation_routines]/3"])
    return "\n".join(lines) + "\n"


def registry(root, *, write=False):
    slots = parse((root / REGISTRY).read_text())
    data = render(slots).encode()
    path = root / INCLUDE
    if not path.exists() or path.read_bytes() != data:
        if not write:
            raise ValueError(f"{INCLUDE} stale; regenerate using --assets")
        path.write_bytes(data)
    return slots


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--write", action="store_true", help="regenerate; default is check only")
    registry(Path(__file__).resolve().parents[1], write=parser.parse_args().write)
