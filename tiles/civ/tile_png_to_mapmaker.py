import os, shutil, csv
from PIL import Image
from agonImages import convert_to_agon_palette, img_to_rgba2, img_to_rgba8
from hex_compare import hex_dump_comparison

def png_to_mapmaker(source_dir, mapmaker_dir, tile_width, tile_height):
    files = []
    for file_name in os.listdir(source_dir):
        if file_name.endswith('.png') and file_name != 'review_image.png':
            files.append(file_name)

    files.sort()

    dir_num = 0
    file_num = 0

    for file_name in files:
        if file_num == 0:
            dir_num += 1
            dir_path = os.path.join(mapmaker_dir, str(dir_num))
            if os.path.exists(dir_path):
                shutil.rmtree(dir_path)
            os.makedirs(dir_path)
        src_filepath = os.path.join(source_dir, file_name)
        tgt_filepath = os.path.join(dir_path, str(file_num) + '.rgba8')
        src_img = Image.open(src_filepath)
        img_to_rgba8(src_img, tgt_filepath)
        file_num += 1
        if file_num == 10:
            file_num = 0

    if file_num < 10:
        img = Image.new('RGBA', (tile_width, tile_height), (255, 0, 255, 255))
        for i in range(file_num, 10):
            tgt_filepath = os.path.join(dir_path, str(i) + '.rgba8')
            img_to_rgba8(img, tgt_filepath)
            
NULL_TILE = 1  # Constant for the null tile

def create_mapmaker_file(mapmaker_dir, mapmaker_filepath, grid_width, grid_height):
    # Adjust metadata for the app's off-by-one behavior
    metadata_width = grid_width - 1
    metadata_height = grid_height - 1

    # Initialize map grid with the null tile value
    map_grid = [[NULL_TILE for _ in range(grid_width)] for _ in range(grid_height)]

    # Traverse tile banks (1-based directories)
    bank_num = 1
    row = 0

    while bank_num <= grid_height:
        bank_dir = os.path.join(mapmaker_dir, str(bank_num))
        if not os.path.exists(bank_dir):
            break  # No more banks
        
        # List files in the bank directory, sorted by their numeric order
        tile_files = sorted(
            [f for f in os.listdir(bank_dir) if f.endswith('.rgba8')],
            key=lambda x: int(os.path.splitext(x)[0])
        )

        # Assign tiles to the current row
        col = 0
        for tile_idx, tile_file in enumerate(tile_files):
            if col >= grid_width:
                break  # Prevent overflowing the row
            # Calculate the tile ID based on the bank and position
            tile_id = (bank_num - 1) * 10 + tile_idx + 10
            map_grid[row][col] = tile_id
            col += 1

        # Move to the next row for the next bank
        row += 1
        if row >= grid_height:
            break  # Prevent overflowing the grid

        bank_num += 1

    # Write the mapmaker file
    with open(mapmaker_filepath, 'wb') as map_file:
        # Write the header as 5-byte integers
        map_file.write(metadata_width.to_bytes(5, 'little'))  # Grid width
        map_file.write(metadata_height.to_bytes(5, 'little'))  # Grid height
        map_file.write((bank_num - 1).to_bytes(5, 'little'))  # Number of banks
        map_file.write((0).to_bytes(5, 'little'))  # Placeholder for custom tiles

        # Write the map data row by row
        for row in map_grid:
            for cell in row:
                # Write each tile ID as 5 bytes
                map_file.write(cell.to_bytes(5, 'little'))

def validate_mapmaker_file(map_file):
    # Generate the output validation filename
    validation_file = os.path.splitext(map_file)[0] + '.txt'

    # Open the map file for reading
    with open(map_file, 'rb') as file:
        # Read header information as 5-byte integers
        grid_width = int.from_bytes(file.read(5), 'little')
        grid_height = int.from_bytes(file.read(5), 'little')
        num_banks = int.from_bytes(file.read(5), 'little')
        custom_tiles = int.from_bytes(file.read(5), 'little')

        # Initialize the data structure for storing map data
        map_data = []

        # Read the map grid
        for _ in range(grid_height):
            row = []
            for _ in range(grid_width):
                # Each cell is a 5-byte integer
                cell_value = int.from_bytes(file.read(5), 'little')
                row.append(cell_value)
            map_data.append(row)

    # Write the validation file
    with open(validation_file, 'w') as outfile:
        # Write header information
        outfile.write(f"Grid Width: {grid_width}\n")
        outfile.write(f"Grid Height: {grid_height}\n")
        outfile.write(f"Number of Banks: {num_banks}\n")
        outfile.write(f"Custom Tiles: {custom_tiles}\n\n")

        # Write map data in human-readable format
        outfile.write("Map Data (Human-Readable):\n")
        for row in map_data:
            outfile.write(' '.join(f"{cell:5}" for cell in row) + '\n')

        # Write map data in machine-readable format
        outfile.write("\nMap Data (Machine-Readable):\n")
        for row in map_data:
            outfile.write(','.join(str(cell) for cell in row) + '\n')

    print(f"Validation file written: {validation_file}")

