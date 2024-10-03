import os
from PIL import Image
import math

def binarize_image(image, threshold=128):
    """Converts an image to grayscale and binarizes it based on a threshold."""
    gray_image = image.convert("L")
    return gray_image.point(lambda p: 255 if p > threshold else 0)

def find_max_font_dimensions(directory):
    """Scans the directory for character .png files and returns the max width and height of the images."""
    max_width = 0
    max_height = 0
    
    for i in range(256):  # ASCII codes 0-255
        filename = f"{i:03d}.png"  # Character file format ddd.png
        filepath = os.path.join(directory, filename)
        
        if os.path.exists(filepath):
            with Image.open(filepath) as img:
                img_width, img_height = img.size
                max_width = max(max_width, img_width)
                max_height = max(max_height, img_height)

    # Round the width up to the nearest multiple of 8
    max_width = math.ceil(max_width / 8) * 8
    return max_width, max_height

def build_master_image(directory, target_width, target_height, chars_per_row=16):
    """Creates a master image by pasting each character image into the correct location."""
    total_chars = 256
    rows = total_chars // chars_per_row
    img_width = chars_per_row * target_width
    img_height = rows * target_height

    # Create a blank grayscale image to hold all the characters
    master_img = Image.new('L', (img_width, img_height), 255)  # Start with white background

    for i in range(256):  # ASCII codes 0-255
        filename = f"{i:03d}.png"
        filepath = os.path.join(directory, filename)
        
        if os.path.exists(filepath):
            # print(f"Processing character {i:03d} from {filename}...")
            with Image.open(filepath) as img:
                # Binarize the image
                binarized_img = binarize_image(img)
                
                # Calculate position of the character in the master image
                char_x = (i % chars_per_row) * target_width
                char_y = (i // chars_per_row) * target_height

                # Paste the binarized image into the correct location
                master_img.paste(binarized_img, (char_x, char_y))
        else:
            print(f"Character {i:03d} not found, leaving blank.")

    return master_img

def save_binary_from_image(image, output_file, target_width, target_height):
    """Converts a PIL image to binary data and saves it to a file."""
    binary_data = []
    for y in range(image.height):
        row_data = 0
        for x in range(image.width):
            pixel = image.getpixel((x, y))
            if pixel == 0:  # Foreground pixel (black)
                row_data |= (1 << (7 - (x % 8)))  # Set bit in byte
            if (x + 1) % 8 == 0 or x == image.width - 1:
                binary_data.append(row_data)
                row_data = 0  # Reset row_data for next byte

    # Save binary data to output file
    with open(output_file, "wb") as bin_file:
        bin_file.write(bytearray(binary_data))
    print(f"Binary data saved to {output_file}, total size: {len(binary_data)} bytes.")


def binary_to_image(binary_file, output_png, img_width, img_height):
    """Converts the binary font file directly to a .png image."""
    try:
        # Read the binary file
        with open(binary_file, "rb") as bin_file:
            binary_data = bin_file.read()

        # Create a new monochrome image (1-bit pixels)
        img = Image.frombytes('1', (img_width, img_height), binary_data)

        # Save the resulting image
        img.save(output_png)
        # print(f"Image saved to {output_png}")

    except Exception as e:
        print(f"Error reading or processing {binary_file}: {e}")

if __name__ == "__main__":
    # Example usage
    directory = "src/assets/img/orig/fonts/rc"  # Replace with the actual directory
    output_file = f"{directory}/rc.data"  # Replace with the desired output file
    output_image = f"{directory}/rc.master.png"  # Output master image file

    # Find the maximum width and height of the character images
    max_width, max_height = find_max_font_dimensions(directory)
    # print(f"Max width: {max_width}, max height: {max_height}")

    # Set target width and height for characters
    target_width, target_height = 24, 32

    # Build the master image
    master_img = build_master_image(directory, target_width, target_height)

    # Save the master image
    master_img.save(output_image)
    # print(f"Master image saved to {output_image}")

    # Convert the master image to binary and save it to a file
    save_binary_from_image(master_img, output_file, target_width, target_height)

    # Convert the binary file back to an image
    img_width = 16 * target_width
    img_height = 16 * target_height
    binary_to_image(output_file, f"{directory}/rc.reconstructed.png", img_width, img_height)
