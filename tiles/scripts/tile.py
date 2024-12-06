import os, shutil
from PIL import Image
import hashlib
import xml.etree.ElementTree as ET
from xml.dom import minidom
from agonImages import img_to_rgba2, img_to_rgba8, convert_to_agon_palette
import csv

def chop_and_deduplicate_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height, h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, set_num):
    source_path = os.path.join(source_dir, file_name)
    level_dir = os.path.join(target_dir, str(set_num))
    if os.path.exists(level_dir):
        shutil.rmtree(level_dir)
    os.makedirs(level_dir)

    img = Image.open(source_path)
    crop_width = tiles_x * h_pitch
    crop_height = tiles_y * v_pitch
    cropped_img = img.crop((start_x, start_y, start_x + crop_width, start_y + crop_height))
    unique_tiles = {}
    tile_map = []
    tile_count = 0
    set_num_hex = f"{set_num:1X}"

    for row in range(tiles_y):
        row_tiles = []
        for col in range(tiles_x):
            x = col * h_pitch
            y = row * v_pitch
            tile = cropped_img.crop((x, y, x + tile_width, y + tile_height))
            tile_hash = hashlib.md5(tile.tobytes()).hexdigest()
            if tile_hash not in unique_tiles:
                output_path = os.path.join(level_dir, f"{tile_count:02X}.png")
                tile.save(output_path)
                unique_tiles[tile_hash] = tile_count
                tile_count += 1
            row_tiles.append(f"{unique_tiles[tile_hash]:02X}")
        tile_map.append(row_tiles)

    map_filepath = os.path.join(target_dir, f"{base_name}_{set_num_hex}_map.csv")
    with open(map_filepath, "w") as csv_file:
        csv_file.write("\n".join(",".join(row) for row in tile_map))
    print(f"Saved CSV map to {map_filepath}")

    xml_filepath = os.path.join(target_dir, f"{base_name}_{set_num_hex}_files.xml")
    root = ET.Element("TileSet")
    ET.SubElement(root, "SourceDir").text = source_dir
    ET.SubElement(root, "TargetDir").text = target_dir
    ET.SubElement(root, "BaseName").text = base_name
    ET.SubElement(root, "FileName").text = file_name
    ET.SubElement(root, "TileWidth").text = str(tile_width)
    ET.SubElement(root, "TileHeight").text = str(tile_height)
    ET.SubElement(root, "HPitch").text = str(h_pitch)
    ET.SubElement(root, "VPitch").text = str(v_pitch)
    ET.SubElement(root, "TilesX").text = str(tiles_x)
    ET.SubElement(root, "TilesY").text = str(tiles_y)
    ET.SubElement(root, "StartX").text = str(start_x)
    ET.SubElement(root, "StartY").text = str(start_y)
    ET.SubElement(root, "SetNum").text = str(set_num)
    ET.SubElement(root, "MapFile").text = map_filepath
    ET.SubElement(root, "TilesFile").text = xml_filepath
    ET.SubElement(root, "FileCount").text = str(tile_count)
    tiles_element = ET.SubElement(root, "Tiles")

    for tile_hash, idx in unique_tiles.items():
        tile_file = os.path.join(level_dir, f"{idx:02X}.png")
        tile_element = ET.SubElement(tiles_element, f"Tile{idx:02X}")
        tile_element.text = tile_file

    rough_string = ET.tostring(root, encoding="utf-8")
    parsed = minidom.parseString(rough_string)
    pretty_xml = parsed.toprettyxml(indent="  ")
    with open(xml_filepath, "w") as xml_file:
        xml_file.write(pretty_xml)
    print(f"Saved files.xml to {xml_filepath}")