def csv_to_mapmaker(csv_filepath, mapmaker_filepath, sample_width, sample_height, offset_row, offset_col, num_banks):
    """
    Creates a mapmaker file from a CSV file, sampling data with specified width, height, and offsets.

    :param csv_filepath: Path to the CSV file.
    :param mapmaker_filepath: Path to save the resulting .map file.
    :param sample_width: Width of the sample area.
    :param sample_height: Height of the sample area.
    :param offset_row: Starting row offset in the CSV file.
    :param offset_col: Starting column offset in the CSV file.
    """
    # Read the CSV file
    with open(csv_filepath, 'r') as file:
        reader = csv.reader(file)
        csv_data = [list(map(int, row)) for row in reader]

    # Dimensions of the CSV file
    csv_rows = len(csv_data)
    csv_cols = len(csv_data[0]) if csv_rows > 0 else 0

    # Initialize a 15x15 grid with null tiles
    grid_width, grid_height = 15, 15
    map_grid = [[NULL_TILE for _ in range(grid_width)] for _ in range(grid_height)]

    # Populate the map grid based on the sample dimensions and offset
    for row in range(sample_height):
        for col in range(sample_width):
            target_row = row + offset_row
            target_col = col + offset_col

            if target_row < csv_rows and target_col < csv_cols:
                # Valid CSV tile: Add 10 to the value
                map_grid[row][col] = csv_data[target_row][target_col] + 10

    # Adjust grid dimensions for the metadata
    metadata_width = grid_width - 1
    metadata_height = grid_height - 1

    # Write the mapmaker file
    with open(mapmaker_filepath, 'wb') as map_file:
        # Write the header as 5-byte integers
        map_file.write(metadata_width.to_bytes(5, 'little'))  # Grid width
        map_file.write(metadata_height.to_bytes(5, 'little'))  # Grid height
        map_file.write((num_banks).to_bytes(5, 'little'))  # Number of banks
        map_file.write((0).to_bytes(5, 'little'))  # Placeholder for custom tiles

        # Write the map data row by row
        for row in map_grid:
            for cell in row:
                # Write each tile ID as 5 bytes
                map_file.write(cell.to_bytes(5, 'little'))

    print(f"Mapmaker file created: {mapmaker_filepath}")

if __name__ == "__main__":
    set_num = '01'
    map_num = '02'
    num_banks = 5

    source_dir = f'tiles/sprites/dead_gunner_sms_lg/dead_gunner_sms_{set_num}'
    mapmaker_dir = os.path.join(source_dir, 'mapmaker')
    tile_width = 16
    tile_height = 16
    # Convert PNG images to the mapmaker directory
    png_to_mapmaker(source_dir, mapmaker_dir, tile_width, tile_height)

    # Create the mapmaker file
    mapmaker_filepath = os.path.join(mapmaker_dir, 'tiles.map')
    grid_width = 15
    grid_height = 15
    create_mapmaker_file(mapmaker_dir, mapmaker_filepath, grid_width, grid_height)
    validate_mapmaker_file(mapmaker_filepath)

    # Create a mapmaker file from a CSV file
    mapmaker_filepath = os.path.join(mapmaker_dir, f'dg_{set_num}_{map_num}.map')
    csv_filename = f'dead_gunner_sms_{set_num}.csv'
    csv_filepath = os.path.join(source_dir, csv_filename)
    sample_width = 15
    sample_height = 15
    offset_row = 0
    offset_col = int(map_num) * sample_width
    csv_to_mapmaker(csv_filepath, mapmaker_filepath, sample_width, sample_height, offset_row, offset_col, num_banks)
    validate_mapmaker_file(mapmaker_filepath)