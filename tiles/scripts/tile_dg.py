import os
from PIL import Image
import hashlib
import xml.etree.ElementTree as ET
from xml.dom import minidom

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

    for row in range(tiles_y):
        row_tiles = []
        for col in range(tiles_x):
            x = col * h_pitch
            y = row * v_pitch
            tile = cropped_img.crop((x, y, x + tile_width, y + tile_height))
            tile = tile.rotate(90, expand=True)
            tile_hash = hashlib.md5(tile.tobytes()).hexdigest()
            if tile_hash not in unique_tiles:
                output_path = os.path.join(target_dir, f"{base_name}_{set_num}_{tile_count:03}.png")
                tile.save(output_path)
                unique_tiles[tile_hash] = tile_count
                output_images.append(output_path)
                tile_count += 1
            row_tiles.append(f"{unique_tiles[tile_hash]:03}")
        tile_map.append(row_tiles)

    tile_map = list(zip(*tile_map))
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
    ET.SubElement(root, "FileCount").text = str(len(output_images))
    tiles_element = ET.SubElement(root, "Tiles")
    for i, file_path in enumerate(output_images):
        tile_element = ET.SubElement(tiles_element, f"Tile{i:03}")
        tile_element.text = file_path
    rough_string = ET.tostring(root, encoding="utf-8")
    parsed = minidom.parseString(rough_string)
    pretty_xml = parsed.toprettyxml(indent="  ")
    with open(xml_filepath, "w") as xml_file:
        xml_file.write(pretty_xml)
    print(f"Saved files.xml to {xml_filepath}")

def generate_combined_xml(combined_xml_filepath, base_name, target_dir, buffer_id):
    xml_files = [f for f in os.listdir(target_dir) if f.startswith(base_name) and f.endswith("_files.xml")]
    xml_files.sort()
    combined_root = ET.Element("TileSets")
    current_buffer_id = buffer_id
    for xml_file in xml_files:
        xml_path = os.path.join(target_dir, xml_file)
        tree = ET.parse(xml_path)
        root = tree.getroot()
        tile_set_header = ET.SubElement(combined_root, "TileSet")
        for child in root:
            if child.tag != "Tiles":  # Exclude the <Tiles> section
                ET.SubElement(tile_set_header, child.tag).text = child.text
        file_count = int(root.find("FileCount").text)
        ET.SubElement(tile_set_header, "BaseBufferId").text = str(current_buffer_id)
        current_buffer_id += file_count
        current_buffer_id = ((current_buffer_id + 255) // 256) * 256
    rough_string = ET.tostring(combined_root, encoding="utf-8")
    parsed = minidom.parseString(rough_string)
    pretty_xml = parsed.toprettyxml(indent="  ")
    with open(combined_xml_filepath, "w") as xml_file:
        xml_file.write(pretty_xml)
    print(f"Combined XML file created at {combined_xml_filepath}")

def generate_asm_img_load(combined_xml_filepath, asm_images_filepath, rgba_target_dir):
    # Parse the combined XML file
    tree = ET.parse(combined_xml_filepath)
    root = tree.getroot()

    # Initialize outputs
    asm_buffer_ids = []
    asm_image_list = []
    asm_file_list = []
    total_images = 0

    # Process each TileSet
    for tile_set in root.findall("TileSet"):
        base_buffer_id = int(tile_set.find("BaseBufferId").text)
        tiles_file = tile_set.find("TilesFile").text

        # Parse individual tiles XML file
        tiles_tree = ET.parse(tiles_file)
        tiles_root = tiles_tree.getroot()
        for tile_idx, tile in enumerate(tiles_root.find("Tiles")):
            tile_filename = tile.text
            image_name = os.path.splitext(os.path.basename(tile_filename))[0]
            buffer_id = base_buffer_id + tile_idx
            tile_width = int(tile_set.find("TileWidth").text)
            tile_height = int(tile_set.find("TileHeight").text)
            tile_size = tile_width * tile_height
            set_num = int(tile_set.find("SetNum").text)

            # Add buffer ID entry for each tile
            asm_buffer_ids.append(f"BUF_{tile_set.find('BaseName').text.upper()}_{set_num:03}_{tile_idx:03}: equ {buffer_id}")

            # Add image list entry
            asm_image_list.append(
                f"\tdl 1, {tile_width}, {tile_height}, {tile_size}, fn_{image_name}, {buffer_id}"
            )

            # Add file list entry
            rgba_filename = os.path.join(rgba_target_dir, f"{image_name}.rgba2").replace("\\", "/")
            asm_file_list.append(f"fn_{image_name}: db \"{rgba_filename}\",0")

        total_images += len(tiles_root.find("Tiles"))

    # Write to assembly images file
    with open(asm_images_filepath, "w") as asm_images_file:
        asm_images_file.write(f"; Generated by tiles_{base_name}.py\n\n")
        asm_images_file.write(f"tiles_{base_name}_num_images: equ {total_images}\n\n")
        asm_images_file.write("; buffer_ids:\n")
        asm_images_file.write("\n".join(asm_buffer_ids) + "\n\n")
        asm_images_file.write(f"tiles_{base_name}_image_list: ; type; width; height; size; filename; bufferId:\n")
        asm_images_file.write("\n".join(asm_image_list) + "\n\n")
        asm_images_file.write(f"tiles_{base_name}_files_list: ; filename:\n")
        asm_images_file.write("\n".join(asm_file_list) + "\n")

    print(f"Assembly images file created at {asm_images_filepath}")

if __name__ == "__main__":
# chop up the source image into tiles and create metadata files
    base_name = "dg"
    file_name=f"{base_name}.png"
    source_dir=f"tiles/{base_name}"
    target_dir = f"tiles/{base_name}/tiled"
    tile_width = 16
    tile_height = 16
    h_pitch = tile_width
    v_pitch = tile_height
    tiles_x = 32
    tiles_y = 16
    columns = 8
    ranges = [(0, 0),(512, 0),(1024, 0),(0, 240),(512, 240),(1024, 240),]
    # ranges = [(0,0)]
    for set_num, (start_x, start_y) in enumerate(ranges):
        chop_and_deduplicate_tiles(source_dir,target_dir,base_name,file_name,tile_width,tile_height,h_pitch,v_pitch,tiles_x,tiles_y,start_x,start_y,set_num)

# generate the assembly code to load the tiles and define levels
    asm_levels_filepath = f"src/asm/levels_{base_name}.inc"
    asm_images_filepath = f"src/asm/images_tiles_{base_name}.inc"
    buffer_id = 512
    combined_xml_filepath = os.path.join(target_dir, f"{base_name}.xml")
    rgba_target_dir = "tgt/tiles"
    generate_combined_xml(combined_xml_filepath, base_name, target_dir, buffer_id)
    generate_asm_img_load(combined_xml_filepath, asm_images_filepath, rgba_target_dir)