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

def quantize_image(img, palette=(0, 85, 170, 255)):
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

def render_and_measure_characters(font_path, point_size, char_range=(32, 127)):
    """
    Renders each character once, measures its bounding box, and returns a dictionary
    with character images (keyed by ASCII code). Also computes the max width and height.

    :param font_path: Path to the .ttf font file.
    :param point_size: Point size to use for rendering the characters.
    :param char_range: Range of characters to render (default is from ASCII 32 to 127).
    :return: A dictionary of character images keyed by ASCII code, and the max width and height.
    """
    char_images = {}
    max_width, max_height = 0, 0  # To track the maximum dimensions of the characters
    font = ImageFont.truetype(font_path, point_size)

    # Loop through each character in the specified range
    for char_code in range(char_range[0], char_range[1] + 1):
        char = chr(char_code)

        # Create an image to render the character (black background, white text)
        char_img = Image.new("L", (256, 256), color=0)  # Black background

        # Use PIL's ImageDraw and ImageFont to render the character
        draw = ImageDraw.Draw(char_img)
        draw.text((0, 0), char, font=font, fill=255)  # White character

        # Find the character's bounding box
        bbox = char_img.getbbox()

        if bbox:
            if char_is_defined(char_img, bbox):
                width = bbox[2]  # Lower right corner x-coordinate gives the width from the origin
                height = bbox[3]  # Lower right corner y-coordinate gives the height from the origin

                # Track the maximum width and height across all characters
                max_width = max(max_width, width)
                max_height = max(max_height, height)

            # Store the character image keyed by its ASCII code
            char_images[char_code] = char_img

    return char_images, max_width, max_height


def create_master_image(char_images, max_width, max_height, char_range=(32, 127)):
    """
    Creates a master image from the character images, iterating through the specified character range.
    If a character image is missing from the char_images dictionary, it uses a blank (black) image.

    :param char_images: Dictionary of character images keyed by ASCII code.
    :param max_width: Maximum character width.
    :param max_height: Maximum character height.
    :param char_range: Range of characters to render (default is from ASCII 32 to 127).
    :return: The master image.
    """
    num_cols, num_rows = 16, 6
    master_width = num_cols * max_width
    master_height = num_rows * max_height

    # Create the master image (black background)
    master_img = Image.new("L", (master_width, master_height), color=0)  # Black background

    # Iterate through the specified character range
    for idx, char_code in enumerate(range(char_range[0], char_range[1] + 1)):
        row, col = divmod(idx, num_cols)
        x_offset = col * max_width
        y_offset = row * max_height

        # Get the character image from the dictionary, or use a blank black image if not found
        img = char_images.get(char_code, Image.new("L", (max_width, max_height), color=0))

        # Find the actual width and height of the character image (or assume max if blank)
        width, height = img.size

        # Crop the character image from the origin (0,0) to max_width and max_height
        cropped_img = img.crop((0, 0, width, height))
        
        # Create a blank image of max dimensions and paste the cropped character in the top-left corner
        final_img = Image.new("L", (max_width, max_height), color=0)
        final_img.paste(cropped_img, (0, 0))

        # Paste the final character image into the master image
        master_img.paste(final_img, (x_offset, y_offset))

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

def create_scaled_char(char, font_path, point_size, metadata_filepath):
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
    quantized_img = quantize_image(char_img)

    return quantized_img

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

def generate_fonts_by_point_size(font_path, output_dir, metadata_dir, threshold, char_range=(32, 127)):
    """
    Loops through a range of point sizes and creates a font image for each size that passes the anti-aliasing threshold test.
    Saves each image with the format 'fontname_widthxheight.png'.
    
    :param font_path: Path to the .ttf font file.
    :param output_dir: Directory to save the generated font images.
    :param metadata_dir: Directory to save the metadata files.
    :param threshold: Threshold for detecting anti-aliasing in grayscale images.
    :param char_range: Range of characters to render (default is from ASCII 32 to 127).
    """
    # Ensure the output and metadata directories exist
    os.makedirs(output_dir, exist_ok=True)
    os.makedirs(metadata_dir, exist_ok=True)

    # Loop through a range of point sizes and generate images
    for point_size in np.arange(6, 73, 1):
        point_size = round(point_size, 1)
        
        # Render and measure characters at the current point size
        char_images, max_width, max_height = render_and_measure_characters(font_path, point_size, char_range)

        anti_aliased = False

        # Check if the font size passes the anti-aliasing threshold
        for char_code in range(char_range[0], char_range[1] + 1):
            # Get the character image by its ASCII code, fallback to a blank black image if None
            image = char_images.get(char_code, Image.new("L", (max_width, max_height), color=0))  # Black blank image

            # Quantize the image to reduce it to a limited palette
            quant_img = quantize_image(image, palette=(0, threshold, 255))

            # Check for anti-aliasing by searching for gray pixels
            if has_gray_pixels(quant_img):
                anti_aliased = True
                break

        # If no anti-aliasing was found, generate the font image and save it
        if not anti_aliased:
            # Create the master image by assembling character images
            master_img = create_master_image(char_images, max_width, max_height, char_range)

            # Quantize and apply threshold to the master image
            master_img = quantize_image(master_img, palette=(0, threshold, 255))

            # Generate the output file name in the format 'fontname_widthxheight.png'
            font_name = os.path.splitext(os.path.basename(font_path))[0]
            output_file = f'{font_name}_{max_width}x{max_height}.png'

            # Save the master image
            master_img.save(os.path.join(output_dir, output_file))
            print(f'Saved: {output_file}')

            # Generate metadata file with max width, height, and point size
            metadata_path = generate_metadata_file(max_width, max_height, point_size, metadata_dir)
            print(f'Metadata saved: {metadata_path}')


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
    return True


if __name__ == '__main__':
    # Define parameters for creating the master font
    threshold = 255  # Threshold for binarizing the image
    font_name = 'planetary_contact'
    font_variant = 'Regular'

    sources_dir = 'src/assets/ttf'
    font_path = f'{sources_dir}/{font_name}/{font_name}.ttf'
    output_dir = f'{sources_dir}/{font_name}/{font_variant}'
    metadata_dir = f'{sources_dir}/{font_name}/{font_variant}'
    metadata_filepath = f'{sources_dir}/{font_name}/{font_variant}/data.txt'

    # Create directory for saving
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    # Delete existing files
    os.system(f'rm -rf {output_dir}/*')

    if True:
        generate_fonts_by_point_size(font_path, output_dir, metadata_dir, threshold)

    # img = Image.open('src/assets/ttf/8_bit_fortress/Regular/master.png')
    # # img = quantize_image(img, palette=(0, 255-20, 255))
    # print(has_gray_pixels(img))
    # hist = grayscale_histogram(img)
    # plt.plot(hist)
    # plt.title("Grayscale Histogram")
    # plt.xlabel("Pixel Intensity")
    # plt.ylabel("Frequency")
    # plt.show()