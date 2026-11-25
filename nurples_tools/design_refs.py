"""Validate Tiled references inside each design silo and shared asset tree."""
import json
from pathlib import Path
import xml.etree.ElementTree as ET


def validate(root: Path):
    root = root.resolve()
    assets = root / "assets"
    for silo in [assets, *(root/"levels").iterdir(), *(root/"template").iterdir()]:
        for path in silo.rglob("*"):
            if path.suffix in (".tmx", ".tsx"):
                refs = [e.attrib["source"] for e in ET.parse(path).iter() if "source" in e.attrib]
            elif path.suffix == ".tiled-project":
                refs = json.loads(path.read_text())["folders"]
            else:
                continue
            for ref in refs:
                target = (path.parent/ref).resolve()
                if (Path(ref).is_absolute() or not target.exists()
                        or not (target.is_relative_to(silo) or target.is_relative_to(assets))):
                    raise ValueError(f"{path}: invalid reference {ref!r} -> {target}")
