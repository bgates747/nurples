import os
import shutil
from PIL import Image, ImageDraw
import math
import re

import os
import shutil
from PIL import Image, ImageDraw
import math
import re

from PIL import Image

def apply_threshold(img, threshold):
    """
    Applies a binary threshold to the image, converting it to strictly black and white.
    
    :param img: A grayscale PIL Image object.
    :param threshold: The grayscale threshold (0-255) to determine black vs. white.
    :return: The thresholded binary image.
    """
    # Convert image to grayscale first, in case it's not already in that mode
    grayscale_img = img.convert('L')
    
    # Apply the threshold: pixels > threshold become white, others become black
    bw_img = grayscale_img.point(lambda p: 255 if p > threshold else 0, mode='1')
    
    return bw_img


def scale_master_font(master_font_dir, font_name, font_variant, target_width, target_height, threshold):
    """
    Scales the master composite font image to the specified target dimensions and saves it
    as a subdirectory under the font's original directory. Applies a binary threshold to ensure
    the system receives strictly black and white pixels.
    
    :param master_font_dir: The directory containing the master font image and metadata.
    :param font_name: The name of the font (e.g., monospace).
    :param font_variant: The variant of the font (e.g., bold).
    :param target_width: The target width for the scaled font.
    :param target_height: The target height for the scaled font.
    :param threshold: The grayscale threshold (0-255) for converting to black and white.
    """
    font_full_name = f'{font_name}_{font_variant}'
    master_img_filepath = os.path.join(master_font_dir, 'master.png')

    # Load the master composite image
    master_img = Image.open(master_img_filepath).convert('L')  # Grayscale
    master_metadata_filepath = os.path.join(master_font_dir, 'data.txt')

    # Read the metadata to get the original cell dimensions
    with open(master_metadata_filepath, 'r') as f:
        lines = f.readlines()
        orig_width = int(lines[0].split(': ')[1].strip())  # Max width from metadata
        orig_height = int(lines[2].split(': ')[1].strip())  # Height from metadata

    # Precompute the padded width (nearest multiple of 8)
    padded_width = math.ceil(target_width / 8) * 8

    # Calculate grid info
    num_cols, num_rows = 16, 14
    composite_width = padded_width * num_cols
    composite_height = target_height * num_rows

    # Create a new composite image for the scaled font
    composite_img = Image.new('L', (composite_width, composite_height))

    # Scale and paste each character into the new composite
    for row in range(num_rows):
        for col in range(num_cols):
            # Calculate the bounding box for the character in the master image
            x0 = col * orig_width
            y0 = row * orig_height
            x1 = x0 + orig_width
            y1 = y0 + orig_height
            char_img = master_img.crop((x0, y0, x1, y1))

            # Scale the character to the target dimensions using bicubic interpolation
            scaled_char_img = char_img.resize((target_width, target_height), Image.BICUBIC)

            # Apply the threshold to convert the scaled image to black and white
            bw_char_img = apply_threshold(scaled_char_img, threshold)

            # Paste the thresholded character into the new composite image
            x_offset = col * padded_width
            y_offset = row * target_height
            composite_img.paste(bw_char_img, (x_offset, y_offset))

    # Create a subdirectory for the scaled font
    output_subdir = os.path.join('src/assets/img/proc/fonts', font_name, font_variant, f'{target_width}x{target_height}')
    if not os.path.exists(output_subdir):
        os.makedirs(output_subdir)

    # Save the scaled composite image
    scaled_img_filepath = os.path.join(output_subdir, 'scaled.png')
    composite_img.save(scaled_img_filepath)
    print(f'Scaled font image saved as {scaled_img_filepath}')

    # Write metadata for the scaled font
    scaled_metadata_filepath = os.path.join(output_subdir, 'data.txt')
    with open(scaled_metadata_filepath, 'w') as f:
        f.write(f"Target width: {target_width}\n")
        f.write(f"Padded width: {padded_width}\n")
        f.write(f"Height: {target_height}\n")
    print(f'Metadata saved as {scaled_metadata_filepath}')

