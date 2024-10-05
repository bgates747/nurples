import os
import shutil
from PIL import Image, ImageDraw, ImageOps
import math
import re

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

# =======================================================================

def create_composite_image_and_cleanup(cropped_images, tgt_dir, font_full_name):
    """
    Creates a composite image from the list of cropped character images.
    Deletes the individual images afterward.
    
    :param cropped_images: List of cropped PIL Image objects.
    :param tgt_dir: Directory to save the final composite image.
    :param font_full_name: The full name of the font (used for the output filename).
    """
    # Set the hardcoded number of columns and rows
    num_cols = 16
    num_rows = 14

    # Get dimensions from the first image (all images should be the same size)
    img_width, img_height = cropped_images[0].size

    # Calculate the composite image dimensions
    composite_width = img_width * num_cols
    composite_height = img_height * num_rows
    composite_img = Image.new('L', (composite_width, composite_height))  # Create grayscale image

    # Place each image in the correct position in the composite
    for idx, img in enumerate(cropped_images):
        row = idx // num_cols
        col = idx % num_cols
        x_offset = col * img_width
        y_offset = row * img_height
        composite_img.paste(img, (x_offset, y_offset))
    
    # Save the composite image
    composite_filepath = os.path.join(tgt_dir, f'{font_full_name}.png')
    composite_img.save(composite_filepath)
    print(f'Composite image saved as {composite_filepath}')

    # No need to delete images since we're working with the list directly
    print(f'Processed {len(cropped_images)} images for composite.')
    
    return composite_img

def crop_to_max_bbox(char_images, threshold_level, font_full_name, tgt_dir):
    """
    Finds the minimum top-left and maximum bottom-right bounding box across all images, 
    crops all images to these coordinates (same for all), rounds the width up to the nearest
    multiple of 8, and generates a metadata text file.
    
    :param char_images: List of PIL Image objects (grayscale).
    :param threshold_level: Threshold for determining background in grayscale images.
    :param font_full_name: The full name of the font (used for metadata filename).
    :param tgt_dir: Directory to save the cropped images.
    """
    min_left = float('inf')
    min_top = float('inf')
    max_right = 0
    max_bottom = 0

    # First pass: Determine the overall bounding box for all images
    for img in char_images:

        # Apply a threshold to remove anti-aliasing effects (convert to binary image)
        binary_img = img.point(lambda p: p > threshold_level and 255)

        # Get the bounding box of the content
        bbox = binary_img.getbbox()

        if bbox:
            min_left = min(min_left, bbox[0])
            min_top = min(min_top, bbox[1])
            max_right = max(max_right, bbox[2])
            max_bottom = max(max_bottom, bbox[3])

    # Calculate the new width (rounded up to the nearest multiple of 8)
    crop_width = max_right - min_left
    padded_width = math.ceil(crop_width / 8) * 8

    # Write metadata to a text file
    metadata_filepath = os.path.join(tgt_dir, f'data.txt')
    with open(metadata_filepath, 'w') as f:
        f.write(f"Min left: {min_left}\n")
        f.write(f"Min top: {min_top}\n")
        f.write(f"Max right: {max_right}\n")
        f.write(f"Max bottom: {max_bottom}\n")
        f.write(f"Original width: {crop_width}px\n")
        f.write(f"Padded width: {padded_width}px\n")
        f.write(f"Height: {max_bottom - min_top}px\n")
    
    print(f'Metadata saved as {metadata_filepath}')

    # Second pass: Crop all images to the computed bounding box and padded width
    cropped_images = []
    for img in char_images:
        # Crop the image to the computed bounding box and padded width
        cropped_img = img.crop((min_left, min_top, min_left + padded_width, max_bottom))
        cropped_images.append(cropped_img)

    print(f'Cropped images saved to {tgt_dir}, all with the same dimensions.')

    # Return the list of cropped images
    return cropped_images

