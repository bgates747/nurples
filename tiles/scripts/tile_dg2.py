import os
import shutil
from PIL import Image
import agonutils as au
import hashlib

def chop_and_deduplicate_tiles(source_dir, target_dir, base_name, file_name, tile_width, tile_height, h_pitch, v_pitch, tiles_x, tiles_y, start_x, start_y, set_num):
    source_path = os.path.join(source_dir, file_name)
    img = Image.open(source_path)
    crop_width = tiles_x * h_pitch
    crop_height = tiles_y * v_pitch
    cropped_img = img.crop((start_x, start_y, start_x + crop_width, start_y + crop_height))
    unique_tiles = {}
    tile_map = []
    tile_count = 0
    output_images = []
    for row in range(tiles_y):
        row_tiles = []
        for col in range(tiles_x):
            x = col * h_pitch
            y = row * v_pitch
            tile = cropped_img.crop((x, y, x + tile_width, y + tile_height))
            tile = tile.rotate(90, expand=True)
            tile_hash = hashlib.md5(tile.tobytes()).hexdigest()
            if tile_hash not in unique_tiles:
                output_path = os.path.join(target_dir, f"{base_name}_{set_num}_{tile_count:03}.png")
                # tile.save(output_path)
                unique_tiles[tile_hash] = tile_count
                output_images.append(output_path)
                tile_count += 1
            row_tiles.append(f"{unique_tiles[tile_hash]:03}")
        tile_map.append(row_tiles)
    rotated_map = list(zip(*tile_map))
    map_filepath = os.path.join(target_dir, f"{base_name}_{set_num}_map.csv")
    ini_filepath = os.path.join(target_dir, f"{base_name}_{set_num}_files.ini")
    save_csv_map(rotated_map, map_filepath)
    save_files_ini(output_images, ini_filepath, map_filepath)

def save_csv_map(tile_map, output_path):
    with open(output_path, "w") as csv_file:
        csv_file.write("\n".join(",".join(row) for row in tile_map))
    print(f"Saved CSV map to {output_path}")

def save_files_ini(output_images, output_path, map_filepath):
    with open(output_path, "w") as ini_file:
        ini_file.write(f"[TileSet]\n")
        ini_file.write(f"MapFile={map_filepath}\n")
        ini_file.write(f"FileCount={len(output_images)}\n\n")
        ini_file.write(f"[Tiles]\n")
        for i, file_path in enumerate(output_images):
            ini_file.write(f"ile{i:03}={file_path}\n")
    print(f"Saved files.ini to {output_path}")

def generate_asm_img_load(base_name, asm_levels_filepath, asm_images_filepath, source_dir, target_dir, buffer_id):
    ini_files = [f for f in os.listdir(source_dir) if f.startswith(base_name) and f.endswith(".ini")]
    ini_files.sort()


if __name__ == "__main__":
# chop up the source image into tiles and create metadata files
    base_name = "dg"
    file_name=f"{base_name}.png"
    source_dir=f"tiles/{base_name}"
    target_dir = f"tiles/{base_name}/tiled"
    tile_width = 16
    tile_height = 16
    h_pitch = tile_width
    v_pitch = tile_height
    tiles_x = 32
    tiles_y = 16
    columns = 8
    ranges = [(0, 0),(512, 0),(1024, 0),(0, 240),(512, 240),(1024, 240),]
    # ranges = [(0,0)]
    for set_num, (start_x, start_y) in enumerate(ranges):
        chop_and_deduplicate_tiles(source_dir,target_dir,base_name,file_name,tile_width,tile_height,h_pitch,v_pitch,tiles_x,tiles_y,start_x,start_y,set_num)

# generate the assembly code to load the tiles and define levels
    asm_levels_filepath = "src/asm/levels.inc"
    asm_images_filepath = "src/asm/images_tiles.inc"
    buffer_id = 512
    generate_asm_img_load(base_name, asm_levels_filepath, asm_images_filepath, source_dir, target_dir, buffer_id)