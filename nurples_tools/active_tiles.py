"""Generate the DG activation table. Actor defaults and callbacks stay handwritten."""

import argparse
from dataclasses import dataclass
from pathlib import Path
import re
import tomllib

REGISTRY = Path("maps/active_tiles.toml")
INCLUDE = Path("src/asm/active_tile_registry.inc")
# Keys are stable runtime identities, not editor coordinates. Runtime bitmap
# 121 is the existing artwork at Tiled local 120; no extra upload is needed.
PAD_ART = {2: 121, 3: 121, 15: 121}
HEADINGS = {"north": 0, "east": 64, "south": 128, "west": 192}


@dataclass(frozen=True)
class Slot:
    name: str
    actor: str
    entry_heading: int | None = None

    @property
    def handler(self):
        return "activate_null_tile" if self.actor == "none" else f"activate_{self.actor}"


def parse(text):
    doc = tomllib.loads(text)
    if doc.keys() - {"slots", "entry_facing"} or not isinstance(doc.get("slots"), dict):
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
    facing = doc.get("entry_facing", {})
    if not isinstance(facing, dict) or facing.keys() - canonical or any(
        not isinstance(v, str) or v not in HEADINGS for v in facing.values()
    ):
        raise ValueError("entry_facing expects canonical actor names and cardinal directions")
    return tuple(Slot(s.name, s.actor, HEADINGS.get(facing.get(s.actor))) for s in slots)


def render(slots):
    lines = ["; Generated from maps/active_tiles.toml; do not edit."]
    lines.extend(f"tile_{s.name}: equ {i}" for i, s in enumerate(slots) if i)
    lines.extend(["", "tile_activation_routines:"])
    lines.extend(f"    dl {s.handler} ; {i}: {s.name}" for i, s in enumerate(slots))
    lines.extend(["tile_activation_routines_end:",
                  "tile_activation_routines_count: equ [tile_activation_routines_end-tile_activation_routines]/3"])
    lines += ["tile_active_art:",
              "    db " + ",".join(str(PAD_ART.get(i, i)) for i in range(len(slots)))]
    lines += ["tile_entry_headings: ; 255 leaves the constructor defaults alone",
              "    db " + ",".join(str(255 if s.entry_heading is None else s.entry_heading)
                                   for s in slots)]
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