def scale_and_create_composite(tgt_dir, font_full_name, num_cols, num_rows, target_width, target_height):
    """
    Scales each character image to the target width and height, rounds up the width to the nearest
    multiple of 8, and creates a composite image of the scaled characters. Saves the composite image 
    and generates a metadata file for the new font dimensions.
    
    :param tgt_dir: Directory containing the source composite image.
    :param font_full_name: The full name of the font (used for the output filename).
    :param num_cols: Number of columns in the grid.
    :param num_rows: Number of rows in the grid.
    :param target_width: Target width for each character (before padding).
    :param target_height: Target height for each character.
    """
    # Precompute the padded width (nearest multiple of 8)
    padded_width = math.ceil(target_width / 8) * 8

    # Load the source composite image
    source_img_path = os.path.join(tgt_dir, f'{font_full_name}.png')
    source_img = Image.open(source_img_path).convert('L')  # Open as grayscale

    # Calculate the original width and height of each character (true dimensions)
    orig_width = source_img.width / num_cols
    orig_height = source_img.height / num_rows

    # Create a new blank image for the composite with the new target dimensions
    composite_width = padded_width * num_cols
    composite_height = target_height * num_rows
    composite_img = Image.new('L', (composite_width, composite_height))

    # Scale and paste each character into the composite image
    for row in range(num_rows):
        for col in range(num_cols):
            # Calculate the bounding box of the character in the source image
            x0 = col * orig_width
            y0 = row * orig_height
            x1 = x0 + orig_width
            y1 = y0 + orig_height
            char_img = source_img.crop((x0, y0, x1, y1))

            # Scale the character to the target dimensions using bicubic interpolation
            scaled_char_img = char_img.resize((target_width, target_height), Image.BICUBIC)

            # Calculate the position in the composite image
            x_offset = col * padded_width
            y_offset = row * target_height

            # Paste the scaled image into the composite
            composite_img.paste(scaled_char_img, (x_offset, y_offset))

    # Create the output directory if it doesn't exist
    output_dir = os.path.join(tgt_dir, f'{font_full_name}_{target_width}x{target_height}')
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Save the composite image
    composite_filepath = os.path.join(output_dir, f'{font_full_name}_{target_width}x{target_height}.png')
    composite_img.save(composite_filepath)
    print(f'Scaled composite image saved as {composite_filepath}')

    # Write metadata to text file
    metadata_filepath = os.path.join(output_dir, f'{font_full_name}_{target_width}x{target_height}.txt')
    with open(metadata_filepath, 'w') as f:
        f.write(f"Target width: {target_width}\n")
        f.write(f"Padded width: {padded_width}\n")
        f.write(f"Height: {target_height}\n")
    
    print(f'Metadata saved as {metadata_filepath}')

def create_composite_image_and_cleanup(tgt_dir, font_full_name, num_cols, num_rows):
    """
    Creates a composite image from individual character images in the target directory.
    Deletes the individual images afterward.
    
    :param tgt_dir: Directory containing the individual character PNG images.
    :param font_full_name: The full name of the font (used for the output filename).
    :param num_cols: Number of columns in the grid.
    :param num_rows: Number of rows in the grid.
    """
    # Get all valid 3-digit decimal filenames
    file_list = sorted([f for f in os.listdir(tgt_dir) if f.endswith('.png') and f[:3].isdigit()])
    
    # Open one of the images to get the homogenized dimensions
    sample_img = Image.open(os.path.join(tgt_dir, file_list[0]))
    img_width, img_height = sample_img.size

    # Create a new blank image for the composite
    composite_width = img_width * num_cols
    composite_height = img_height * num_rows
    composite_img = Image.new('L', (composite_width, composite_height))

    # Place each image in the correct position in the composite
    for idx, filename in enumerate(file_list):
        img = Image.open(os.path.join(tgt_dir, filename))
        row = idx // num_cols
        col = idx % num_cols
        x_offset = col * img_width
        y_offset = row * img_height
        composite_img.paste(img, (x_offset, y_offset))
    
    # Save the composite image
    composite_filepath = os.path.join(tgt_dir, f'{font_full_name}.png')
    composite_img.save(composite_filepath)
    print(f'Composite image saved as {composite_filepath}')

    # Delete the individual images
    for filename in file_list:
        os.remove(os.path.join(tgt_dir, filename))
    print(f'Deleted {len(file_list)} individual images.')

