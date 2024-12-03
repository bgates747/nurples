from PIL import Image
import os
import shutil

def chop_image(source_dir, file_name, tile_width=16, tile_height=16, h_pitch=16, v_pitch=16):
    """
    Chops an image into tiles with specified width, height, and pitch,
    saving them into a target directory named after the source file.

    Args:
        source_dir (str): Path to the source directory.
        file_name (str): Name of the source image file.
        tile_width (int): Width of each tile (default: 16).
        tile_height (int): Height of each tile (default: 16).
        h_pitch (int): Horizontal pitch between tiles (default: 16).
        v_pitch (int): Vertical pitch between tiles (default: 16).
    """
    source_path = os.path.join(source_dir, file_name)
    target_dir = os.path.join(source_dir, os.path.splitext(file_name)[0])

    # Delete and recreate the target directory
    if os.path.exists(target_dir):
        shutil.rmtree(target_dir)
    os.makedirs(target_dir)

    img = Image.open(source_path)

    # Define groups and their properties
    groups = [
        ("blinky", 8),
        ("pinky", 8),
        ("inky", 8),
        ("clyde", 8),
        ("reverse", 8)
    ]

    x, y = 0, 0

    for group_name, group_length in groups:
        for i in range(group_length):
            # Crop the tile
            tile = img.crop((x, y, x + tile_width, y + tile_height))
            
            # Save the tile in the target directory
            output_path = os.path.join(target_dir, f"{group_name}_{i}.png")
            tile.save(output_path)
            print(f"Saved {output_path} x={x} y={y}")
            
            # Update x for the next tile
            x += h_pitch
        
        # Reset x and increment y after completing the group
        x = 0
        y += v_pitch

# Example usage
chop_image("beegee747/src/assets/design/sprites", "ghosts.png")
