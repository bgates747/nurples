import os
import shutil
from PIL import Image
import agonutils as au
import configparser
import hashlib

def chop_and_deduplicate_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height, h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, columns, start_num, set_num, make_dirs=False):
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
                tile_num = tile_count + start_num
                output_path = os.path.join(target_dir, f"{base_name}_{set_num}_{tile_num:03}.png")
                tile.save(output_path)
                unique_tiles[tile_hash] = tile_num
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

    return tile_count + start_num

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

def generate_manifest(target_dir, set_count):
    """
    Generates a manifest file for all metadata files.
    """
    manifest_path = os.path.join(target_dir, "manifest.ini")
    config = configparser.ConfigParser()

    config["Sets"] = {"SetCount": set_count}
    for set_num in range(set_count):
        metadata_path = os.path.join(target_dir, f"set_{set_num:02}_metadata.ini")
        config["Sets"][str(set_num)] = metadata_path

    with open(manifest_path, "w") as manifest_file:
        config.write(manifest_file)

    print(f"Manifest file written: {manifest_path}")
def generate_assembly_from_metadata(manifest_path, asm_output_file):
    """
    Generates assembly files from metadata files listed in a manifest.
    """
    config = configparser.ConfigParser()
    config.read(manifest_path)

    set_count = int(config["Sets"]["SetCount"])
    metadata_files = [config["Sets"][str(i)] for i in range(set_count)]

    with open(asm_output_file, "w") as asm_file:
        asm_file.write(f"num_levels: equ {set_count}\n")
        asm_file.write("tiles_levels:\n")

        for set_num, metadata_path in enumerate(metadata_files):
            set_config = configparser.ConfigParser()
            set_config.read(metadata_path)

            base_buffer_id = set_config["SetInfo"]["BaseBufferID"]
            csv_file = set_config["SetInfo"]["CSVFile"]
            num_rows = int(set_config["SetInfo"]["TileCount"])

            asm_file.write(f"    dl tiles_level_{set_num:02}\n")
            asm_file.write(f"tiles_level_{set_num:02}: ; Level {set_num:02}\n")
            asm_file.write(f"{base_buffer_id}: db {num_rows}\n")

            with open(csv_file, "r") as csv_input:
                rows = csv_input.readlines()

            for row_index, row in enumerate(rows):
                tile_values = row.strip().split(",")
                hex_values = [f"0x{int(value):02x}" for value in tile_values]
                asm_file.write(f"    db {','.join(hex_values)} ; row {row_index}\n")

        asm_file.write("\n")

def make_images(buffer_id, images_type, asm_images_filepath, originals_dir, output_dir_png, output_dir_rgba, palette_name, palette_dir, palette_conv_type, transparent_rgb, del_non_png, do_palette):
    # Load the palette
    palette_filepath = f'{palette_dir}/{palette_name}'
    
    # Delete rgba output directory and recreate it
    if os.path.exists(output_dir_rgba):
        shutil.rmtree(output_dir_rgba)
    os.makedirs(output_dir_rgba)

    # Delete processed output directory and recreate it
    if os.path.exists(output_dir_png):
        shutil.rmtree(output_dir_png)
    os.makedirs(output_dir_png)

    # Convert .jpeg, .jpg, and .gif files to .png
    for input_image_filename in os.listdir(originals_dir):
        input_image_path = os.path.join(originals_dir, input_image_filename)
        if input_image_filename.endswith(('.jpeg', '.jpg', '.gif')):
            # Load the image
            img = Image.open(input_image_path)

            # Convert to .png format and save with the same name but .png extension
            png_filename = os.path.splitext(input_image_filename)[0] + '.png'
            png_filepath = os.path.join(originals_dir, png_filename)
            img.save(png_filepath, 'PNG')

            if del_non_png:
                # Optionally, delete the original .jpeg, .jpg, or .gif file after conversion
                os.remove(input_image_path)

    # Copy all .png files from the input directory to the output png directory
    for input_image_filename in os.listdir(originals_dir):
        if input_image_filename.endswith('.png') and "_review_" not in input_image_filename:
            input_image_path = os.path.join(originals_dir, input_image_filename)
            output_image_path = os.path.join(output_dir_png, input_image_filename)
            shutil.copy(input_image_path, output_image_path)

    # Scan the output directory for all .png files and sort them
    filenames = sorted([f for f in os.listdir(output_dir_png) if f.endswith('.png')])

    # Initialize variables
    num_images = 0
    image_type = 1  # RGBA2222

    image_list = []
    files_list = []
    buffer_ids = []

    # Process the images
    for input_image_filename in filenames:
        input_image_path = os.path.join(output_dir_png, input_image_filename)

        # Continue only if it's a .png file
        if input_image_filename.endswith('.png'):
            # Open the image
            img = Image.open(input_image_path)

            # Remove ICC profile if present to avoid the warning
            if "icc_profile" in img.info:
                img.info.pop("icc_profile")
                # Re-save the image to remove the incorrect ICC profile
                img.save(input_image_path, 'PNG')

        else:
            continue

        image_filepath = f'{output_dir_png}/{input_image_filename}'
        file_name, ext = os.path.splitext(input_image_filename)

        with Image.open(image_filepath) as img:
            if do_palette:
                au.convert_to_palette(image_filepath, image_filepath, palette_filepath, palette_conv_type, transparent_rgb)

            if image_type == 1:
                rgba_filepath = f'{output_dir_rgba}/{file_name}.rgba2'
                au.img_to_rgba2(image_filepath, rgba_filepath)
            else:
                rgba_filepath = f'{output_dir_rgba}/{file_name}.rgba8'
                au.img_to_rgba8(image_filepath, rgba_filepath)

            buffer_ids.append(f'BUF_{file_name.upper()}: equ {buffer_id}\n')

            image_width, image_height = img.width, img.height
            image_filesize = os.path.getsize(rgba_filepath)

            image_list.append(f'\tdl {image_type}, {image_width}, {image_height}, {image_filesize}, fn_{file_name}, {buffer_id}\n')

            files_list.append(f'fn_{file_name}: db "{images_type}/{file_name}.rgba2",0 \n') 

            buffer_id += 1
            num_images += 1

    # Open assembly file for writing
    with open(f'{asm_images_filepath}', 'w') as f:
        f.write(f'; Generated by make_images.py\n\n')

        f.write(f'{images_type}_num_images: equ {num_images}\n\n')

        f.write(f'; buffer_ids:\n')
        f.write(''.join(buffer_ids))
        f.write(f'\n') 

        f.write(f'{images_type}_image_list: ; type; width; height; filename; bufferId:\n')
        f.write(''.join(image_list))
        f.write(f'\n') 

        f.write(f'; files_list: ; filename:\n')
        f.write(''.join(files_list))