def crop_to_max_width(tgt_dir, background_level, font_full_name):
    """
    Finds the maximum rightmost white pixel across all valid images, rounds it up to the nearest
    number divisible by 8, crops all images to this width, and generates a metadata text file.
    Skips non-3-digit decimal filenames.
    
    :param tgt_dir: Directory containing the individual character PNG images.
    :param background_level: Threshold for determining background in grayscale images.
    :param font_full_name: The full name of the font (used for metadata filename).
    """
    max_rightmost_white = 0

    # Regular expression for 3-digit decimal filenames
    valid_file_pattern = re.compile(r'^\d{3}\.png$')

    # First pass: Find the maximum rightmost white pixel across all images
    for filename in os.listdir(tgt_dir):
        if valid_file_pattern.match(filename):
            file_path = os.path.join(tgt_dir, filename)
            img = Image.open(file_path).convert('L')  # Open as grayscale

            # Get image dimensions
            width, height = img.size
            pixels = img.load()

            # Scan columns from right to left to find the maximum rightmost white pixel
            for x in range(width - 1, -1, -1):
                if any(pixels[x, y] > background_level for y in range(height)):  # Check for white pixels based on threshold
                    max_rightmost_white = max(max_rightmost_white, x)
                    break

    # Calculate the new width (rounded up to the nearest multiple of 8)
    max_width = max_rightmost_white + 1
    padded_width = math.ceil(max_width / 8) * 8

    # Write metadata to text file
    metadata_filepath = os.path.join(tgt_dir, f'{font_full_name}.txt')
    with open(metadata_filepath, 'w') as f:
        f.write(f"Max width: {max_width}\n")
        f.write(f"Padded width: {padded_width}\n")
        f.write(f"Height: {height}\n")
    
    print(f'Metadata saved as {metadata_filepath}')

    # Second pass: Crop all images to the computed max width
    for filename in os.listdir(tgt_dir):
        if valid_file_pattern.match(filename):
            file_path = os.path.join(tgt_dir, filename)
            img = Image.open(file_path).convert('L')  # Open as grayscale

            # Crop the image to the new width (same for all images)
            cropped_img = img.crop((0, 0, padded_width, height))

            # Save the cropped image back to the original filename in grayscale
            cropped_img.save(file_path)

    print(f'Cropped images to max width: {max_width}px and padded width: {padded_width}px.')


def draw_bounding_boxes(img, row_start_coords, col_start_coords, save_dir):
    """
    Draws bounding boxes for each cell in the character grid and saves the result.
    
    :param img: PIL Image object (grayscale).
    :param row_start_coords: List of starting y-coordinates for each row.
    :param col_start_coords: List of starting x-coordinates for each column.
    :param save_dir: Directory to save the image with bounding boxes.
    """
    # Convert to RGB for drawing
    img = img.convert("RGB")
    draw = ImageDraw.Draw(img)
    
    # Iterate over rows and columns to draw bounding boxes
    for row_idx, y0 in enumerate(row_start_coords):
        for col_idx, x0 in enumerate(col_start_coords):
            # Determine y1 and x1 for the current character (next start coord minus 1 pixel)
            y1 = row_start_coords[row_idx + 1] - 1 if row_idx + 1 < len(row_start_coords) else img.height
            x1 = col_start_coords[col_idx + 1] - 1 if col_idx + 1 < len(col_start_coords) else img.width

            # Draw the bounding box in cyan (1 pixel wide)
            draw.rectangle([x0, y0, x1, y1], outline="cyan", width=1)

    # img.show()
    return img

def crop_font_image(crop_box, src_img_filepath):
    img = Image.open(src_img_filepath)
    cropped_img = img.crop(crop_box)
    return cropped_img

def convert_to_grayscale(img, background_level):
    """
    Convert the image to grayscale, considering any pixel greater than background_level as white,
    and any less than or equal as black.
    """
    grayscale_img = img.convert('L')
    return grayscale_img