def extract_chars(img, row_start_coords, col_start_coords, gridded_img_filepath):
    """
    Draws bounding boxes for each cell in the character grid, saves the gridded image for validation,
    and extracts characters from the image as grayscale images.
    
    :param img: PIL Image object (grayscale).
    :param row_start_coords: List of starting y-coordinates for each row.
    :param col_start_coords: List of starting x-coordinates for each column.
    :param gridded_img_filepath: Filepath to save the image with bounding boxes.
    :return: List of cropped character images and the gridded image with bounding boxes.
    """
    print(row_start_coords, col_start_coords)

    # Convert a copy of the image to RGB for drawing the grid (bounding boxes)
    gridded_img = img.convert("RGB")
    draw = ImageDraw.Draw(gridded_img)
    
    # Calculate the cell height and width
    cell_height = row_start_coords[1] - row_start_coords[0]
    cell_width = col_start_coords[1] - col_start_coords[0]

    # Initialize a list to hold cropped character images
    char_images = []

    # Iterate over rows and columns to draw bounding boxes and extract characters
    for y0 in row_start_coords:
        for x0 in col_start_coords:
            # Calculate the bottom and right coordinates for each cell
            y1 = y0 + cell_height
            x1 = x0 + cell_width

            # Draw the bounding box in cyan (1 pixel wide)
            draw.rectangle([x0, y0, x1, y1], outline="cyan", width=1)

            # Crop the character from the original grayscale image (no color conversion here)
            char_img = img.crop((x0+1, y0+1, x1, y1))

            # Append the cropped character image to the list
            char_images.append(char_img)

    # Save the gridded image (with bounding boxes) for validation
    gridded_img.save(gridded_img_filepath)

    # Return both the list of cropped grayscale character images and the gridded image
    return char_images, gridded_img


def get_row_and_col_coords(cell_width, cell_height, num_cols, num_rows):
    """
    Generates the starting coordinates for rows and columns, ensuring exact cell sizes.
    
    :param cell_width: Width of each cell in the grid.
    :param cell_height: Height of each cell in the grid.
    :param num_cols: Number of columns in the grid.
    :param num_rows: Number of rows in the grid.
    :return: Two lists: row start coordinates and column start coordinates.
    """

    # Generate start coordinates for each row and column
    col_start_coords = [cell_width * i for i in range(num_cols)]
    row_start_coords = [cell_height * i for i in range(num_rows)]

    return row_start_coords, col_start_coords

def convert_to_grayscale(img):
    """
    Convert the image to grayscale, considering any pixel greater than threshold_level as white,
    and any less than or equal as black.
    """
    grayscale_img = img.convert('L')
    return grayscale_img

def crop_font_image(crop_box, src_img_filepath):
    # Open the image
    img = Image.open(src_img_filepath)

    # Get image dimensions
    img_width, img_height = img.size

    # Extract crop box dimensions
    crop_box_left, crop_box_top, crop_box_right, crop_box_bottom = crop_box
    crop_box_width = crop_box_right - crop_box_left
    crop_box_height = crop_box_bottom - crop_box_top

    # Check if the image dimensions match the target crop dimensions
    if img_width == crop_box_width and img_height == crop_box_height:
        print(f"Image dimensions already match the target dimensions: {img_width}x{img_height}. No cropping needed.")
        return img  # No cropping is needed, return the original image

    # Crop the image if dimensions do not match
    cropped_img = img.crop(crop_box)

    # Save the cropped image back to the original filename
    # cropped_img.save(src_img_filepath)
    # print(f"Image cropped to {crop_box_width}x{crop_box_height} and saved to {src_img_filepath}.")

    return cropped_img

