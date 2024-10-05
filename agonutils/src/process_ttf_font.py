import os
from PIL import Image, ImageDraw, ImageFont
import numpy as np
from fontTools.ttLib import TTFont
import matplotlib.pyplot as plt

def white_to_black(image):
    """
    Changes all pure white pixels (255) to black (0) in a grayscale image.
    
    :param image: A PIL Image object in "L" mode (grayscale).
    :return: A modified PIL Image object with white pixels changed to black.
    """
    # Ensure the image is in grayscale mode
    if image.mode != 'L':
        raise ValueError("Image must be in grayscale (L) mode")
    
    # Convert the image to a NumPy array for faster manipulation
    pixel_data = np.array(image)

    # Change all white pixels (255) to black (0)
    pixel_data[pixel_data == 255] = 0

    # Convert back to a PIL image
    return Image.fromarray(pixel_data)

def grayscale_histogram(image):
    """
    Returns and plots the histogram of values in a grayscale image, excluding pixels with values 0 and 255.
    
    :param image: A PIL Image object in "L" mode (grayscale).
    :return: A list representing the histogram (pixel intensity counts from 1 to 254).
    """
    # Ensure the image is in grayscale mode
    if image.mode != 'L':
        raise ValueError("Image must be in grayscale (L) mode")

    # Get the histogram of the image
    histogram = image.histogram()

    # Exclude values for pixel intensity 0 and 255
    filtered_histogram = histogram[1:255]  # Pixels from 1 to 254

    # Return the filtered histogram
    return filtered_histogram

def has_gray_pixels(image):
    """
    Checks if a grayscale image contains any gray pixels (i.e., values between 1 and 254).
    
    :param image: A PIL Image object in "L" mode (grayscale).
    :return: True if there are any gray pixels, False otherwise.
    """
    # Convert the PIL image to an 8-bit grayscale image if it is not already
    if image.mode != "L":
        image = image.convert("L")

    # Convert the PIL image to a NumPy array
    pixel_data = np.array(image)

    # Check if there are any pixels with values between 1 and 254
    return np.any((pixel_data > 0) & (pixel_data < 255))

def extract_bitmap_glyphs(ttf_path):
    """
    Extracts embedded bitmap glyphs from a .ttf file and returns them in a list of images

    :param ttf_path: Path to the .ttf file.
    """
    # Create an empty list to hold the images
    images = []

    # Load the font
    font = TTFont(ttf_path)

    # Check if the font has embedded bitmaps by looking for the EBDT and EBLC tables
    if 'EBDT' not in font or 'EBLC' not in font:
        print("No embedded bitmap glyphs found in this font.")
        return

    # Get the EBLC (Embedded Bitmap Location) and EBDT (Embedded Bitmap Data) tables
    eblc = font['EBLC']
    ebdt = font['EBDT']

    # Iterate through the bitmap strikes (sizes) available in the font
    for strike in eblc.strikes:
        print(f"Extracting bitmaps for strike size: {strike.ppem}")

        # Iterate through each glyph index in the strike
        for glyph_id, location in strike.glyphLocations.items():
            glyph_name = font.getGlyphName(glyph_id)

            # Get the bitmap data for the glyph
            bitmap_data = ebdt.getBitmap(glyph_id, location)
            if bitmap_data:
                # Convert the bitmap to an image (PIL Image)
                img = Image.frombytes('L', bitmap_data['size'], bitmap_data['bitmap'], 'raw')
                
                # Add the image to the list along with the glyph name
                images.append((glyph_name, img))

    return images

def quantize_to_palette(img, palette=(0, 85, 170, 255)):
    """
    Quantizes a grayscale image to a limited palette.
    
    :param img: The grayscale image to quantize.
    :param palette: The limited color palette (defaults to 0, 85, 170, and 255).
    :return: The quantized image.
    """
    # Convert the image to a NumPy array
    img_np = np.array(img)
    
    # Find the closest palette value for each pixel
    quantized_img_np = np.zeros_like(img_np)
    for i, color in enumerate(palette):
        mask = np.abs(img_np - color) <= np.abs(img_np - quantized_img_np)
        quantized_img_np[mask] = color

    # Convert back to a PIL image and return
    quantized_img = Image.fromarray(quantized_img_np, mode="L")
    return quantized_img