def get_row_and_col_coords(img_width, img_height, num_cols, num_rows):
    """
    Generates the starting coordinates for rows and columns, ensuring exact cell sizes.
    
    :param img_width: Total width of the image.
    :param img_height: Total height of the image.
    :param num_cols: Number of columns in the grid.
    :param num_rows: Number of rows in the grid.
    :return: Two lists: row start coordinates and column start coordinates.
    """
    # Calculate exact cell width and height
    col_width = img_width // num_cols
    row_height = img_height // num_rows

    # Generate start coordinates for each row and column
    col_start_coords = [col_width * i for i in range(num_cols)]
    row_start_coords = [row_height * i for i in range(num_rows)]

    return row_start_coords, col_start_coords

def extract_and_save_characters(img, row_start_coords, col_start_coords, save_dir):
    """
    Extracts individual characters from a grid of characters and saves them as images.
    
    :param img: PIL Image object (grayscale).
    :param row_start_coords: List of starting y-coordinates for each row.
    :param col_start_coords: List of starting x-coordinates for each column.
    :param save_dir: Directory to save the extracted character images.
    """
    char_code = 32  # Starting ASCII code for first character
    
    # Iterate over rows and columns to extract characters
    for row_idx, y0 in enumerate(row_start_coords):
        for col_idx, x0 in enumerate(col_start_coords):
            # Determine y1 and x1 for the current character (next start coord minus 1 pixel)
            y1 = row_start_coords[row_idx + 1] - 1 if row_idx + 1 < len(row_start_coords) else img.height
            x1 = col_start_coords[col_idx + 1] - 1 if col_idx + 1 < len(col_start_coords) else img.width

            # Crop the character from the image
            char_img = img.crop((x0, y0, x1, y1))

            # Save the character as a PNG file with the ASCII code in the filename
            file_name = f'{save_dir}/{char_code:03d}.png'  # Format file name as ddd.png
            char_img.save(file_name)

            char_code += 1  # Move to the next ASCII character

            # Stop after processing all characters from 32 to 255
            if char_code > 255:
                return

def make_font_master(crop_box, src_img_filepath, tgt_master_img_filepath, gridded_img_filepath, metadata_filepath, background_level):
    """
    Parses the original font image, crops, binarizes, and creates a master composite font image.
    Saves the final master image, an intermediate gridded image, and the metadata file.
    
    :param crop_box: The crop box (left, upper, right, lower) for the source image.
    :param src_img_filepath: The path to the original source image (raw.png).
    :param tgt_master_img_filepath: The path to save the final master composite image (master.png).
    :param gridded_img_filepath: The path to save the gridded image (gridded.png) for validation.
    :param metadata_filepath: The path to save the metadata file (data.txt).
    :param background_level: Grayscale threshold for the background.
    """
    
    # Crop and convert the image to grayscale
    img = crop_font_image(crop_box, src_img_filepath)
    img = convert_to_grayscale(img, background_level)

    # Image dimensions and grid info
    img_width, img_height = img.size
    num_cols, num_rows = 16, 14  # Fixed grid layout for this font

    # Get row and column start coordinates
    row_start_coords, col_start_coords = get_row_and_col_coords(img_width, img_height, num_cols, num_rows)

    # Draw bounding boxes for each cell and save the gridded image
    img_with_boxes = draw_bounding_boxes(img, row_start_coords, col_start_coords, gridded_img_filepath)
    img_with_boxes.save(gridded_img_filepath)  # Save the gridded image

    # Extract and save characters temporarily for processing
    temp_dir = os.path.join(os.path.dirname(metadata_filepath), 'temp_characters')
    if os.path.exists(temp_dir):
        shutil.rmtree(temp_dir)
    os.makedirs(temp_dir)

    extract_and_save_characters(img, row_start_coords, col_start_coords, temp_dir)

    # Crop to max width, generate metadata, and save individual images
    font_full_name = os.path.splitext(os.path.basename(tgt_master_img_filepath))[0]
    crop_to_max_width(temp_dir, background_level, font_full_name)

    # Create the master composite image from the cropped characters
    create_composite_image_and_cleanup(temp_dir, font_full_name, num_cols, num_rows)

    # Move the final master composite image to the correct location
    final_composite_img = os.path.join(temp_dir, f'{font_full_name}.png')
    shutil.move(final_composite_img, tgt_master_img_filepath)

    # Move the metadata to the correct location
    temp_metadata = os.path.join(temp_dir, f'{font_full_name}.txt')
    shutil.move(temp_metadata, metadata_filepath)

    # Clean up temp directory
    shutil.rmtree(temp_dir)

    print(f"Master font '{font_full_name}' created and saved successfully.")

