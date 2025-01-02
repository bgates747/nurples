import os, shutil
from PIL import Image
import hashlib
import xml.etree.ElementTree as ET
from xml.dom import minidom
from agonImages import img_to_rgba2, img_to_rgba8, convert_to_agon_palette
import csv

def chop_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height, h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, set_num, bufferId):
    if not os.path.exists(target_dir):
        os.makedirs(target_dir)

    level_dir = os.path.join(target_dir, str(set_num))
    if os.path.exists(level_dir):
        shutil.rmtree(level_dir)
    os.makedirs(level_dir)

    # Create the null tile at index 0
    null_tile = Image.new("RGBA", (tile_width, tile_height), (0, 0, 0, 0))
    null_tile_path = os.path.join(level_dir, "000.png")
    null_tile.save(null_tile_path)

    # Load and crop the source image
    img = Image.open(os.path.join(source_dir, file_name))
    img = img.convert("RGBA")
    cropped_img = img.crop((start_x, start_y, start_x + tiles_x * h_pitch, start_y + tiles_y * v_pitch))

    tile_map = []
    # saved_tiles maps tile_index to its file path
    # Include the null tile in saved_tiles
    saved_tiles = {0: null_tile_path}

    # Start indexing actual tiles at 1, null tile is 0
    tile_index = 1
    # We have 1 file already: the null tile
    file_count = 1

    for row in range(tiles_y):
        row_tiles = []
        for col in range(tiles_x):
            x = col * h_pitch
            y = row * v_pitch
            tile = cropped_img.crop((x, y, x + tile_width, y + tile_height))
            # Check transparency
            # tile is assumed to be RGBA
            extrema = tile.getextrema()
            if extrema and extrema[3][1] == 0:
                # Fully transparent - use the null tile index 0 in the CSV
                print(f"Tile at index {tile_index:03} is completely transparent. Skipping.")
                row_tiles.append("000")
            else:
                # Non-transparent tile
                filename_str = f"{tile_index:03}.png"
                output_path = os.path.join(level_dir, filename_str)
                tile.save(output_path)
                saved_tiles[tile_index] = output_path
                print(f"Saved tile {tile_index:03} to {output_path}")
                row_tiles.append(f"{tile_index:03}")
                file_count += 1
            tile_index += 1
        tile_map.append(row_tiles)

    # Save the CSV map
    map_filepath = os.path.join(target_dir, f"{base_name}_{set_num}_map.csv")
    with open(map_filepath, "w") as csv_file:
        for row in tile_map:
            csv_file.write(",".join(row) + "\n")
    print(f"Saved CSV map to {map_filepath}")

    # Save the XML file
    xml_filepath = os.path.join(target_dir, f"{base_name}_{set_num}_files.xml")
    root = ET.Element("TileSet")
    ET.SubElement(root, "BaseName").text = base_name
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
    ET.SubElement(root, "FileCount").text = str(file_count)
    ET.SubElement(root, "BaseBufferId").text = str(bufferId)
    tiles_element = ET.SubElement(root, "Tiles")

    # Include the null tile (index 0) and all non-transparent tiles
    for idx in sorted(saved_tiles.keys()):
        tile_element = ET.SubElement(tiles_element, "Tile")
        tile_element.set("index", str(idx))
        tile_element.text = saved_tiles[idx]

    rough_string = ET.tostring(root, encoding="utf-8")
    parsed = minidom.parseString(rough_string)
    pretty_xml = parsed.toprettyxml(indent="  ")
    with open(xml_filepath, "w") as xml_file:
        xml_file.write(pretty_xml)
    print(f"Saved files.xml to {xml_filepath}")

def generate_asm_img_load(xml_tile_filepath, asm_images_filepath, asm_img_dir):
    """
    Generates assembly image load instructions based on an updated XML tile file schema.
    """
    tree = ET.parse(xml_tile_filepath)
    root = tree.getroot()

    asm_bufferIds = []
    asm_image_list = []
    asm_file_list = []
    total_images = 0

    # Parse the tile set metadata
    base_name = root.find("BaseName").text.lower()
    base_bufferId = int(root.find("BaseBufferId").text)
    tile_width = int(root.find("TileWidth").text)
    tile_height = int(root.find("TileHeight").text)
    tiles = root.find("Tiles")
    target_dir = root.find("TargetDir").text

    # Process each tile
    for tile in tiles.findall("Tile"):
        tile_index = int(tile.attrib["index"])
        tile_filename = tile.text
        bufferId = base_bufferId + tile_index  # Simplified bufferId calculation
        image_name = os.path.splitext(os.path.basename(tile_filename))[0]
        asm_label = f"fn_{base_name}_{tile_index:03}"  # Create label

        # Add to assembly lists
        tile_size = tile_width * tile_height
        asm_image_list.append(
            f"\tdl 1, {tile_width}, {tile_height}, {tile_size}, {asm_label}, {bufferId}"
        )
        relative_path = os.path.relpath(tile_filename, target_dir)
        rgba_filename = os.path.join(asm_img_dir, os.path.splitext(relative_path)[0] + ".rgba2").replace("\\", "/")
        asm_file_list.append(f"{asm_label}: db \"{rgba_filename}\",0")

        total_images += 1

    # Write to the assembly file
    with open(asm_images_filepath, "w") as asm_images_file:
        asm_images_file.write(f"; Generated by tiles_{base_name}.py\n\n")
        asm_images_file.write(f"tiles_{base_name}_num_images: equ {total_images}\n\n")
        asm_images_file.write("; bufferIds:\n")
        asm_images_file.write("\n".join(asm_bufferIds) + "\n\n")
        asm_images_file.write(f"tiles_{base_name}_image_list: ; type; width; height; size; filename; bufferId:\n")
        asm_images_file.write("\n".join(asm_image_list) + "\n\n")
        asm_images_file.write(f"tiles_{base_name}_files_list: ; filename:\n")
        asm_images_file.write("\n".join(asm_file_list) + "\n")

    print(f"Assembly images file created at {asm_images_filepath}")

