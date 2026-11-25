"""Read Earth scenery, static art, and spawn cells as independent inputs."""
from pathlib import Path
import xml.etree.ElementTree as ET
from PIL import Image

EARTH_TILE_BASE = 0x0600  # low byte is the runtime tile ID; zero is transparent


def earth_layers(source: Path):
    root = ET.parse(source).getroot()
    if (root.get("width"), root.get("height"), root.get("tilewidth"), root.get("tileheight")) != ("16","256","16","16"):
        raise ValueError("Earth requires 16x256 cells")
    layers = [e for e in root if e.tag in ("layer", "imagelayer", "objectgroup", "group")]
    if [(e.tag, e.get("name")) for e in layers] != [
            ("imagelayer", "Background"), ("layer", "Tiles"), ("layer", "Spawns")]:
        raise ValueError(f"{source}: expected Background, Tiles and Spawns in drawing order")
    for e in layers:
        if any(float(e.get(k, "0")) for k in ("x", "y", "offsetx", "offsety")):
            raise ValueError(f"{source}: nonzero layer offset")
    def cells_of(layer):
        data = layer.find("data")
        if ((layer.get("width"), layer.get("height")) != ("16", "256")
                or data is None or data.attrib != {"encoding": "csv"} or len(data)):
            raise ValueError(f"{source}: expected finite 16x256 CSV for {layer.get('name')}")
        rows = [s.removesuffix(",").split(",") for s in data.text.strip().splitlines()]
        if len(rows) != 256 or any(len(row) != 16 for row in rows):
            raise ValueError(f"{source}: invalid row dimensions on {layer.get('name')}")
        return [int(x) for row in rows for x in row]
    ts, = root.findall("tileset")
    if ts.get("firstgid") != "1":
        raise ValueError("Unsupported Earth layer format")
    spec = source.parent / ts.attrib["source"]
    atlas = ET.parse(spec).getroot()
    sheet = Image.open(spec.parent / atlas.find("image").attrib["source"]).convert("RGBA")
    plane = Image.open(source.parent / root.find("imagelayer/image").attrib["source"]).convert("RGBA")
    if plane.size != (256,4096) or sheet.width != 256:
        raise ValueError("Invalid Earth artwork dimensions")
    structures, markers = map(cells_of, layers[1:])
    if any(g and not 17 <= g <= min(255, int(atlas.attrib["tilecount"])) for g in structures):
        raise ValueError(f"{source}: Tiles requires scenery, not sprite markers or transform flags")
    if any(not 0 <= g <= 4 for g in markers):
        raise ValueError(f"{source}: Spawns accepts only the four defined sprite markers")
    def rows(values):
        return tuple(tuple(values[n:n+16]) for n in range(4080,-1,-16))
    return plane,sheet,rows(markers),rows(structures)
