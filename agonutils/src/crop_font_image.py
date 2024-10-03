import os
import shutil
from PIL import Image, ImageDraw
import math
import re

def crop_to_max_width(tgt_dir):
    """
    Finds the maximum rightmost white pixel across all valid images, rounds it up to the nearest
    number divisible by 8, and crops all images to this width. Skips non-3-digit decimal filenames.
    
    :param tgt_dir: Directory containing the individual character PNG images.
    """
    max_rightmost_white = 0

    # Regular expression for 3-digit decimal filenames
    valid_file_pattern = re.compile(r'^\d{3}\.png$')

    # First pass: Find the maximum rightmost white pixel across all images
    for filename in os.listdir(tgt_dir):
        if valid_file_pattern.match(filename):
            file_path = os.path.join(tgt_dir, filename)
            img = Image.open(file_path).convert('1')  # Open as black-and-white

            # Get image dimensions
            width, height = img.size
            pixels = img.load()

            # Scan columns from right to left to find the maximum rightmost white pixel
            for x in range(width - 1, -1, -1):
                if any(pixels[x, y] == 255 for y in range(height)):  # Check for a white pixel
                    max_rightmost_white = max(max_rightmost_white, x)
                    break

    # Calculate the new width (rounded up to the nearest multiple of 8)
    new_width = math.ceil((max_rightmost_white + 1) / 8) * 8
    # print(f'Maximum rightmost white pixel found at: {max_rightmost_white}, cropping width: {new_width}')

    # Second pass: Crop all images to the computed max width
    for filename in os.listdir(tgt_dir):
        if valid_file_pattern.match(filename):
            file_path = os.path.join(tgt_dir, filename)
            img = Image.open(file_path).convert('1')  # Open as black-and-white

            # Get image height
            _, height = img.size

            # Crop the image to the new width (same for all images)
            cropped_img = img.crop((0, 0, new_width, height))

            # Save the cropped image back to the original filename
            cropped_img.save(file_path)
            # print(f'Cropped {filename} to {new_width}px width.')

def draw_bounding_boxes(img, row_start_coords, col_start_coords, save_dir):
    """
    Draws bounding boxes for each cell in the character grid and saves the result.
    
    :param img: PIL Image object (binarized).
    :param row_start_coords: List of starting y-coordinates for each row.
    :param col_start_coords: List of starting x-coordinates for each column.
    :param save_dir: Directory to save the image with bounding boxes.
    """
    # Create a copy of the image to draw on
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

    return img


def crop_font_image(crop_box, src_img_filepath):
    img = Image.open(src_img_filepath)
    cropped_img = img.crop(crop_box)
    return cropped_img

def binarize_image(img, threshold):
    grayscale_img = img.convert('L')
    bw_img = grayscale_img.point(lambda p: 255 if p > threshold else 0, '1')
    return bw_img

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
    
    :param img: PIL Image object.
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

def make_font(crop_box, src_img_filepath, font_name, font_variant, bin_threshold):
    # Remove all files in the target directory
    if os.path.exists(tgt_dir):
        shutil.rmtree(tgt_dir)
    os.makedirs(tgt_dir, exist_ok=True)

    # Crop and binarize the image
    img = crop_font_image(crop_box, src_img_filepath)
    # img = binarize_image(img, bin_threshold)

    # Image dimensions and grid info
    img_width, img_height = img.size
    num_cols, num_rows = 16, 14

    # Get row and column start coordinates
    row_start_coords, col_start_coords = get_row_and_col_coords(img_width, img_height, num_cols, num_rows)

    # Draw bounding boxes for each cell
    img_with_boxes = draw_bounding_boxes(img, row_start_coords, col_start_coords, tgt_dir)
    img_with_boxes.save(f'{tgt_dir}/bounding_boxes.png')
    # img_with_boxes.show()

    # Extract and save characters
    extract_and_save_characters(img, row_start_coords, col_start_coords, tgt_dir)

    # Crop to max width
    crop_to_max_width(tgt_dir)

if __name__ == '__main__':
    bin_threshold = 128+64
    crop_box = (0, 100, 1632, 1008)  # (left, upper, right, lower)
    font_name = 'monospace'
    font_variant = 'bold'
    font_full_name = f'{font_name}_{font_variant}'
    src_dir = f'src/assets/img/orig/fonts/{font_name}'
    src_img_filepath = f'{src_dir}/{font_full_name}.png'
    tgt_dir = f'{src_dir}/{font_full_name}'

    make_font(crop_box, src_img_filepath, font_name, font_variant, bin_threshold)
