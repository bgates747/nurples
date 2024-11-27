import os
from PIL import Image

def tile_image(input_file, output_dir, tile_size=(256, 256)):
    # Open the source image
    img = Image.open(input_file)
    width, height = img.size

    # Ensure output directory exists
    os.makedirs(output_dir, exist_ok=True)

    # Calculate number of tiles
    tile_width, tile_height = tile_size
    tiles_x = width // tile_width
    tiles_y = height // tile_height

    # Start tiling from the lower-right corner
    tile_index = 0
    for x in range(tiles_x - 1, -1, -1):  # Iterate from right to left
        for y in range(tiles_y - 1, -1, -1):  # Iterate from bottom to top
            # Define the crop box
            left = x * tile_width
            upper = y * tile_height
            right = left + tile_width
            lower = upper + tile_height

            # Crop the tile and save it
            tile = img.crop((left, upper, right, lower))
            tile_filename = os.path.join(output_dir, f"xevious_{tile_index:02}.png")
            tile.save(tile_filename)
            print(f"Saved {tile_filename}")

            tile_index += 1

# Input and output paths
input_file = "assets/img/design/xevious.png"
output_dir = "assets/img/orig/sprites"

# Tile the image
tile_image(input_file, output_dir)
