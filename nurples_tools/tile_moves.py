"""Preview/apply one DG tilesheet rearrangement, using Tiled's local IDs."""

import argparse
import hashlib
import re
import subprocess
import tempfile
import xml.etree.ElementTree as ET
from collections import Counter
from io import BytesIO
from pathlib import Path

from PIL import Image

from nurples_tools.maps import _load_tile_catalog, _validate_tileset
from nurples_tools.tile_bindings import PROPERTY, exchange

TSX = Path("tiles/dg/dg0.tsx")
PNG = TSX.with_suffix(".png")
CSV = TSX.with_name("dg_0_map.csv")
CAT = TSX.with_name("dg_0_files.xml")


def git(root, *args, data=None):
    return subprocess.run(
        ["git", "-C", root, *args], input=data, capture_output=True, check=True
    ).stdout


def tracked(root, *patterns):
    return [Path(p.decode()) for p in git(root, "ls-files", "-z", "--", *patterns)
            .split(b"\0") if p]


def check_clean(root):
    if git(root, "status", "--porcelain", "--untracked-files=all"):
        raise ValueError("checkpoint all changes first; a clean checkout is required")
    if any(line.startswith(b"S ") for line in git(root, "ls-files", "-t", "-z").split(b"\0")):
        raise ValueError("run in the full checkout, not the sparse design checkout")


def rewrite_map(root, path, a, b):
    raw = (root / path).read_bytes()
    tree = ET.fromstring(raw)
    sets = tree.findall("tileset")
    sources = [(root / path.parent / t.get("source", "")).resolve() for t in sets]
    if (root / TSX).resolve() not in sources:
        if any(p.name == "dg0.tsx" or not t.get("source") for p, t in zip(sources, sets)):
            raise ValueError(f"{path}: unresolved or embedded tileset")
        return None
    layers = tree.findall("layer")
    if (tree.tag != "map" or len(sets) != 1 or sets[0].get("firstgid") != "1"
            or len(sets[0]) or len(layers) != 1 or tree.get("infinite", "0") != "0"
            or any(e.tag not in {"tileset", "layer"} for e in tree)):
        raise ValueError(f"{path}: expected a single finite DG tile layer")
    layer = layers[0]
    data = layer.find("data")
    if (len(layer) != 1 or data is None or data.attrib != {"encoding": "csv"}
            or len(data) or not data.text):
        raise ValueError(f"{path}: expected plain CSV data")
    # Splice only CSV bytes. Preserve the rest of the Tiled document verbatim.
    matches = list(re.finditer(rb"<data\b[^>]*>([^<]*)</data\s*>", raw))
    if len(matches) != 1:
        raise ValueError(f"{path}: unsupported CSV serialization")
    start, end = matches[0].span(1)
    payload = raw[start:end]
    if not re.fullmatch(rb"\s*\d+(?:\s*,\s*\d+)*\s*", payload):
        raise ValueError(f"{path}: malformed CSV")
    ids = [int(token) for token in payload.split(b",")]
    if (len(ids) != int(layer.attrib["width"]) * int(layer.attrib["height"])
            or max(ids) > 255):
        raise ValueError(f"{path}: unexpected cell count or unsupported GID/flip flags")
    counts = Counter(ids)
    change = {a: str(b).encode(), b: str(a).encode()}
    updated = re.sub(rb"\d+", lambda m: change.get(int(m[0]), m[0]), payload)
    return raw[:start] + updated + raw[end:], counts[a], counts[b]


