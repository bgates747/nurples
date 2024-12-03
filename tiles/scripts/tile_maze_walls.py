from PIL import Image, ImageDraw
import os
import shutil
import hashlib

def chop_and_deduplicate_tiles(source_dir, file_name, tile_width=8, tile_height=8, h_pitch=8, v_pitch=8, tiles_x=28, tiles_y=31, start_x=152, start_y=64):
    """
    Chops an image into tiles with specified width, height, and pitch,
    deduplicates the tiles based on their content, saves unique tiles, and generates a CSV map and review image.

    Args:
        source_dir (str): Path to the source directory.
        file_name (str): Name of the source image file.
        tile_width (int): Width of each tile (default: 8).
        tile_height (int): Height of each tile (default: 8).
        h_pitch (int): Horizontal pitch between tiles (default: 8).
        v_pitch (int): Vertical pitch between tiles (default: 8).
        tiles_x (int): Number of tiles horizontally.
        tiles_y (int): Number of tiles vertically.
        start_x (int): Starting x-coordinate of the grid.
        start_y (int): Starting y-coordinate of the grid.
    """
    source_path = os.path.join(source_dir, file_name)
    base_file_name = os.path.splitext(file_name)[0]
    target_dir = os.path.join(source_dir, base_file_name)

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
                output_path = os.path.join(target_dir, f"{base_file_name}_{tile_count:02}.png")
                tile.save(output_path)
                unique_tiles[tile_hash] = tile_count
                tile_count += 1

            # Add the tile index (padded) to the row
            row_tiles.append(f"{unique_tiles[tile_hash]:02}")
        
        # Append the row data to the map
        tile_map.append(",".join(row_tiles))

    # Save the map to a CSV file
    save_csv_map(tile_map, os.path.join(target_dir, f"{base_file_name}.csv"))

    # Generate a review image of unique tiles
    generate_review_image(base_file_name, unique_tiles, target_dir, tile_width, tile_height, columns=8)

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

def generate_review_image(base_file_name, unique_tiles, target_dir, tile_width, tile_height, columns=8):
    """
    Generates a single image containing all unique tiles for review.

    Args:
        unique_tiles (dict): Dictionary of unique tiles with their indices.
        target_dir (str): Directory where the review image will be saved.
        tile_width (int): Width of each tile.
        tile_height (int): Height of each tile.
        columns (int): Number of columns in the review image.
    """
    rows = (len(unique_tiles) + columns - 1) // columns  # Calculate required rows
    review_img = Image.new("RGBA", (columns * tile_width, rows * tile_height))

    for index, tile_file in enumerate(sorted(unique_tiles.values())):
        tile_path = os.path.join(target_dir, f"{base_file_name}_{tile_file:02}.png")
        tile = Image.open(tile_path)
        col = index % columns
        row = index // columns
        review_img.paste(tile, (col * tile_width, row * tile_height))

    review_img_path = os.path.join(target_dir, "review_image.png")
    review_img.save(review_img_path)
    print(f"Saved review image to {review_img_path}")

# Example usage
chop_and_deduplicate_tiles("beegee747/src/assets/design/sprites", "maze_pellets.png")