def adjust_dimensions(screen_width, screen_height, char_cols, char_rows):
    """
    Adjusts the character dimensions to the nearest values such that the screen width and height
    are evenly divisible by the target character width and height.

    :param screen_width: The width of the screen in pixels.
    :param screen_height: The height of the screen in pixels.
    :param char_cols: The number of columns (characters) across the screen.
    :param char_rows: The number of rows (characters) down the screen.
    :return: Adjusted target character width and height.
    """

    # Ideal width and height per character
    ideal_width = screen_width / char_cols
    ideal_height = screen_height / char_rows

    # Round to the nearest integer such that screen resolution is divisible by character dimensions
    target_width = round(ideal_width)
    target_height = round(ideal_height)

    # Adjust width and height to ensure exact divisibility
    while screen_width % target_width != 0:
        target_width += 1 if ideal_width < target_width else -1

    while screen_height % target_height != 0:
        target_height += 1 if ideal_height < target_height else -1

    return target_width, target_height

if __name__ == '__main__':
    # crop_box = (0, 100, 1632, 1008)  # (left, upper, right, lower)
    # font_name = 'monospace'
    # font_variant = 'bold'
    # font_full_name = f'{font_name}_{font_variant}'

    char_width = 58.25
    char_height = 67.1428571428571428571

    crop_box_left = 0
    crop_box_top = 100
    crop_box_right = char_width * 16 + crop_box_left - 1
    crop_box_bottom = char_height * 14 + crop_box_top - 1

    crop_box = (crop_box_left, crop_box_top, crop_box_right, crop_box_bottom)
    font_name = 'notosansmono'
    font_variant = 'regular'
    font_full_name = f'{font_name}_{font_variant}'

    background_level = 80  # Define the background level in grayscale
    # Define the input (source) directories according to the new scheme
    src_dir = f'src/assets/img/orig/fonts/masters/{font_name}/{font_variant}'
    src_img_filepath = f'{src_dir}/raw.png'  # Raw input image
    tgt_master_img_filepath = f'{src_dir}/master.png'  # Final master image
    gridded_img_filepath = f'{src_dir}/gridded.png'  # Gridded intermediate validation image
    metadata_filepath = f'{src_dir}/data.txt'  # Metadata file

    # Conditional block for master font creation, controlled by `if False:`
    if True:
        # Call the function to create the master font
        make_font_master(crop_box, src_img_filepath, tgt_master_img_filepath, gridded_img_filepath, metadata_filepath, background_level)
        print(f"Master font created at: {tgt_master_img_filepath}")



    # List of character resolutions to process
    character_resolutions = [
        (8, 10), 
        (8, 20), (10, 25), (16, 10), (16, 16),
        (16, 20), (16, 32), (20, 25), (32, 32)
    ]

    # Loop through the character resolutions and scale the master font
    for target_width, target_height in character_resolutions:
        # Define the output (target) directories for scaling based on the new scheme
        tgt_proc_dir = f'src/assets/img/proc/fonts/{font_name}/{font_variant}/{target_width}x{target_height}'
        scaled_img_filepath = f'{tgt_proc_dir}/scaled.png'
        scaled_metadata_filepath = f'{tgt_proc_dir}/data.txt'

        # Ensure the target directory exists
        if not os.path.exists(tgt_proc_dir):
            os.makedirs(tgt_proc_dir)

        # Call the scaling function for each character resolution
        scale_master_font(src_dir, font_name, font_variant, target_width, target_height,threshold=127-8
                          )
        
        print(f"Generated scaled font for {target_width}x{target_height} at {scaled_img_filepath}")

    img = Image.open('src/assets/img/proc/fonts/notosansmono/regular/16x20/scaled.png')
    img.show()