def char_is_defined(image, bbox):
    """
    Checks whether the character image (cropped to its bounding box) is empty by inspecting the 
    outermost pixels (top row, bottom row, leftmost column, and rightmost column). If any of these
    pixels are black, the character is considered defined.

    :param image: The original PIL Image object representing the character.
    :param bbox: The bounding box (left, upper, right, lower) for the character.
    :return: False if the character image is empty, True otherwise.
    """
    # Crop the image to the bounding box
    cropped_image = image.crop(bbox)
    pixels = cropped_image.load()  # Get pixel data from the cropped image
    width, height = cropped_image.size

    # Check top row
    for x in range(width):
        if pixels[x, 0] == 0: 
            return True

    # Check bottom row
    for x in range(width):
        if pixels[x, height - 1] == 0:  
            return True

    # Check leftmost column
    for y in range(height):
        if pixels[0, y] == 0:  
            return True

    # Check rightmost column
    for y in range(height):
        if pixels[width - 1, y] == 0:
            return True

    # If all edge pixels are white, the character is considered undefined
    return False

def render_and_measure_characters(font_path, point_size, threshold, char_range=(32, 127)):
    """
    Renders each character once, measures its bounding box, and returns a dictionary
    with character images and their dimensions (from origin). Also computes the max width and height.

    :param font_path: Path to the .ttf font file.
    :param point_size: Point size to use for rendering the characters.
    :param char_range: Range of characters to render (default is from ASCII 32 to 127).
    :return: A dictionary of character images with their dimensions and max width/height.
    """
    # Load the font
    font = ImageFont.truetype(font_path, point_size)
    
    char_images = {}
    max_width, max_height = 0, 0  # To track the maximum dimensions of the characters

    # Loop through each character in the specified range
    for char_code in range(char_range[0], char_range[1] + 1):
        char = chr(char_code)

        # Create a temporary image to render the character (black background, white text)
        temp_img = Image.new("L", (256, 256), color=0)  # "L" mode for grayscale, black background
        draw = ImageDraw.Draw(temp_img)

        draw.text((0, 0), char, font=font, fill=255)

        # invert colors for bounding box calculation
        inverted_img = Image.new("L", (256, 256), color=255)
        inverted_img.paste(temp_img)

        # Threshhold the image
        inverted_img = quantize_to_palette(inverted_img, palette=(0, threshold, 255))

        # Find the character's bounding box
        bbox = inverted_img.getbbox()  # Returns (left, upper, right, lower) bounding box

        if bbox:
            if char_is_defined(temp_img, bbox):
                width = bbox[2]  # Lower right corner x-coordinate gives the width from the origin
                height = bbox[3]  # Lower right corner y-coordinate gives the height from the origin

                # Track the maximum width and height across all characters
                max_width = max(max_width, width)
                max_height = max(max_height, height)
            else:
                width, height = 0,0

            # Save the character image and dimensions in the dictionary
            char_images[char_code] = (temp_img, width, height)

    return char_images, max_width, max_height

def create_master_image(char_images, max_width, max_height, output_dir):
    """
    Creates a master image from the character images and saves it.

    :param char_images: Dictionary of character images with their dimensions.
    :param max_width: Maximum character width.
    :param max_height: Maximum character height.
    :param font_name: The font name used for directory structure and naming.
    :param font_variant: The font variant used for directory structure and naming.
    :return: Path to the saved master image.
    """
    num_cols, num_rows = 16, 6
    master_width = num_cols * max_width
    master_height = num_rows * max_height

    # Create the master image (black background)
    master_img = Image.new("L", (master_width, master_height), color=0)  # Black background

    # Paste each character image into the master image
    for idx, (char_code, (img, width, height)) in enumerate(char_images.items()):
        row, col = divmod(idx, num_cols)
        x_offset = col * max_width
        y_offset = row * max_height

        # Crop the character image from the origin (0,0) to max_width and max_height
        cropped_img = img.crop((0, 0, width, height))
        
        # Create a blank image of max dimensions and paste the cropped character in the top-left corner
        final_img = Image.new("L", (max_width, max_height), color=0)
        final_img.paste(cropped_img, (0, 0))

        # Paste the final character image into the master image
        master_img.paste(final_img, (x_offset, y_offset))

    # Create directory for saving
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Save the master image
    master_img_path = os.path.join(output_dir, 'master.png')
    master_img.save(master_img_path)
    print(f"Master image saved as {master_img_path}")

    return master_img

def generate_metadata_file(max_width, max_height, point_size, metadata_dir):
    """
    Generates a metadata file (data.txt) with information about the maximum character dimensions
    and the point size used for rendering the master image.
    
    :param font_name: The font name used for directory structure and naming.
    :param font_variant: The font variant used for directory structure and naming.
    :param max_width: Maximum character width.
    :param max_height: Maximum character height.
    :param point_size: The point size used for creating the master image.
    """
    metadata_path = os.path.join(metadata_dir, 'data.txt')
    
    with open(metadata_path, 'w') as f:
        f.write(f"Max width: {max_width}\n")
        f.write(f"Max height: {max_height}\n")
        f.write(f"Point size: {point_size}\n")

    print(f"Metadata file saved as {metadata_path}")
    return metadata_path

