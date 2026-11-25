"""Read Earth scenery, static art, and spawn cells as independent inputs."""
from pathlib import Path
import xml.etree.ElementTree as ET
from PIL import Image

EARTH_TILE_BASE = 0x0600  # low byte is the runtime tile ID; zero is transparent


def earth_layers(source: Path):
    root = ET.parse(source).getroot()
    if (root.get("width"), root.get("height"), root.get("tilewidth"), root.get("tileheight")) != ("16","256","16","16"):
        raise ValueError("Earth requires 16x256 cells")
    layers = root.findall("layer")
    if [item.get("name") for item in layers] not in (["Structures"], ["Structures", "Spawns"]):
        raise ValueError("Expected Structures and optional Spawns above it")
    layer = layers[0]
    data = layer.find("data")
    ts, = root.findall("tileset")
    if ts.get("firstgid") != "1" or data.get("encoding") != "csv":
        raise ValueError("Unsupported Earth layer format")
    spec = source.parent / ts.attrib["source"]
    atlas = ET.parse(spec).getroot()
    sheet = Image.open(spec.parent / atlas.find("image").attrib["source"]).convert("RGBA")
    plane = Image.open(source.parent / root.find("imagelayer/image").attrib["source"]).convert("RGBA")
    if plane.size != (256,4096) or sheet.width != 256:
        raise ValueError("Invalid Earth artwork dimensions")
    cells = [int(x) for x in data.text.strip().split(",")]
    if len(cells)!=4096 or not all(0<=g<=int(atlas.attrib["tilecount"]) for g in cells):
        raise ValueError("Invalid Earth map cells or transform flags")
    if any(g > 255 for g in cells):
        raise ValueError("Earth structure GID does not fit a runtime tile byte")
    structures = [g if g > 16 else 0 for g in cells]
    markers = [g if g <= 16 else 0 for g in cells]
    if len(layers) == 2:
        if any(markers):
            raise ValueError("Move active markers from Structures to Spawns")
        spawn = layers[1]
        if (spawn.get("width"), spawn.get("height"), spawn.find("data").get("encoding")) != ("16", "256", "csv"):
            raise ValueError("Expected 16x256 CSV spawn layer")
        markers = [int(x) for x in spawn.findtext("data").strip().split(",")]
        if len(markers) != len(cells) or any(not 0 <= g <= 4 for g in markers):
            raise ValueError("Spawns accepts only empty cells or the four defined pad markers")
    def rows(values):
        return tuple(tuple(values[n:n+16]) for n in range(4080,-1,-16))
    return plane,sheet,rows(markers),rows(structures)
