"""Flatten static structures onto ordered backgrounds; markers stay map bytes."""
from pathlib import Path
import xml.etree.ElementTree as ET
from PIL import Image


def earth_layers(source: Path):
    root = ET.parse(source).getroot()
    if (root.get("width"), root.get("height"), root.get("tilewidth"), root.get("tileheight")) != ("16","256","16","16"):
        raise ValueError("Earth requires 16x256 cells")
    layer, = root.findall("layer")
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
    return plane,tuple(tuple(cells[n:n+16]) for n in range(4080,-1,-16))