def create_single_char_image(char, font_path, point_size, metadata_filepath):
    # Read metadata to get the max width, max height, and point size
    max_width_master, max_height_master, point_size = read_metadata(metadata_filepath)

    # Compute the ratio between the target point size and the point size
    point_size_ratio = point_size / point_size

    # Compute the dimensions of the image based on this ratio
    target_width = int(max_width_master * point_size_ratio)
    target_height = int(max_height_master * point_size_ratio)

    # Load the font at the given point size
    font = ImageFont.truetype(font_path, point_size)

    # Create an image with the computed dimensions
    char_img = Image.new("L", (target_width, target_height), color=0)  # Black background
    draw = ImageDraw.Draw(char_img)

    # Render the character onto the image
    draw.text((0, 0), char, font=font, fill=255)  # White character

    # Quantize the image to the specified 4-level grayscale palette
    quantized_img = quantize_to_palette(char_img)

    return quantized_img

def create_scaled_font(font_path, target_width, target_height, font_name, font_variant, point_size, metadata_filepath, scaled_output_dir):
    """
    Creates a scaled font by rendering at a calculated point size, scaling it to fit the target width and height,
    and maintaining aspect ratio without cropping. All characters are uniformly scaled.
    
    :param font_path: Path to the .ttf font file.
    :param target_width: Target width in pixels for each character.
    :param target_height: Target height in pixels for each character.
    :param font_name: The font name used for directory structure and naming.
    :param font_variant: The font variant used for directory structure and naming.
    :param point_size: The point size used for creating the master font.
    :return: Path to the saved scaled font image.
    """
    # Read the master font metadata (max dimensions in pixels)
    max_width_master, max_height_master, _ = read_metadata(metadata_filepath)

    # Determine the scaling factors based on both axes
    width_ratio = target_width / max_width_master
    height_ratio = target_height / max_height_master

    print(f"Width, height ratios: {width_ratio}, {height_ratio}")

    # Use the larger scaling factor to ensure the character fits within the target dimensions
    scaling_factor = max(width_ratio, height_ratio)

    # Calculate the point size for rendering based on the scaling factor
    target_point_size = int(point_size * scaling_factor)

    # Precompute the scaled width and height for all characters
    scaled_width = int(max_width_master * scaling_factor)
    scaled_height = int(max_height_master * scaling_factor)

    print(f"Scaled width, height: {scaled_width}, {scaled_height}")

    # Create a blank image for the final composite
    num_cols, num_rows = 16, 6  # Grid size for ASCII 32 to 127
    scaled_img = Image.new("L", (num_cols * target_width, num_rows * target_height), color=0)

    # Loop through characters and paste each one into the composite image
    for idx, char_code in enumerate(range(32, 256)):
        row, col = divmod(idx, num_cols)
        x_offset = col * target_width
        y_offset = row * target_height

        img = create_single_char_image(chr(char_code), font_path, target_point_size, metadata_filepath)
        
        # Resize the character image to the precomputed uniform size (scaled_width, scaled_height)
        scaled_char = img.resize((target_width, target_height), Image.BICUBIC)

        # Paste the scaled character into the final image, top-left aligned (no centering)
        scaled_img.paste(scaled_char, (x_offset, y_offset))

    # Create the output directory
    if not os.path.exists(scaled_output_dir):
        os.makedirs(scaled_output_dir)

    # Save the scaled font image
    scaled_img_path = os.path.join(scaled_output_dir, f'{font_name}_{font_variant}_{target_width}x{target_height}.png')
    scaled_img.save(scaled_img_path)
    print(f"Scaled font saved as {scaled_img_path}")

    # Save metadata
    scaled_metadata_filepath = os.path.join(scaled_output_dir, 'data.txt')
    save_scaled_metadata(scaled_metadata_filepath, target_width, target_height)

    return scaled_img_path