if __name__ == "__main__":
    base_name = "dg"
    csv_dir = f"tiles/{base_name}/tiled"
    target_dir = csv_dir  # Target directory for generated metadata
    asm_output_file = "src/asm/levels.inc"
    asm_images_filepath = f"src/asm/images_tiles.inc"

    make_dirs = True
    tile_width = 16
    tile_height = 16
    h_pitch = tile_width
    v_pitch = tile_height
    tiles_x = 32
    tiles_y = 16
    columns = 8

    # Define ranges for image chopping
    ranges = [
        (0, 0),
        (512, 0),
        (1024, 0),
        (0, 240),
        (512, 240),
        (1024, 240),
    ]

    buffer_id = 512  # Starting buffer ID
    metadata_files = []  # To store paths to metadata files

    for set_num, (start_x, start_y) in enumerate(ranges):
        # Reset numbering for each set
        start_num = 0

        # Process images and generate tiles, CSV, and metadata
        start_num = chop_and_deduplicate_tiles(
            source_dir=f"tiles/{base_name}",
            target_dir=csv_dir,
            base_name=base_name,
            file_name=f"{base_name}.png",
            tile_width=tile_width,
            tile_height=tile_height,
            h_pitch=h_pitch,
            v_pitch=v_pitch,
            tiles_x=tiles_x,
            tiles_y=tiles_y,
            start_x=start_x,
            start_y=start_y,
            columns=columns,
            start_num=start_num,
            set_num=set_num,
            make_dirs=make_dirs
        )
        make_dirs = False  # Only recreate directories for the first set

        # Calculate base buffer ID for the set and save metadata
        base_buffer_id = ((buffer_id + 255) // 256) * 256
        metadata_path = os.path.join(target_dir, f"set_{set_num:02}_metadata.ini")
        metadata_files.append(metadata_path)

        buffer_id = base_buffer_id + start_num

    # Generate manifest file
    generate_manifest(target_dir, len(metadata_files))

    # Generate the assembly file for levels
    generate_assembly_from_metadata(
        manifest_path=os.path.join(target_dir, "manifest.ini"),
        asm_output_file=asm_output_file
    )

    # Generate the assembly file for images
    originals_dir = csv_dir
    output_dir_png = "assets/img/proc/tiles"
    output_dir_rgba = "tgt/tiles"
    palette_name = "Agon64.gpl"
    palette_dir = "build/palettes"
    palette_conv_type = "rgb"
    transparent_rgb = (0, 0, 0, 0)
    del_non_png = False
    do_palette = False

    make_images(buffer_id, "tiles", asm_images_filepath, originals_dir, output_dir_png, output_dir_rgba,palette_name, palette_dir, palette_conv_type, transparent_rgb, del_non_png, do_palette)
