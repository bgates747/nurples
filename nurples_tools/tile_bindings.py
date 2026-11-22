"""Translate DG editor GIDs to permanent runtime bytes (zero stays empty)."""

import xml.etree.ElementTree as ET

PROPERTY = "nurples_runtime_bindings"
IDENTITY = tuple(range(256))


def bindings(tsx: ET.Element) -> tuple[int, ...]:
    """Unmentioned slots are identity; overrides must form a permutation."""
    props = tsx.findall(f"./properties/property[@name='{PROPERTY}']")
    values = list(IDENTITY)
    if not props:
        return IDENTITY
    if len(props) != 1:
        raise ValueError("duplicate DG bindings property")
    seen = set()
    for pair in props[0].attrib.get("value", "").split(","):
        gid, runtime = map(int, pair.split(":"))
        if gid in seen or not (1 <= gid <= 255 and 1 <= runtime <= 255):
            raise ValueError(f"invalid DG binding: {pair}")
        seen.add(gid)
        values[gid] = runtime
    if len(set(values)) != 256:
        raise ValueError("DG bindings must preserve distinct runtime IDs")
    return tuple(values)


def exchange(tsx: ET.Element, a: int, b: int) -> None:
    values = list(bindings(tsx))
    values[a], values[b] = values[b], values[a]
    pairs = ",".join(f"{i}:{v}" for i, v in enumerate(values) if i != v)
    props = tsx.find("properties")
    if props is None:
        props = ET.Element("properties")
        tsx.insert(0, props)
    prop = props.find(f"property[@name='{PROPERTY}']")
    if prop is not None:
        props.remove(prop)
    if pairs:
        ET.SubElement(props, "property", name=PROPERTY, value=pairs)
    if not len(props):
        tsx.remove(props)
