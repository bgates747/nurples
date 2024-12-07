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

    # Create a completely transparent tile for index 0
    transparent_tile = Image.new("RGBA", (tile_width, tile_height), (255, 255, 255, 0))
    transparent_tile_path = os.path.join(level_dir, "000.png")  # 3-digit decimal
    transparent_tile.save(transparent_tile_path)
    print(f"Created transparent tile at {transparent_tile_path}")

    img = Image.open(source_path)
    crop_width = tiles_x * h_pitch
    crop_height = tiles_y * v_pitch
    cropped_img = img.crop((start_x, start_y, start_x + crop_width, start_y + crop_height))
    unique_tiles = {}
    tile_map = []
    tile_count = 1  # Start from 1 because 0 is reserved for the null tile

    for row in range(tiles_y):
        row_tiles = []
        for col in range(tiles_x):
            x = col * h_pitch
            y = row * v_pitch
            tile = cropped_img.crop((x, y, x + tile_width, y + tile_height))
            tile_hash = hashlib.md5(tile.tobytes()).hexdigest()
            if tile_hash not in unique_tiles:
                output_path = os.path.join(level_dir, f"{tile_count:03}.png")  # 3-digit decimal
                tile.save(output_path)
                unique_tiles[tile_hash] = tile_count
                tile_count += 1
            row_tiles.append(f"{unique_tiles[tile_hash]:03}")  # 3-digit decimal
        tile_map.append(row_tiles)

    map_filepath = os.path.join(target_dir, f"{base_name}_{set_num}_map.csv")
    with open(map_filepath, "w") as csv_file:
        csv_file.write("\n".join(",".join(row) for row in tile_map))
    print(f"Saved CSV map to {map_filepath}")

    xml_filepath = os.path.join(target_dir, f"{base_name}_{set_num}_files.xml")
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

    # Include the transparent tile as the first entry
    tile_element = ET.SubElement(tiles_element, "Tile000")
    tile_element.text = transparent_tile_path

    for tile_hash, idx in unique_tiles.items():
        tile_file = os.path.join(level_dir, f"{idx:03}.png")  # 3-digit decimal
        tile_element = ET.SubElement(tiles_element, f"Tile{idx:03}")
        tile_element.text = tile_file

    rough_string = ET.tostring(root, encoding="utf-8")
    parsed = minidom.parseString(rough_string)
    pretty_xml = parsed.toprettyxml(indent="  ")
    with open(xml_filepath, "w") as xml_file:
        xml_file.write(pretty_xml)
    print(f"Saved files.xml to {xml_filepath}")

def chop_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height, h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, set_num):
    """
    Chops tiles from a source image, saves non-transparent tiles to the target directory, and generates XML and CSV files.
    Separate counters for tile index (consistent grid mapping) and file count (non-transparent tiles saved).
    """
    source_path = os.path.join(source_dir, file_name)
    level_dir = os.path.join(target_dir, str(set_num))
    if os.path.exists(level_dir):
        shutil.rmtree(level_dir)
    os.makedirs(level_dir)

    img = Image.open(source_path)
    crop_width = tiles_x * h_pitch
    crop_height = tiles_y * v_pitch
    cropped_img = img.crop((start_x, start_y, start_x + crop_width, start_y + crop_height))
    tile_map = []
    saved_tiles = {}  # Dictionary to store mappings of indices to file paths

    tile_index = 1  # Start from 1 because 0 is reserved for the null tile
    file_count = 0  # Count only non-transparent tiles

    for row in range(tiles_y):
        row_tiles = []
        for col in range(tiles_x):
            x = col * h_pitch
            y = row * v_pitch
            tile = cropped_img.crop((x, y, x + tile_width, y + tile_height))

            # Check if the tile is completely transparent
            if tile.getextrema()[3][1] == 0:  # Alpha channel's max value is 0
                print(f"Tile at index {tile_index:03} is completely transparent. Skipping.")
                row_tiles.append("000")  # Mark this grid position as null
            else:
                # Save non-transparent tiles
                output_path = os.path.join(level_dir, f"{tile_index:03}.png")  # 3-digit decimal
                tile.save(output_path)
                saved_tiles[tile_index] = output_path  # Map tile index to its file path
                print(f"Saved tile {tile_index:03} to {output_path}")
                row_tiles.append(f"{tile_index:03}")  # Record this index in the map
                file_count += 1  # Increment file count only for non-transparent tiles
            tile_index += 1  # Always bump the tile index, regardless of transparency
        tile_map.append(row_tiles)

    # Save the CSV map
    map_filepath = os.path.join(target_dir, f"{base_name}_{set_num}_map.csv")
    with open(map_filepath, "w") as csv_file:
        csv_file.write("\n".join(",".join(row) for row in tile_map))
    print(f"Saved CSV map to {map_filepath}")

    # Save the XML file
    xml_filepath = os.path.join(target_dir, f"{base_name}_{set_num}_files.xml")
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
    ET.SubElement(root, "FileCount").text = str(file_count)  # Total non-transparent tiles
    tiles_element = ET.SubElement(root, "Tiles")

    # Record only non-transparent tiles in the XML
    for idx, path in saved_tiles.items():
        tile_element = ET.SubElement(tiles_element, f"Tile{idx:03}")
        tile_element.text = path

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
        ET.SubElement(tile_set_header, "BaseBufferId").text = str(current_bufferId)  # Decimal
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
        asm_bufferIds.append(f"BUF_{base_name.upper()}_{set_num:02}_00: equ {base_bufferId}")  # Decimal buffer ID

        tiles_tree = ET.parse(tiles_file)
        tiles_root = tiles_tree.getroot()

        target_dir = tiles_root.find("TargetDir").text

        for tile_idx, tile in enumerate(tiles_root.find("Tiles")):
            tile_filename = tile.text
            image_name = os.path.splitext(os.path.basename(tile_filename))[0]
            asm_label = f"fn_{base_name.lower()}_{set_num}_{int(image_name, 16):03}"  # Handle hexadecimal
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
                row_data = [f"{int(value.strip()):03}" for value in row if value.strip().isdigit()]  # 3-digit decimal
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