def build_plan(root, op, src, dst):
    if not (0 <= src <= 254 and 0 <= dst <= 254 and src != dst):
        raise ValueError("choose two distinct Tiled local IDs in 0..254")
    a, b = src + 1, dst + 1
    ids = _validate_tileset(root / TSX)
    tree = ET.parse(root / TSX).getroot()
    # Animation frames, per-tile collision shapes and wangsets would also need
    # rewriting. Do not silently drop or leave behind unknown slot references.
    if (any(e.tag not in {"properties", "image"} for e in tree)
            or any(p.tag != "property" or p.get("name") != PROPERTY
                   for props in tree.findall("properties") for p in props)
            or any(int(tree.get(k, "0")) for k in ("spacing", "margin"))):
        raise ValueError("unsupported DG tileset metadata")
    if tracked(root, "*.tmj", "*.tx"):
        raise ValueError("Tiled JSON maps/templates need a reference review first")
    catalog, _ = _load_tile_catalog(root / CAT)
    original = (root / CSV).read_bytes()
    rows = original.splitlines()
    if len(rows) != 16 or any(not re.fullmatch(rb"\d{3}(,\d{3}){15}", row) for row in rows):
        raise ValueError("unsupported DG historical cross-reference grid")
    entries = re.findall(rb"\d{3}", original)
    changed = {}
    counts = [0, 0]
    report = [f"{op} Tiled local {src} / {dst}; GIDs {a} / {b}; "
              f"runtime identities {ids[a]} / {ids[b]}"]
    for path in tracked(root, "*.tmx"):
        result = rewrite_map(root, path, a, b)
        if result is None:
            continue
        raw, n, m = result
        counts[0] += n
        counts[1] += m
        report.append(f"{path}: {n} / {m} cells")
        if n or m:
            changed[path] = raw

    with Image.open(root / PNG) as image:
        if image.mode != "RGBA" or image.size != (256, 256):
            raise ValueError("expected 256x256 RGBA atlas")
        boxes = [(i % 16 * 16, i // 16 * 16, i % 16 * 16 + 16, i // 16 * 16 + 16)
                 for i in (src, dst)]
        tiles = [image.crop(box) for box in boxes]
        occupied = [bool(tile.getchannel("A").getbbox() or ids[g] in catalog or n)
                    for tile, g, n in zip(tiles, (a, b), counts)]
        if not occupied[0]:
            raise ValueError("source slot is free")
        if op == "move" and (occupied[1] or ids[b] <= 16 or int(entries[dst])):
            raise ValueError("move destination is occupied or reserved; not a free slot")
        if op == "swap" and not occupied[1]:
            raise ValueError("swap destination has no tile; use move for a free slot")
        if op not in {"move", "swap"}:
            raise ValueError("unknown operation")
        for box, tile in zip(boxes, reversed(tiles)):
            image.paste(tile, box)
        out = BytesIO()
        image.save(out, format="PNG")
        changed[PNG] = out.getvalue()

    # Preserve this older cell-indexed cross-reference too. Its zero entries
    # are not authoritative: some recovered artwork postdates the CSV.
    entries[src], entries[dst] = entries[dst], entries[src]
    iterator = iter(entries)
    changed[CSV] = re.sub(rb"\d{3}", lambda _: next(iterator), original)
    exchange(tree, a, b)
    ET.indent(tree, space=" ")
    changed[TSX] = ET.tostring(tree, encoding="utf-8", xml_declaration=True) + b"\n"
    return changed, report


def binary_patch(root, changed):
    # Let Git handle all-or-nothing application (including the binary PNG),
    # with index/worktree preimage checks. No project writes while planning.
    with tempfile.TemporaryDirectory(prefix="nurples-map-") as work:
        tmp = Path(work)
        git(tmp, "init", "-q")
        git(tmp, "config", "core.autocrlf", "false")
        for path in changed:
            (tmp / path).parent.mkdir(exist_ok=True, parents=True)
            (tmp / path).write_bytes((root / path).read_bytes())
        git(tmp, "add", "-f", "--", *changed)
        for path, raw in changed.items():
            (tmp / path).write_bytes(raw)
        return git(tmp, "diff", "--binary", "--full-index", "--no-ext-diff",
                   "--no-textconv", "--src-prefix=a/", "--dst-prefix=b/")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("operation", choices=("move", "swap"))
    parser.add_argument("source", type=int)
    parser.add_argument("destination", type=int)
    parser.add_argument("--apply", metavar="PREVIEW_TOKEN", help="apply and stage the preview")
    args = parser.parse_args()
    try:
        root = Path(git(".", "rev-parse", "--show-toplevel").decode().strip())
        check_clean(root)
        head = git(root, "rev-parse", "HEAD").strip()
        changed, report = build_plan(root, args.operation, args.source, args.destination)
        patch = binary_patch(root, changed)
        token = hashlib.sha256(head + b"\n" + report[0].encode() + b"\n" + patch).hexdigest()
        print("\n".join(report))
        git(root, "apply", "--index", "--check", "-", data=patch)
        if args.apply is None:
            print(f"Dry run; no changes. To stage this exact plan repeat with --apply {token}")
        else:
            if args.apply != token:
                raise ValueError("stale or incorrect preview token; preview again")
            if git(root, "branch", "--show-current").strip() != b"dev":
                raise ValueError("only apply from dev")
            check_clean(root)
            if head != git(root, "rev-parse", "HEAD").strip():
                raise ValueError("HEAD changed; preview again")
            git(root, "apply", "--index", "-", data=patch)
            print("Staged for review. No commit was made.")
    except subprocess.CalledProcessError as exc:
        parser.exit(2, f"Git refused the operation: {exc.stderr.decode().strip()}\n")
    except (ValueError, OSError, KeyError, ET.ParseError) as exc:
        parser.exit(2, f"error: {exc}\n")


if __name__ == "__main__":
    main()
