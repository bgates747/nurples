import os
from PIL import Image

def tile_image(input_file, output_dir, tile_size=(32, 32)):
    # Open the source image
    img = Image.open(input_file)
    width, height = img.size

    # Ensure output directory exists
    os.makedirs(output_dir, exist_ok=True)

    # Calculate number of tiles
    tile_width, tile_height = tile_size
    tiles_x = width // tile_width
    tiles_y = height // tile_height

    # Start tiling from the top-left corner
    tile_index = 0
    for y in range(tiles_y):  # Iterate row by row (downwards)
        for x in range(tiles_x):  # Iterate column by column (across)
            # Define the crop box
            left = x * tile_width
            upper = y * tile_height
            right = left + tile_width
            lower = upper + tile_height

            # Crop the tile and save it
            tile = img.crop((left, upper, right, lower))
            tile_filename = os.path.join(output_dir, f"xevious_sprite_{tile_index:03}.png")
            tile.save(tile_filename)
            print(f"Saved {tile_filename}")

            tile_index += 1

# Input and output paths
input_file = "assets/img/design/xevious_sprites.png"
output_dir = "assets/img/orig/sprites"

# Tile the image
tile_image(input_file, output_dir)