def generate_asm_tiled_level(tiled_map_filepath, asm_levels_filepath, tileset_number, level_number, bufferId):
    tree = ET.parse(tiled_map_filepath)
    root = tree.getroot()
    num_cols = root.attrib['width']
    num_rows = root.attrib['height']
    layer = root.find('layer')
    csv_data = layer.find('data').text.strip()
    rows = [row.split(',') for row in csv_data.split('\n')]
    rows = list(reversed(rows))
    level_data = []
    for row in rows:
        row_data = [f"{int(value.strip()):03}" for value in row if value.strip().isdigit()]
        level_data.append(f"\tdb {','.join(row_data)}")
    tileset_base_name = f"tileset_{tileset_number:02X}"
    level_label = f"{tileset_base_name}_level_{level_number:02X}"
    level_asm = (
        f"{level_label}: ; Level {level_number}\n"
        f"\tdb {num_cols}          ; num cols\n"
        f"\tdl {num_rows}          ; num rows\n"
        f"\tdl {bufferId} ; base bufferId\n"
        + "\n".join(level_data)
    )
    with open(asm_levels_filepath, "w") as asm_file:
        asm_file.write(f"; Generated from {tiled_map_filepath}\n\n")
        asm_file.write(f"{tileset_base_name}_num_levels: db 1\n\n")  # Only one level
        asm_file.write(f"{tileset_base_name}_levels:\n")
        asm_file.write(f"\tdl {level_label}\n")
        asm_file.write("\tdl 0 ; list terminator\n\n")
        asm_file.write(level_asm)
    print(f"Assembly levels file created at {asm_levels_filepath}")

def main(bufferId, tile_width, tile_height, h_pitch, v_pitch, base_name, tiles_x, tiles_y, ranges, asm_src_dir, source_dir, target_dir,tileset_number):
    file_name = f"{base_name}{tileset_number}.png"
    if not os.path.exists(target_dir):
        os.makedirs(target_dir)

    for set_num, (start_x, start_y) in enumerate(ranges):
        chop_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height, h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, set_num)

    asm_images_filepath = f"{asm_src_dir}/images_tiles_{base_name}.inc"
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

    asm_levels_filepath = f"{asm_src_dir}/levels_{base_name}.inc"
    generate_asm_levels(combined_xml_filepath, asm_levels_filepath)

    return next_bufferId

if __name__ == "__main__":
    root_src_dir = "tiles"
    asm_src_dir = "src/asm"
    bufferId = 512
    tile_width = 16
    tile_height = 16
    h_pitch = tile_width
    v_pitch = tile_height

    base_name = "dg"
    tileset_number = 0
    tiles_x = 16
    tiles_y = 16
    ranges = [(0, 0)]
    source_dir = f"{root_src_dir}/{base_name}"
    target_dir = f"{root_src_dir}/{base_name}"
    next_bufferId = main(bufferId, tile_width, tile_height, h_pitch, v_pitch, base_name, tiles_x, tiles_y, ranges, asm_src_dir, source_dir, target_dir, tileset_number)
    print(f"Next buffer ID: {next_bufferId}")

    level_number = 0
    tiled_map_filepath = "tiles/dg/dg0_00.tmx"
    asm_levels_tiled_filepath = f"{asm_src_dir}/levels_tileset_{tileset_number}.inc"
    generate_asm_tiled_level(tiled_map_filepath, asm_levels_tiled_filepath, tileset_number, level_number, bufferId)