"""Build the offline full-frame splash evaluation without touching game assets."""

import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
from xml.etree import ElementTree as ET

import numpy as np
from PIL import Image, ImageFilter

from nurples_tools.images import convert_to_agon_palette, img_to_rgba2, rgba2_to_img


ROOT = Path(__file__).resolve().parents[3]
HERE = Path(__file__).resolve().parent
ART = Path("assets/img/orig/ui")
MASTER = ART / "splash_bg_single_engine_cel.png"
FINISHED = ART / "splash_bg_single_engine_cel_320x240.xcf"
BAYER = ART / "splash_sprite_faithful_v3_512x384.png"
PALETTE = Path("build/palettes/Agon64.gpl")
SHA256 = {
    MASTER: "4ed644cab714b76adae1f2b063f271bbf8049aa70edc261dae4a596d7369effa",
    FINISHED: "562c7db659c24658f442af0ea141b56238623e04a892e2e0d63864a0dee8b414",
    BAYER: "a7166277186cc05e1806da6ac7a5b8ffe85597a43f09ac05511dacf7f20d627a",
    PALETTE: "c61dd1476415352e74add70f1ed0c81b1d141361a12b26587a6829ee66ec7f94",
    Path("potrace_bridge.py"): "9d813c7360663a79d8d8062ad4f69bcbdfb6f04801949da8e2761f38e9508f55",
    Path("cairo_flat_svg.py"): "d78f4ee46f18b8a7bddbdddf8de2be38a3447b3ea00ffed303e22b7cadb4c267",
}
SIZE = (512, 384)


def verify_input(path, key):
    actual = hashlib.sha256(path.read_bytes()).hexdigest()
    if actual != SHA256[key]:
        raise ValueError(f"Input hash mismatch: {path} ({actual})")