def make_font_master(tgt_dir, crop_box, src_img_filepath, tgt_master_img_filepath, gridded_img_filepath, threshold_level, cell_width, cell_height):
    """
    Parses the original font image, crops, binarizes, and creates a master composite font image.
    Saves the final master image, an intermediate gridded image, and the metadata file.
    
    :param crop_box: The crop box (left, upper, right, lower) for the source image.
    :param src_img_filepath: The path to the original source image (raw.png).
    :param tgt_master_img_filepath: The path to save the final master composite image (master.png).
    :param gridded_img_filepath: The path to save the gridded image (gridded.png) for validation.
    :param metadata_filepath: The path to save the metadata file (data.txt).
    :param threshold_level: Grayscale threshold for the background.
    """
    
    # Crop and convert the image to grayscale
    input_img = crop_font_image(crop_box, src_img_filepath)
    input_img = convert_to_grayscale(input_img)

    # Invert colors in input_img
    input_img = ImageOps.invert(input_img)

    # return input_img

    num_cols, num_rows = 16, 14

    # Get row and column start coordinates
    row_start_coords, col_start_coords = get_row_and_col_coords(cell_width, cell_height, num_cols, num_rows)

    # Draw bounding boxes for each cell and save the gridded image
    char_images, gridded_img = extract_chars(input_img, row_start_coords, col_start_coords, gridded_img_filepath)

    # return gridded_img

    # Crop to max width, generate metadata, and save individual images
    font_full_name = os.path.splitext(os.path.basename(tgt_master_img_filepath))[0]
    cropped_images = crop_to_max_bbox(char_images, threshold_level, font_full_name, tgt_dir)

    # Create the master composite image from the cropped characters
    print(f"Master font '{font_full_name}' created and saved successfully.")
    return create_composite_image_and_cleanup(cropped_images, tgt_dir, font_full_name)

if __name__ == '__main__':
    crop_box_left = 130
    crop_box_top = 253
    cell_width = 75
    cell_height = 47

    crop_box_width = cell_width * 16
    crop_box_height = cell_height * 14
    crop_box_right = crop_box_left + crop_box_width + 1
    crop_box_bottom = crop_box_top + crop_box_height + 1


    crop_box = (crop_box_left, crop_box_top, crop_box_right, crop_box_bottom)
    font_name = 'verminvibes'
    font_variant = 'normal'
    font_full_name = f'{font_name}_{font_variant}'

    threshold_level = 128  # Define the background level in grayscale
    # Define the input (source) directories according to the new scheme
    src_dir = f'src/assets/img/orig/fonts/masters/{font_name}/{font_variant}'
    src_img_filepath = f'{src_dir}/raw.png'  # Raw input image
    tgt_master_img_filepath = f'{src_dir}/master.png'  # Final master image
    gridded_img_filepath = f'{src_dir}/gridded.png'  # Gridded intermediate validation image
    metadata_filepath = f'{src_dir}/data.txt'  # Metadata file

    # Conditional block for master font creation, controlled by `if False:`
    if True:
        # Call the function to create the master font
        master_img = make_font_master(src_dir, crop_box, src_img_filepath, tgt_master_img_filepath, gridded_img_filepath, threshold_level, cell_width, cell_height)
        print(f"Master font created at: {tgt_master_img_filepath}")

        master_img.show()


    # # List of character resolutions to process
    # character_resolutions = [
    #     (8, 10), 
    #     (8, 20), (10, 25), (16, 10), (16, 16),
    #     (16, 20), (16, 32), (20, 25), (32, 32)
    # ]

    # # Loop through the character resolutions and scale the master font
    # for target_width, target_height in character_resolutions:
    #     # Define the output (target) directories for scaling based on the new scheme
    #     tgt_proc_dir = f'src/assets/img/proc/fonts/{font_name}/{font_variant}/{target_width}x{target_height}'
    #     scaled_img_filepath = f'{tgt_proc_dir}/scaled.png'
    #     scaled_metadata_filepath = f'{tgt_proc_dir}/data.txt'

    #     # Ensure the target directory exists
    #     if not os.path.exists(tgt_proc_dir):
    #         os.makedirs(tgt_proc_dir)

    #     # Call the scaling function for each character resolution
    #     scale_master_font(src_dir, font_name, font_variant, target_width, target_height,threshold=127-8
    #                       )
        
    #     print(f"Generated scaled font for {target_width}x{target_height} at {scaled_img_filepath}")

    # img = Image.open('src/assets/img/proc/fonts/notosansmono/regular/16x20/scaled.png')
    # img.show()