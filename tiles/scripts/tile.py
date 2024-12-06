import os, shutil
from PIL import Image
import hashlib
import xml.etree.ElementTree as ET
from xml.dom import minidom
from agonImages import img_to_rgba2, img_to_rgba8, convert_to_agon_palette
import csv

def chop_and_deduplicate_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height, h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, set_num):
    source_path = os.path.join(source_dir, file_name)
    img = Image.open(source_path)
    crop_width = tiles_x * h_pitch
    crop_height = tiles_y * v_pitch
    cropped_img = img.crop((start_x, start_y, start_x + crop_width, start_y + crop_height))
    unique_tiles = {}
    tile_map = []
    tile_count = 0
    output_images = []
    set_num_hex = f"{set_num:1X}"

    for row in range(tiles_y):
        row_tiles = []
        for col in range(tiles_x):
            x = col * h_pitch
            y = row * v_pitch
            tile = cropped_img.crop((x, y, x + tile_width, y + tile_height))
            tile_hash = hashlib.md5(tile.tobytes()).hexdigest()
            if tile_hash not in unique_tiles:
                output_path = os.path.join(target_dir, f"{base_name}_{set_num_hex}_{tile_count:02X}.png")
                tile.save(output_path)
                unique_tiles[tile_hash] = tile_count
                output_images.append(output_path)
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
    ET.SubElement(root, "FileCount").text = str(len(output_images))
    tiles_element = ET.SubElement(root, "Tiles")
    for i, file_path in enumerate(output_images):
        tile_element = ET.SubElement(tiles_element, f"Tile{i:02X}")
        tile_element.text = file_path
    rough_string = ET.tostring(root, encoding="utf-8")
    parsed = minidom.parseString(rough_string)
    pretty_xml = parsed.toprettyxml(indent="  ")
    with open(xml_filepath, "w") as xml_file:
        xml_file.write(pretty_xml)
    print(f"Saved files.xml to {xml_filepath}")

def generate_combined_xml(combined_xml_filepath, base_name, target_dir, bufferId):
    xml_files = [f for f in os.listdir(target_dir) if f.startswith(base_name) and f.endswith("_files.xml")]
    xml_files.sort()
    combined_root = ET.Element("TileSets")
    current_bufferId = bufferId
    for xml_file in xml_files:
        xml_path = os.path.join(target_dir, xml_file)
        tree = ET.parse(xml_path)
        root = tree.getroot()
        tile_set_header = ET.SubElement(combined_root, "TileSet")
        for child in root:
            if child.tag != "Tiles":  # Exclude the <Tiles> section
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
    for tile_set in root.findall("TileSet"):
        base_bufferId = int(tile_set.find("BaseBufferId").text)
        tiles_file = tile_set.find("TilesFile").text
        file_count = int(tile_set.find("FileCount").text)
        base_name = tile_set.find("BaseName").text.upper()
        set_num = int(tile_set.find("SetNum").text)
        asm_bufferIds.append(f"BUF_{base_name}_{set_num:1X}_00: equ {base_bufferId}")
        tiles_tree = ET.parse(tiles_file)
        tiles_root = tiles_tree.getroot()
        for tile_idx, tile in enumerate(tiles_root.find("Tiles")):
            tile_filename = tile.text
            image_name = os.path.splitext(os.path.basename(tile_filename))[0]
            bufferId = base_bufferId + tile_idx
            tile_width = int(tile_set.find("TileWidth").text)
            tile_height = int(tile_set.find("TileHeight").text)
            tile_size = tile_width * tile_height
            asm_image_list.append(
                f"\tdl 1, {tile_width}, {tile_height}, {tile_size}, fn_{image_name}, {bufferId}"
            )
            rgba_filename = os.path.join(asm_img_dir, f"{image_name}.rgba2").replace("\\", "/")
            asm_file_list.append(f"fn_{image_name}: db \"{rgba_filename}\",0")

        total_images += file_count
    with open(asm_images_filepath, "w") as asm_images_file:
        asm_images_file.write(f"; Generated by tiles_{base_name.lower()}.py\n\n")
        asm_images_file.write(f"tiles_{base_name.lower()}_num_images: equ {total_images}\n\n")
        asm_images_file.write("; bufferIds:\n")
        asm_images_file.write("\n".join(asm_bufferIds) + "\n\n")
        asm_images_file.write(f"tiles_{base_name.lower()}_image_list: ; type; width; height; size; filename; bufferId:\n")
        asm_images_file.write("\n".join(asm_image_list) + "\n\n")
        asm_images_file.write(f"tiles_{base_name.lower()}_files_list: ; filename:\n")
        asm_images_file.write("\n".join(asm_file_list) + "\n")
    print(f"Assembly images file created at {asm_images_filepath}")

