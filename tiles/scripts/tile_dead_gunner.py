from PIL import Image, ImageDraw
import os
import shutil
import hashlib

def chop_and_deduplicate_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height, h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, columns, start_num, set_num, make_dirs=False):
    """
    Chops an image into tiles with specified width, height, and pitch,
    deduplicates the tiles based on their content, saves unique tiles, and generates a CSV map and review image.

    Args:
        source_dir (str): Path to the source directory.
        target_dir (str): Path to the target directory.
        base_name (str): Base name for output files.
        file_name (str): Name of the source image file.
        tile_width (int): Width of each tile.
        tile_height (int): Height of each tile.
        h_pitch (int): Horizontal pitch between tiles.
        v_pitch (int): Vertical pitch between tiles.
        tiles_x (int): Number of tiles horizontally.
        tiles_y (int): Number of tiles vertically.
        start_x (int): Starting x-coordinate of the grid.
        start_y (int): Starting y-coordinate of the grid.
        columns (int): Number of columns for the review image.
        start_num (int): Starting tile number.
        set_num (int): Set number for naming files.
        make_dirs (bool): Whether to recreate the target directory.
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
            
            # Generate a hash for the tile
            tile_hash = hashlib.md5(tile.tobytes()).hexdigest()

            # Check if the tile is unique
            if tile_hash not in unique_tiles:
                # Save the unique tile
                tile_num = tile_count + start_num
                output_path = os.path.join(target_dir, f"{base_name}_{tile_num:03}.png")
                tile.save(output_path)
                unique_tiles[tile_hash] = tile_num
                output_images.append(output_path)
                tile_count += 1

            # Add the tile index to the row
            row_tiles.append(f"{unique_tiles[tile_hash]:03}")
        
        # Append the row data to the map
        tile_map.append(",".join(row_tiles))

    # Save the map to a CSV file
    save_csv_map(tile_map, os.path.join(target_dir, f"{base_name}_{set_num:02}.csv"))

    # Generate a review image for this set
    review_file = os.path.join(target_dir, f"{base_name}_review_{set_num:02}.png")
    generate_review_image(review_file, output_images, tile_width, tile_height, columns)

    return tile_count + start_num

def save_csv_map(tile_map, output_path):
    """
    Saves the tile map to a CSV file.

    Args:
        tile_map (list): List of rows containing tile indices.
        output_path (str): Path to the output CSV file.
    """
    with open(output_path, "w") as csv_file:
        csv_file.write("\n".join(tile_map))
    print(f"Saved CSV map to {output_path}")

def generate_review_image(review_file, image_paths, tile_width, tile_height, columns):
    """
    Generates a single image containing all unique tiles for review.

    Args:
        review_file (str): Path to save the review image.
        image_paths (list): List of paths to tile images.
        tile_width (int): Width of each tile.
        tile_height (int): Height of each tile.
        columns (int): Number of columns in the review image.
    """
    rows = (len(image_paths) + columns - 1) // columns  # Calculate required rows
    review_img = Image.new("RGBA", (columns * tile_width, rows * tile_height))

    for index, tile_path in enumerate(image_paths):
        tile = Image.open(tile_path)
        col = index % columns
        row = index // columns
        review_img.paste(tile, (col * tile_width, row * tile_height))

    review_img.save(review_file)
    print(f"Saved review image to {review_file}")

if __name__ == "__main__":
    base_name = "dg"
    source_dir = f"tiles/{base_name}"
    file_name = f"{base_name}.png"
    target_dir = f"tiles/{base_name}/tiled"

    make_dirs = True
    tile_width = 16
    tile_height = 16
    h_pitch = tile_width
    v_pitch = tile_height
    tiles_x = 32
    tiles_y = 16
    columns = 8
    start_num = 0
    set_num = -1

    start_num = (start_num + 9) // 10 * 10
    set_num += 1
    start_x = 0
    start_y = 0
    start_num = chop_and_deduplicate_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height,h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, columns, start_num, set_num, make_dirs)

    make_dirs = False
    start_num = (start_num + 9) // 10 * 10
    set_num += 1
    start_x = 512
    start_y = 0
    start_num = chop_and_deduplicate_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height,h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, columns, start_num, set_num, make_dirs)
    
    start_num = (start_num + 9) // 10 * 10
    set_num += 1
    start_x = 1024
    start_y = 0
    start_num = chop_and_deduplicate_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height,h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, columns, start_num, set_num, make_dirs)
    
    start_num = (start_num + 9) // 10 * 10
    set_num += 1
    start_x = 0
    start_y = 240
    start_num = chop_and_deduplicate_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height,h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, columns, start_num, set_num, make_dirs)
    
    start_num = (start_num + 9) // 10 * 10
    set_num += 1
    start_x = 512
    start_y = 240
    start_num = chop_and_deduplicate_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height,h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, columns, start_num, set_num, make_dirs)

    start_num = (start_num + 9) // 10 * 10
    set_num += 1
    start_x = 1024
    start_y = 240
    start_num = chop_and_deduplicate_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height,h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, columns, start_num, set_num, make_dirs)