def render_font_at_point_size(font_path, point_size, threshold, char_range=(32, 127)):
    """
    Renders each character at the specified point size and returns a dictionary
    with character images and their dimensions.
    
    :param font_path: Path to the .ttf font file.
    :param point_size: Point size to use for rendering the characters.
    :param char_range: Range of characters to render (default is from ASCII 32 to 127).
    :return: A dictionary with character images and their dimensions.
    """
    font = ImageFont.truetype(font_path, point_size)
    
    char_images = {}
    max_width, max_height = 0, 0

    for char_code in range(char_range[0], char_range[1] + 1):
        char = chr(char_code)
        temp_img = Image.new("L", (256, 256), color=0)
        draw = ImageDraw.Draw(temp_img)
        draw.text((0, 0), char, font=font, fill=255)

        temp_img = quantize_to_palette(temp_img, palette=(0, threshold, 255))

        bbox = temp_img.getbbox()

        if bbox:
            width = bbox[2]  # Lower right corner x-coordinate gives the width from the origin
            height = bbox[3]  # Lower right corner y-coordinate gives the height from the origin
            max_width = max(max_width, width)
            max_height = max(max_height, height)
            char_images[char_code] = (temp_img, width, height)

    return char_images, max_width, max_height

def read_metadata(metadata_filepath):
    """
    Reads the metadata file to extract the max width and height of the characters and the point size.
    
    :param metadata_filepath: Path to the metadata file.
    :return: max_width, max_height, and point_size as integers.
    """
    max_width = max_height = point_size = 0
    with open(metadata_filepath, 'r') as f:
        for line in f:
            if "Max width" in line:
                max_width = int(line.split(": ")[1].strip())
            elif "Max height" in line:
                max_height = int(line.split(": ")[1].strip())
            elif "Point size" in line:
                point_size = int(line.split(": ")[1].strip())
    return max_width, max_height, point_size


def save_scaled_metadata(filepath, target_width, target_height):
    """
    Saves metadata about the scaled font.
    
    :param filepath: Path to save the metadata file.
    :param target_width: Target width in pixels for each character.
    :param target_height: Target height in pixels for each character.
    """
    with open(filepath, 'w') as f:
        f.write(f"Target width: {target_width}\n")
        f.write(f"Target height: {target_height}\n")
    print(f"Metadata saved as {filepath}")


if __name__ == '__main__':
    # Define parameters for creating the master font
    point_size = 70  # Point size for the font rendering
    threshold = 128  # Threshold for binarizing the image
    font_name = '8_bit_fortress'
    font_variant = 'Regular'

    sources_dir = 'src/assets/ttf'
    font_path = f'{sources_dir}/{font_name}/{font_name}.ttf'
    output_dir = f'{sources_dir}/{font_name}/{font_variant}'
    metadata_dir = f'{sources_dir}/{font_name}/{font_variant}'
    metadata_filepath = f'{sources_dir}/{font_name}/{font_variant}/data.txt'

    if True:
        # Extract bitmap glyphs from the font
        char_images = extract_bitmap_glyphs(font_path)
        if char_images:
            max_width, max_height = 0, 0
            for char_name, char_img in char_images:
                max_width = max(max_width, char_img.width)
                max_height = max(max_height, char_img.height)
        else:
            # Loop through a range of point sizes until finding one that generates images with no anti-aliasing
            for point_size in range(6, 73):
                char_images, max_width, max_height = render_and_measure_characters(font_path, point_size, threshold)
                anti_aliased = False
                for char_code, (image, width, height) in char_images.items():
                    if has_gray_pixels(image):
                        anti_aliased = True
                        break
                if not anti_aliased:
                    break

        # Create and save the master image
        master_img = create_master_image(char_images, max_width, max_height, output_dir)
        master_img = white_to_black(master_img)
        master_img.show()

        # Generate metadata file with max width, height, and point_size
        metadata_path = generate_metadata_file(max_width, max_height, point_size, metadata_dir)

    if False:
        # List of character resolutions to process
        character_resolutions = [
            (12, 16), (16, 16), (16, 20), (16, 24), (24, 24), (24, 32), (32, 32), (32, 40),
        ]

        # Loop through the character resolutions and scale the master font
        for target_width, target_height in character_resolutions:
            # Create the scaled font using the master font's point size as reference
            scaled_output_dir = f'src/assets/img/proc/fonts/{font_name}/{font_variant}/{target_width}x{target_height}'

            create_scaled_font(font_path, target_width, target_height, font_name, font_variant, point_size, metadata_filepath, scaled_output_dir)

    img = Image.open('src/assets/ttf/8_bit_fortress/Regular/master.png')
    # img = quantize_to_palette(img, palette=(0, 255-20, 255))
    print(has_gray_pixels(img))
    img = white_to_black(img)
    img.show()
    hist = grayscale_histogram(img)
    plt.plot(hist)
    plt.title("Grayscale Histogram")
    plt.xlabel("Pixel Intensity")
    plt.ylabel("Frequency")
    plt.show()