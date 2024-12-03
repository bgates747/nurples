from PIL import Image
import os
import shutil

def chop_image(source_dir, file_name, output_base, start_x, start_y, tile_width=16, tile_height=16, h_pitch=16, v_pitch=16, rows=1, cols=1):
    """
    Chops a specified sample region of an image into tiles with given dimensions and pitch,
    saving them into a target directory named after the output base filename.

    Args:
        source_dir (str): Path to the source directory.
        file_name (str): Name of the source image file.
        output_base (str): Base filename for output, used to name the target directory.
        start_x (int): Starting x-coordinate of the sample region.
        start_y (int): Starting y-coordinate of the sample region.
        tile_width (int): Width of each tile (default: 16).
        tile_height (int): Height of each tile (default: 16).
        h_pitch (int): Horizontal pitch between tiles (default: 16).
        v_pitch (int): Vertical pitch between tiles (default: 16).
        rows (int): Number of rows in the sample.
        cols (int): Number of columns in the sample.
    """
    source_path = os.path.join(source_dir, file_name)
    target_dir = os.path.join(source_dir, output_base)

    # Delete and recreate the target directory
    if os.path.exists(target_dir):
        shutil.rmtree(target_dir)
    os.makedirs(target_dir)

    img = Image.open(source_path)

    x, y = start_x, start_y

    for row in range(rows):
        for col in range(cols):
            # Crop the tile
            tile = img.crop((x, y, x + tile_width, y + tile_height))
            
            # Save the tile in the target directory
            output_path = os.path.join(target_dir, f"{output_base}_{row:02}_{col:02}.png")
            tile.save(output_path)
            print(f"Saved {output_path} x={x} y={y}")
            
            # Update x for the next tile in the row
            x += h_pitch

        # Reset x and move to the next row
        x = start_x
        y += v_pitch

# Example usage
chop_image(source_dir="beegee747/src/assets/design/sprites",file_name="pac-man.png",output_base="pac_man",start_x=8,start_y=168,tile_width=16,tile_height=16,h_pitch=16,v_pitch=16,rows=4,cols=3)

chop_image(source_dir="beegee747/src/assets/design/sprites",file_name="pac-man.png",output_base="pac_ded",start_x=56,start_y=168,tile_width=16,tile_height=16,h_pitch=16,v_pitch=16,rows=1,cols=11)

chop_image(source_dir="beegee747/src/assets/design/sprites",file_name="pac-man.png",output_base="pac_liv",start_x=56,start_y=216,tile_width=16,tile_height=16,h_pitch=16,v_pitch=16,rows=1,cols=1)

chop_image(source_dir="beegee747/src/assets/design/sprites",file_name="pac-man.png",output_base="pac_big",start_x=8,start_y=232,tile_width=32,tile_height=32,h_pitch=32,v_pitch=32,rows=1,cols=3)