def make_images(xml_tile_filepath, rgba_target_dir, image_type, do_palette, palette_conv_type, transparent_rgb):
    # Parse the XML file
    tree = ET.parse(xml_tile_filepath)
    root = tree.getroot()

    # Ensure the root element is <TileSet>
    if root.tag != "TileSet":
        raise ValueError("Root element is not <TileSet>.")

    # Get the SetNum value to create a subdirectory
    set_num = root.find("SetNum")
    if set_num is None or not set_num.text.isdigit():
        raise ValueError("No valid <SetNum> found in the <TileSet>.")
    set_num_dir = os.path.join(rgba_target_dir, f"{int(set_num.text)}")

    # Ensure subdirectory exists and is cleared of non-recursive files
    if not os.path.exists(set_num_dir):
        os.makedirs(set_num_dir, exist_ok=True)
    else:
        for file in os.listdir(set_num_dir):
            file_path = os.path.join(set_num_dir, file)
            if os.path.isfile(file_path):
                os.remove(file_path)

    # Find the Tiles section in the XML
    tiles = root.find("Tiles")
    if tiles is None:
        raise ValueError("No <Tiles> section found in the <TileSet>.")

    # Process each tile in the Tiles section
    for tile in tiles:
        input_image_path = tile.text
        relative_path = os.path.basename(input_image_path)
        base_name_no_ext = os.path.splitext(relative_path)[0]
        
        # Construct target file path within the set_num subdirectory
        if image_type == 1:
            rgba_filepath = os.path.join(set_num_dir, base_name_no_ext + ".rgba2")
        else:
            rgba_filepath = os.path.join(set_num_dir, base_name_no_ext + ".rgba8")
        
        with Image.open(input_image_path) as img:
            # Apply palette conversion if needed
            if do_palette:
                img = convert_to_agon_palette(img, 64, palette_conv_type, transparent_rgb)
            
            # Save image in the specified format
            if image_type == 1:
                img_to_rgba2(img, rgba_filepath)
            else:
                img_to_rgba8(img, rgba_filepath)

def generate_asm_levels(xml_tile_filepath, asm_levels_filepath):
    tree = ET.parse(xml_tile_filepath)
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

def main(bufferId, tile_width, tile_height, h_pitch, v_pitch, base_name, tiles_x, tiles_y, ranges, asm_src_dir, source_dir, target_dir, tileset_number):
    file_name = f"{base_name}{tileset_number}.png"
    if not os.path.exists(target_dir):
        os.makedirs(target_dir)

    for set_num, (start_x, start_y) in enumerate(ranges):
        chop_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height, h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, set_num, bufferId)

    asm_images_filepath = f"{asm_src_dir}/images_tiles_{base_name}.inc"
    xml_tile_filepath = os.path.join(target_dir, f"{base_name}_{tileset_number}_files.xml")
    asm_img_dir = f"tiles/{base_name}"
    generate_asm_img_load(xml_tile_filepath, asm_images_filepath, asm_img_dir)
    
    image_type = 1
    do_palette = False
    palette_conv_type = 'rgb'
    transparent_rgb = (0, 0, 0, 0)
    rgba_target_dir = f"tgt/tiles/{base_name}"
    if os.path.exists(rgba_target_dir):
        for filename in os.listdir(rgba_target_dir):
            file_path = os.path.join(rgba_target_dir, filename)
            if os.path.isfile(file_path):
                os.unlink(file_path)
    else:
        os.makedirs(rgba_target_dir)

    make_images(xml_tile_filepath, rgba_target_dir, image_type, do_palette, palette_conv_type, transparent_rgb)

    asm_levels_filepath = f"{asm_src_dir}/levels_{base_name}.inc"
    generate_asm_levels(xml_tile_filepath, asm_levels_filepath)

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
    main(bufferId, tile_width, tile_height, h_pitch, v_pitch, base_name, tiles_x, tiles_y, ranges, asm_src_dir, source_dir, target_dir, tileset_number)

    level_number = 0
    tiled_map_filepath = "tiles/dg/dg0_02.tmx"
    asm_levels_tiled_filepath = f"{asm_src_dir}/levels_tileset_{tileset_number}.inc"
    generate_asm_tiled_level(tiled_map_filepath, asm_levels_tiled_filepath, tileset_number, level_number, bufferId)
