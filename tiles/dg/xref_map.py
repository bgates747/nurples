import xml.etree.ElementTree as ET
from PIL import Image
import csv
import os
import shutil

def load_csv_to_list(file_path):
    """Load a CSV file into a 2D list."""
    with open(file_path, 'r') as f:
        reader = csv.reader(f)
        return [row for row in reader]

def load_tileset_from_xml(file_path):
    """Load tile paths from the XML file."""
    tree = ET.parse(file_path)
    root = tree.getroot()
    tiles = {}
    for tile in root.findall('Tiles/*'):
        tiles[tile.tag] = tile.text
    return tiles

def load_tmx(file_path):
    """Parse the TMX file and return metadata and tile data."""
    tree = ET.parse(file_path)
    root = tree.getroot()
    
    map_data = {
        "tilewidth": int(root.attrib['tilewidth']),
        "tileheight": int(root.attrib['tileheight']),
        "width": int(root.attrib['width']),
        "height": int(root.attrib['height'])
    }
    
    layer = root.find('layer')
    data = layer.find('data').text.strip().split(',')
    map_data['tiles'] = [int(x) for x in data]
    return map_data

def create_image(tmx_data, csv_data, tileset, output_size):
    """Create an image from TMX data, CSV data, and the tileset."""
    tile_width = tmx_data['tilewidth']
    tile_height = tmx_data['tileheight']
    map_width = tmx_data['width']
    map_height = tmx_data['height']
    tiles = tmx_data['tiles']

    # Create a blank RGBA image for transparency
    output_image = Image.new('RGBA', output_size, (255, 255, 255, 0))

    for row in range(map_height):
        for col in range(map_width):
            tile_index = tiles[row * map_width + col] - 1  # TMX is 1-based
            if tile_index < 0:  # Null tile
                continue
            
            # Get the corresponding hex value from the CSV
            csv_value = csv_data[tile_index // 16][tile_index % 16]
            tile_tag = f"Tile{csv_value.upper()}"
            
            # Get the tile path from the tileset
            if tile_tag in tileset:
                tile_image_path = tileset[tile_tag]
                tile_image = Image.open(tile_image_path)
                x = col * tile_width
                y = row * tile_height
                output_image.paste(tile_image, (x, y), tile_image)

    return output_image

def copy_and_renumber_tiles_from_tmx(tmx_data, tileset, output_dir):
    """
    Copy tiles to the output directory, renumbered according to the TMX data.
    Renumbering keeps the 1-based indexing and converts indices to 8-bit hex.
    """
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    for tile_value in set(tmx_data['tiles']):
        if tile_value == 0:  # Null tile, skip
            continue
        
        hex_value = f"{tile_value:02X}"
        tile_tag = f"Tile{hex_value}"
        
        if tile_tag in tileset:
            src_path = tileset[tile_tag]
            dest_path = os.path.join(output_dir, f"{hex_value}.png")
            shutil.copy(src_path, dest_path)
            print(f"Copied {src_path} to {dest_path}")

if __name__ == "__main__":
    # File paths
    tmx_file = "tiles/dg/tiled/dg0.tmx"
    csv_file = "tiles/dg/dg_0_map.csv"
    xml_file = "tiles/dg/dg_0_files.xml"
    output_tile_dir = "tiles/proc/dg/0"

    # Load files
    tmx_data = load_tmx(tmx_file)
    csv_data = load_csv_to_list(csv_file)
    tileset = load_tileset_from_xml(xml_file)

    # Determine output image size
    output_size = (tmx_data['width'] * tmx_data['tilewidth'], 
                   tmx_data['height'] * tmx_data['tileheight'])

    # # Create the image
    # output_image = create_image(tmx_data, csv_data, tileset, output_size)

    # # Show the image
    # output_image.show()

    # Copy and renumber tiles according to the TMX map
    copy_and_renumber_tiles_from_tmx(tmx_data, tileset, output_tile_dir)
