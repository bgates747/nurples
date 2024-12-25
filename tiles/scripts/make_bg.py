from PIL import Image
import os
import random

def create_composites(source_path, target_directory, output_prefix, tile_size, grid_size, num_outputs):
    # Ensure target directory exists
    os.makedirs(target_directory, exist_ok=True)

    # Open the source image
    original_image = Image.open(source_path)

    # Calculate grid coordinates of tiles
    tile_coordinates = [
        (x * tile_size, y * tile_size)
        for y in range(grid_size)
        for x in range(grid_size)
    ]

    # Generate new composite images
    for n in range(num_outputs):
        composite_image = Image.new("RGBA", original_image.size)

        for tile_index, (x, y) in enumerate(tile_coordinates):
            # Randomly choose a source tile
            source_x, source_y = random.choice(tile_coordinates)
            source_tile = original_image.crop((source_x, source_y, source_x + tile_size, source_y + tile_size))

            # Paste it into the composite image
            composite_image.paste(source_tile, (x, y))

        # Save the composite image
        output_path = os.path.join(target_directory, f"{output_prefix}_{n}.png")
        composite_image.save(output_path)

if __name__ == "__main__":
    source_image_path = "assets/img/design/bg_stars.png"
    target_dir = "tiles/bg/orig"
    prefix = "bg_stars"
    tile_size = 64
    grid_size = 4  # 256 / 64 = 4
    num_composites = 8

    create_composites(source_image_path, target_dir, prefix, tile_size, grid_size, num_composites)