def bridge(jukebox, name):
    key = Path(f"{name}.py")
    path = jukebox / key
    verify_input(path, key)
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise ImportError(path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def read_palette(path):
    colors = []
    for line in path.read_text().splitlines():
        fields = line.split(maxsplit=3)
        if len(fields) >= 3 and all(field.isdecimal() for field in fields[:3]):
            colors.append(tuple(int(field) for field in fields[:3]))
    if len(colors) != 64 or len(set(colors)) != 64:
        raise ValueError("Expected 64 distinct Agon colors")
    return colors


def indexed(image, colors):
    """Lossless palette indexing. Unknown RGB values and any alpha are errors."""
    rgba = np.asarray(image.convert("RGBA"))
    if np.any(rgba[:, :, 3] != 255):
        raise ValueError("The splash must be opaque")
    rgb = rgba[:, :, :3]
    indices = np.full(rgb.shape[:2], 255, dtype=np.uint8)
    for index, color in enumerate(colors):
        indices[np.all(rgb == color, axis=2)] = index
    if np.any(indices == 255):
        raise ValueError("Off-palette splash pixel")
    return indices


def save_indexed(indices, colors, path):
    height, width = indices.shape
    preview = Image.frombytes("P", (width, height), indices.tobytes())
    preview.putpalette([value for color in colors for value in color])
    preview.save(path)


def make_svg(labels, colors, trace, path, coverage):
    """Paint broad colors first; later masks overwrite the colors below them."""
    height, width = labels.shape
    counts = np.bincount(labels.ravel(), minlength=len(colors))
    order = sorted(np.flatnonzero(counts), key=lambda i: (-counts[i], i))
    ranks = np.zeros(len(colors), dtype=np.uint8)
    ranks[order] = np.arange(len(order))
    rank_image = ranks[labels]

    def fill(index):
        return "#%02x%02x%02x" % colors[index]

    root = ET.Element("svg", xmlns="http://www.w3.org/2000/svg", width=str(width),
                      height=str(height), viewBox=f"0 0 {width} {height}")
    ET.SubElement(root, "title").text = "Purple Nurples — full-frame vector splash"
    ET.SubElement(root, "rect", id=f"color-{order[0]:02}-base", width=str(width),
                  height=str(height), fill=fill(order[0]))
    layers = []
    for rank, index in enumerate(order[1:], 1):
        mask = rank_image >= rank
        if coverage == "local":
            # Extend this fill two source pixels into later layers only.
            # Re-tracing every later shape here repeats most of the geometry.
            own = Image.fromarray((labels == index).astype(np.uint8) * 255)
            mask &= np.asarray(own.filter(ImageFilter.MaxFilter(5))) != 0
        paths = trace.trace(mask, alphamax=0.55, opttolerance=0.2)
        holes = components = segments = 0
        for component in paths:
            if component["sign"] != "+":
                continue
            children = [paths[i] for i in component["children"]]
            if any(child["sign"] != "-" for child in children):
                raise ValueError("Unexpected non-hole child in traced compound path")
            members = [component, *children]
            ET.SubElement(root, "path", {
                "id": f"color-{index:02}-part-{component['id']:04}",
                "fill": fill(index), "fill-rule": "evenodd",
                "d": " ".join(trace.path_data(member) for member in members),
            })
            components += 1
            holes += len(children)
            segments += sum(len(member["segments"]) for member in members)
        layers.append({"color": fill(index), "source_color_pixels": int(counts[index]),
                       "components": components, "holes": holes, "segments": segments})
    ET.indent(root)
    ET.ElementTree(root).write(path, encoding="utf-8", xml_declaration=True)
    return layers


def make_review(out, jukebox, coverage):
    for key in (MASTER, FINISHED, BAYER, PALETTE):
        verify_input(ROOT / key, key)
    trace = bridge(jukebox, "potrace_bridge")
    renderer = bridge(jukebox, "cairo_flat_svg")
    colors = read_palette(ROOT / PALETTE)
    out.mkdir(parents=True, exist_ok=False)
    shutil.copyfile(ROOT / BAYER, out / "bayer.png")
    subprocess.run(["convert", str(ROOT / FINISHED) + "[0]", "-strip",
                    str(out / "gimp-finish.png")], check=True)
    with Image.open(out / "gimp-finish.png") as finished:
        if finished.size != (320, 240):
            raise ValueError("Unexpected GIMP reference dimensions")
        indexed(finished, colors)
    with Image.open(out / "bayer.png") as bayer:
        if bayer.size != SIZE:
            raise ValueError("Unexpected Bayer reference dimensions")
        indexed(bayer, colors)

    with Image.open(ROOT / MASTER) as source:
        if source.size != (1448, 1086):
            raise ValueError("Unexpected cel master dimensions")
        master = source.convert("RGB")
    # A small prefilter removes isolated label noise at source resolution;
    # the same labels feed both the control and vector pass.
    labels = indexed(convert_to_agon_palette(master.filter(ImageFilter.MedianFilter(3)),
                                            64, "RGB"), colors)
    master.resize(SIZE, Image.Resampling.LANCZOS).save(out / "source.png")
    flat = np.asarray(Image.fromarray(labels).resize(SIZE, Image.Resampling.NEAREST))
    save_indexed(flat, colors, out / "flat.png")
    layers = make_svg(labels, colors, trace, out / "splash.svg", coverage)
    rendering = renderer.render(out / "splash.svg", out / "cairo.png", SIZE)
    with Image.open(out / "cairo.png") as raster:
        if raster.size != SIZE:
            raise ValueError("Unexpected vector raster dimensions")
        candidate = indexed(raster, colors)
    (out / "cairo.png").unlink()
    save_indexed(candidate, colors, out / "vector.png")
    with Image.open(out / "vector.png") as preview:
        img_to_rgba2(preview, out / "splash.rgba2")
        decoded = rgba2_to_img(out / "splash.rgba2", *SIZE)
        if decoded.convert("RGBA").tobytes() != preview.convert("RGBA").tobytes():
            raise ValueError("RGBA2222 round trip changed a pixel")

    # Inspect only registered rasters. The dim grayscale context is a guide;
    # magenta means the tracer moved or changed that pixel's fill.
    changed = np.not_equal(candidate, flat)
    palette = np.asarray(colors, dtype=np.uint8)
    gray = (palette[flat].astype(np.uint16).mean(axis=2) / 3).astype(np.uint8)
    diff = np.repeat(gray[:, :, None], 3, axis=2)
    diff[changed] = (255, 0, 170)
    Image.fromarray(diff).save(out / "difference.png")
    shutil.copyfile(HERE / "review-template.html", out / "index.html")
    report = {
        "target": list(SIZE), "opaque": True, "off_palette_pixels": 0,
        "source_filter": "3×3 RGB median at 1448×1086, then canonical Agon64 RGB; no dither",
        "coverage": coverage,
        "trace": {"version": trace.library().potrace_version().decode(),
                  "alphamax": 0.55, "opttolerance": 0.2, "turdsize": 0,
                  "turnpolicy": "BLACK", "inkscape_simplify": False},
        "rendering": rendering, "control_colors": int(np.unique(flat).size),
        "vector_colors": int(np.unique(candidate).size),
        "changed_pixels": int(changed.sum()),
        "changed_fraction": float(changed.mean()), "exact_rgba2222_roundtrip": True,
        "layers": layers,
        "hashes": {entry.name: hashlib.sha256(entry.read_bytes()).hexdigest()
                   for entry in sorted(out.iterdir()) if entry.is_file()},
    }
    (out / "verification.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=Path, required=True, help="new output directory")
    parser.add_argument("--jukebox", type=Path, default=ROOT.parent / "AgonJukebox" /
                        "docs/tasks/SKIN-013/flat-color-full-02", help="pinned Jukebox bridges")
    parser.add_argument("--coverage", choices=("local", "nested"), default="local",
                        help="mask overlap strategy; nested is the large reference trial")
    args = parser.parse_args()
    make_review(args.out, args.jukebox, args.coverage)


if __name__ == "__main__":
    main()