def make_images(combined_xml_filepath, rgba_target_dir, image_type, do_palette, palette_conv_type, transparent_rgb):
    tree = ET.parse(combined_xml_filepath)
    root = tree.getroot()
    for tile_set in root.findall("TileSet"):
        tiles_file = tile_set.find("TilesFile").text
        tiles_tree = ET.parse(tiles_file)
        tiles_root = tiles_tree.getroot()
        for tile in tiles_root.find("Tiles"):
            input_image_path = tile.text
            file_name, ext = os.path.splitext(os.path.basename(input_image_path))
            with Image.open(input_image_path) as img:
                if do_palette:
                    img = convert_to_agon_palette(img, 64, palette_conv_type, transparent_rgb)
                if image_type == 1:
                    rgba_filepath = os.path.join(rgba_target_dir, f"{file_name}.rgba2")
                    img_to_rgba2(img, rgba_filepath)
                else:
                    rgba_filepath = os.path.join(rgba_target_dir, f"{file_name}.rgba8")
                    img_to_rgba8(img, rgba_filepath)

def generate_asm_levels(combined_xml_filepath, asm_levels_filepath):
    tree = ET.parse(combined_xml_filepath)
    root = tree.getroot()
    num_levels = 0
    levels_list = []
    levels_data = []
    for set_num, tile_set in enumerate(root.findall("TileSet")):
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
    with open(asm_levels_filepath, "w") as asm_levels_file:
        asm_levels_file.write(f"; Generated by tiles_{base_name.lower()}_levels.py\n\n")
        asm_levels_file.write(f"tiles_{base_name.lower()}_num_levels: db {num_levels}\n\n")
        asm_levels_file.write(f"tiles_{base_name.lower()}_levels:\n")
        asm_levels_file.write("\n".join(levels_list) + "\n")
        asm_levels_file.write("\tdl 0 ; list terminator\n\n")
        asm_levels_file.write("\n".join(levels_data) + "\n")
    print(f"Assembly levels file created at {asm_levels_filepath}")

def main(bufferId, tile_width, tile_height, h_pitch, v_pitch, base_name, tiles_x, tiles_y, ranges):
    file_name=f"{base_name}.png"
    source_dir=f"tiles/{base_name}"
    target_dir = f"tiles/{base_name}/tiled"

    if os.path.exists(target_dir):
        shutil.rmtree(target_dir)
    os.makedirs(target_dir)
    for set_num, (start_x, start_y) in enumerate(ranges):
        chop_and_deduplicate_tiles(source_dir,target_dir,base_name,file_name,tile_width,tile_height,h_pitch,v_pitch,tiles_x,tiles_y,start_x,start_y,set_num)

# generate the assembly code to load the tileS
    asm_images_filepath = f"src/asm/images_tiles_{base_name}.inc"
    combined_xml_filepath = os.path.join(target_dir, f"{base_name}.xml")
    asm_img_dir = f"tiles/{base_name}"
    next_bufferId = generate_combined_xml(combined_xml_filepath, base_name, target_dir, bufferId)
    generate_asm_img_load(combined_xml_filepath, asm_images_filepath, asm_img_dir)
    
# convert the tiles to RGBA2 format
    image_type=1
    do_palette=False
    palette_conv_type='rgb'
    transparent_rgb = (0, 0, 0, 0)
    rgba_target_dir = f"tgt/tiles/{base_name}"
    if os.path.exists(rgba_target_dir):
        shutil.rmtree(rgba_target_dir)
    os.makedirs(rgba_target_dir)
    make_images(combined_xml_filepath, rgba_target_dir, image_type, do_palette, palette_conv_type, transparent_rgb)

# generate the assembly code for levels
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