import os
import shutil
from PIL import Image
import agonutils as au
import configparser
import hashlib

def chop_and_deduplicate_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height, h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, set_num, make_dirs=False):
    """
    Chops an image into tiles with specified width, height, and pitch,
    deduplicates the tiles based on their content, saves unique tiles, and generates a CSV map and review image.
    """
    source_path = os.path.join(source_dir, file_name)

    if make_dirs:
        # Delete and recreate the target directory
        if os.path.exists(target_dir):
            shutil.rmtree(target_dir)
        os.makedirs(target_dir)

    # Open the source image
    img = Image.open(source_path)

    # Crop the image to the grid area starting at (start_x, start_y)
    crop_width = tiles_x * h_pitch
    crop_height = tiles_y * v_pitch
    cropped_img = img.crop((start_x, start_y, start_x + crop_width, start_y + crop_height))

    # Store unique tiles and their hashes
    unique_tiles = {}
    tile_map = []  # To store the CSV map data
    tile_count = 0
    output_images = []  # To store paths of created images

    for row in range(tiles_y):
        row_tiles = []
        for col in range(tiles_x):
            # Calculate tile position within the cropped image
            x = col * h_pitch
            y = row * v_pitch
            tile = cropped_img.crop((x, y, x + tile_width, y + tile_height))
            
            # Rotate the tile 90 degrees counterclockwise
            tile = tile.rotate(90, expand=True)

            # Generate a hash for the tile
            tile_hash = hashlib.md5(tile.tobytes()).hexdigest()

            # Check if the tile is unique
            if tile_hash not in unique_tiles:
                # Save the unique tile
                output_path = os.path.join(target_dir, f"{base_name}_{set_num}_{tile_count:03}.png")
                tile.save(output_path)
                unique_tiles[tile_hash] = tile_count
                output_images.append(output_path)
                tile_count += 1

            # Add the tile index to the row
            row_tiles.append(f"{unique_tiles[tile_hash]:03}")
        
        # Append the row data to the map
        tile_map.append(row_tiles)

    # Rotate the CSV map by transposing it
    rotated_map = list(zip(*tile_map))

    # Save the rotated map to a CSV file
    save_csv_map(rotated_map, os.path.join(target_dir, f"{base_name}_{set_num:02}.csv"))

    # Save metadata for the set
    metadata_path = os.path.join(target_dir, f"set_{set_num:02}_metadata.ini")
    config = configparser.ConfigParser()

    config["SetInfo"] = {
        "SetNumber": set_num,
        "BaseBufferID": f"BUF_{base_name.upper()}_{set_num}_000",
        "TileCount": tile_count,
        "CSVFile": os.path.join(target_dir, f"{base_name}_{set_num:02}.csv")
    }

    config["TileImages"] = {str(i): path for i, path in enumerate(output_images)}

    with open(metadata_path, "w") as metadata_file:
        config.write(metadata_file)
    print(f"Metadata file written: {metadata_path}")


def save_csv_map(tile_map, output_path):
    """
    Saves the tile map to a CSV file.

    Args:
        tile_map (list): List of rows containing tile indices.
        output_path (str): Path to the output CSV file.
    """
    with open(output_path, "w") as csv_file:
        csv_file.write("\n".join(",".join(row) for row in tile_map))
    print(f"Saved CSV map to {output_path}")