def generate_combined_xml(combined_xml_filepath, base_name, target_dir, bufferId):
    xml_files = []
    for root, dirs, files in os.walk(target_dir):
        for f in files:
            if f.startswith(base_name) and f.endswith("_files.xml"):
                xml_files.append(os.path.join(root, f))
    xml_files.sort()
    combined_root = ET.Element("TileSets")
    current_bufferId = bufferId
    for xml_path in xml_files:
        tree = ET.parse(xml_path)
        root = tree.getroot()
        tile_set_header = ET.SubElement(combined_root, "TileSet")
        for child in root:
            if child.tag != "Tiles":
                ET.SubElement(tile_set_header, child.tag).text = child.text
        file_count = int(root.find("FileCount").text)
        ET.SubElement(tile_set_header, "BaseBufferId").text = str(current_bufferId)
        current_bufferId += file_count
        current_bufferId = ((current_bufferId + 255) // 256) * 256
    rough_string = ET.tostring(combined_root, encoding="utf-8")
    parsed = minidom.parseString(rough_string)
    pretty_xml = parsed.toprettyxml(indent="  ")
    with open(combined_xml_filepath, "w") as xml_file:
        xml_file.write(pretty_xml)
    print(f"Combined XML file created at {combined_xml_filepath}")
    return current_bufferId

def generate_asm_img_load(combined_xml_filepath, asm_images_filepath, asm_img_dir):
    tree = ET.parse(combined_xml_filepath)
    root = tree.getroot()
    asm_bufferIds = []
    asm_image_list = []
    asm_file_list = []
    total_images = 0
    base_name_for_header = None
    for tile_set in root.findall("TileSet"):
        if base_name_for_header is None:
            base_name_for_header = tile_set.find("BaseName").text
        base_bufferId = int(tile_set.find("BaseBufferId").text)
        tiles_file = tile_set.find("TilesFile").text
        file_count = int(tile_set.find("FileCount").text)
        base_name = tile_set.find("BaseName").text
        set_num = int(tile_set.find("SetNum").text)
        asm_bufferIds.append(f"BUF_{base_name.upper()}_{set_num:1X}_00: equ {base_bufferId}")

        tiles_tree = ET.parse(tiles_file)
        tiles_root = tiles_tree.getroot()

        target_dir = tiles_root.find("TargetDir").text

        for tile_idx, tile in enumerate(tiles_root.find("Tiles")):
            tile_filename = tile.text
            image_name = os.path.splitext(os.path.basename(tile_filename))[0]  
            asm_label = f"fn_{base_name.lower()}_{set_num}_{image_name}"

            bufferId = base_bufferId + tile_idx
            tile_width = int(tile_set.find("TileWidth").text)
            tile_height = int(tile_set.find("TileHeight").text)
            tile_size = tile_width * tile_height

            asm_image_list.append(
                f"\tdl 1, {tile_width}, {tile_height}, {tile_size}, {asm_label}, {bufferId}"
            )
            relative_path = os.path.relpath(tile_filename, target_dir)
            base_no_ext = os.path.splitext(relative_path)[0]
            rgba_filename = os.path.join(asm_img_dir, base_no_ext + ".rgba2").replace("\\", "/")

            asm_file_list.append(f"{asm_label}: db \"{rgba_filename}\",0")

        total_images += file_count

    if base_name_for_header is None:
        base_name_for_header = "unknown"
    else:
        base_name_for_header = base_name_for_header.lower()

    with open(asm_images_filepath, "w") as asm_images_file:
        asm_images_file.write(f"; Generated by tiles_{base_name_for_header}.py\n\n")
        asm_images_file.write(f"tiles_{base_name_for_header}_num_images: equ {total_images}\n\n")
        asm_images_file.write("; bufferIds:\n")
        asm_images_file.write("\n".join(asm_bufferIds) + "\n\n")
        asm_images_file.write(f"tiles_{base_name_for_header}_image_list: ; type; width; height; size; filename; bufferId:\n")
        asm_images_file.write("\n".join(asm_image_list) + "\n\n")
        asm_images_file.write(f"tiles_{base_name_for_header}_files_list: ; filename:\n")
        asm_images_file.write("\n".join(asm_file_list) + "\n")
    print(f"Assembly images file created at {asm_images_filepath}")

def make_images(combined_xml_filepath, rgba_target_dir, image_type, do_palette, palette_conv_type, transparent_rgb):
    tree = ET.parse(combined_xml_filepath)
    root = tree.getroot()
    target_dir = None
    first_set = root.find("TileSet")
    if first_set is not None:
        target_dir = first_set.find("TargetDir").text

    for tile_set in root.findall("TileSet"):
        tiles_file = tile_set.find("TilesFile").text
        tiles_tree = ET.parse(tiles_file)
        tiles_root = tiles_tree.getroot()
        for tile in tiles_root.find("Tiles"):
            input_image_path = tile.text
            if target_dir:
                relative_path = os.path.relpath(input_image_path, target_dir)
            else:
                relative_path = os.path.basename(input_image_path)

            base_name_no_ext = os.path.splitext(relative_path)[0]
            if image_type == 1:
                rgba_filepath = os.path.join(rgba_target_dir, base_name_no_ext + ".rgba2")
            else:
                rgba_filepath = os.path.join(rgba_target_dir, base_name_no_ext + ".rgba8")

            os.makedirs(os.path.dirname(rgba_filepath), exist_ok=True)

            with Image.open(input_image_path) as img:
                if do_palette:
                    img = convert_to_agon_palette(img, 64, palette_conv_type, transparent_rgb)
                if image_type == 1:
                    img_to_rgba2(img, rgba_filepath)
                else:
                    img_to_rgba8(img, rgba_filepath)

def generate_asm_levels(combined_xml_filepath, asm_levels_filepath):
    tree = ET.parse(combined_xml_filepath)
    root = tree.getroot()
    num_levels = 0
    levels_list = []
    levels_data = []
    base_name_for_header = None
    for set_num, tile_set in enumerate(root.findall("TileSet")):
        if base_name_for_header is None:
            base_name_for_header = tile_set.find("BaseName").text
        num_levels += 1
        map_file = tile_set.find("MapFile").text
        num_cols = tile_set.find("TilesX").text
        num_rows = tile_set.find("TilesY").text
        base_bufferId = int(tile_set.find("BaseBufferId").text)
        base_name = tile_set.find("BaseName").text.upper()
        level_data = []
        with open(map_file, "r") as csv_file:
            reader = csv.reader(csv_file)
            for row in reversed(list(reader)):  # Reverse rows for top-scroller
                row_data = [f"0x{value.upper()}" for value in row]
                level_data.append(f"\tdb {','.join(row_data)}")
        levels_list.append(f"\tdl tiles_{base_name.lower()}_level_{set_num}")
        levels_data.append(
            f"tiles_{base_name.lower()}_level_{set_num}: ; Level {set_num}\n"
            f"\tdb {num_cols}          ; num cols\n"
            f"\tdl {num_rows}          ; num rows\n"
            f"\tdl {base_bufferId} ; base bufferId\n"
            + "\n".join(level_data)
        )
    if base_name_for_header is None:
        base_name_for_header = "unknown"
    else:
        base_name_for_header = base_name_for_header.lower()
    with open(asm_levels_filepath, "w") as asm_levels_file:
        asm_levels_file.write(f"; Generated by tiles_{base_name_for_header}_levels.py\n\n")
        asm_levels_file.write(f"tiles_{base_name_for_header}_num_levels: db {num_levels}\n\n")
        asm_levels_file.write(f"tiles_{base_name_for_header}_levels:\n")
        asm_levels_file.write("\n".join(levels_list) + "\n")
        asm_levels_file.write("\tdl 0 ; list terminator\n\n")
        asm_levels_file.write("\n".join(levels_data) + "\n")
    print(f"Assembly levels file created at {asm_levels_filepath}")

def main(bufferId, tile_width, tile_height, h_pitch, v_pitch, base_name, tiles_x, tiles_y, ranges):
    file_name = f"{base_name}.png"
    source_dir = f"tiles/{base_name}"
    target_dir = f"tiles/{base_name}"
    if not os.path.exists(target_dir):
        os.makedirs(target_dir)

    for set_num, (start_x, start_y) in enumerate(ranges):
        chop_and_deduplicate_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height, h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, set_num)

    asm_images_filepath = f"src/asm/images_tiles_{base_name}.inc"
    combined_xml_filepath = os.path.join(target_dir, f"{base_name}.xml")
    asm_img_dir = f"tiles/{base_name}"
    next_bufferId = generate_combined_xml(combined_xml_filepath, base_name, target_dir, bufferId)
    generate_asm_img_load(combined_xml_filepath, asm_images_filepath, asm_img_dir)
    
    image_type = 1
    do_palette = False
    palette_conv_type = 'rgb'
    transparent_rgb = (0, 0, 0, 0)
    rgba_target_dir = f"tgt/tiles/{base_name}"
    if os.path.exists(rgba_target_dir):
        shutil.rmtree(rgba_target_dir)
    os.makedirs(rgba_target_dir)
    make_images(combined_xml_filepath, rgba_target_dir, image_type, do_palette, palette_conv_type, transparent_rgb)

    asm_levels_filepath = f"src/asm/levels_{base_name}.inc"
    generate_asm_levels(combined_xml_filepath, asm_levels_filepath)

    return next_bufferId

if __name__ == "__main__":
    next_bufferId = 512
    tile_width = 16
    tile_height = 16
    h_pitch = tile_width
    v_pitch = tile_height

    base_name = "dg"
    tiles_x = 16
    tiles_y = 32
    ranges = [(256, 1024), (256, 512), (256, 0), (0, 1024), (0, 512), (0, 0)]
    # ranges = [(256, 1024)]
    next_bufferId = main(next_bufferId, tile_width, tile_height, h_pitch, v_pitch, base_name, tiles_x, tiles_y, ranges)
    print(f"Next buffer ID: {next_bufferId}")

    base_name = "xevious"
    tiles_x = 16
    tiles_y = 128
    ranges = [(768, 0), (512, 0), (256, 0), (0, 0)]
    # ranges = [(768, 0)]
    next_bufferId = main(next_bufferId, tile_width, tile_height, h_pitch, v_pitch, base_name, tiles_x, tiles_y, ranges)
    print(f"Next buffer ID: {next_bufferId}")