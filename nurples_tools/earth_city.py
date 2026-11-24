"""Flatten static structures onto ordered backgrounds; markers stay map bytes."""
from pathlib import Path
import xml.etree.ElementTree as ET
from PIL import Image


def compile_march(rows):
    """Group ranks separated by at most one spacer row; order is scroll-in order."""
    sizes = {2: (16, 16), 3: (32, 16), 4: (28, 28)}
    waves = []
    result = [list(row) for row in rows]
    for y, row in enumerate(rows):
        rank = [(x, gid) for x, gid in enumerate(row) if gid in sizes]
        if rank:
            if not waves or y - waves[-1][-1][0] > 2:
                waves.append([])
            waves[-1].append((y, rank))
            for x, _ in rank:
                result[y][x] = 0
    desc = []
    available = 0
    for wave in waves:
        first, last = wave[0][0], wave[-1][0]
        delay = max(0, available - first)
        if sum(len(rank) for _, rank in wave) > 40 or (last-first)*16 > 255:
            raise ValueError("Earth formation exceeds slot or entrance-timer capacity")
        left = min(x*16+8-sizes[g][0]//2 for _, rank in wave for x,g in rank)
        right = max(x*16+8+sizes[g][0]//2 for _, rank in wave for x,g in rank)
        travel = 256-right+left
        if left < 0 or right > 256 or not 1 <= travel <= 255:
            raise ValueError("Earth formation has no legal horizontal sweep")
        # At release of the final rank, earlier ranks have already descended
        # their row spacing. At half speed the last survivor must pass Y=335
        # BEFORE the next wave's admission (which precedes movement that tick).
        latest_exit = 0
        for y, rank in wave:
            wait = (last-y)*16
            dest = y+delay
            if dest >= len(result):
                raise ValueError("Earth wave extends beyond map")
            for x,g in rank:
                if result[dest][x]:
                    raise ValueError("Delayed formation conflicts with another marker")
                result[dest][x] = g
                top = wait+8-sizes[g][1]//2
                latest_exit = max(latest_exit, 2*(336-top))
            desc.append(((256-dest)%256, wait, left or travel, travel, 1 if left else 2))
        available = last+delay+latest_exit//16+1
    text = "\nearth_rank_count: equ " + str(len(desc)) + "\nearth_rank_programs:\n"
    text += "".join("\tdb " + ",".join(map(str, d)) + "\n" for d in desc)
    return tuple(tuple(row) for row in result), text


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
    for n,g in enumerate(cells):
        if g>16:
            i=g-1
            x,y=i%16*16,i//16*16
            plane.alpha_composite(sheet.crop((x,y,x+16,y+16)),(n%16*16,n//16*16))
            cells[n]=0
    if len(layers) == 2:
        spawn = layers[1]
        if (spawn.get("width"), spawn.get("height"), spawn.find("data").get("encoding")) != ("16", "256", "csv"):
            raise ValueError("Expected 16x256 CSV spawn layer")
        markers = [int(x) for x in spawn.findtext("data").strip().split(",")]
        if len(markers) != len(cells) or any(not 0 <= g <= 4 for g in markers):
            raise ValueError("Spawns accepts only empty cells or the four defined pad markers")
        if any(cells):
            raise ValueError("Move active markers from Structures to Spawns")
        cells = markers
    return plane,tuple(tuple(cells[n:n+16]) for n in range(4080,-1,-16))
