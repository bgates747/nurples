import gzip
import struct
import os
from PIL import Image
from make_agon_font import binary_to_image, save_binary_from_image, build_master_image, find_max_font_dimensions

# PSF1 and PSF2 magic numbers
PSF1_MAGIC = b'\x36\x04'
PSF2_MAGIC = b'\x72\xb5\x4a\x86'
PSF1_MODE512 = 1

def read_psf(file_path):
    """Detects PSF version and reads the PSF font file."""
    with open(file_path, 'rb') as f:
        # Read the first 4 bytes to check for the magic number
        magic = f.read(4)
        
        if magic[:2] == PSF1_MAGIC:
            return read_psf1(file_path)
        elif magic == PSF2_MAGIC:
            return read_psf2(file_path)
        else:
            raise ValueError(f"Not a valid PSF1 or PSF2 file: {file_path}")

def read_psf1(file_path):
    """Reads a PSF1 font file and returns glyph bitmaps and metadata."""
    with open(file_path, 'rb') as f:
        # Read the header (4 bytes)
        magic, mode, char_size = struct.unpack('2sBB', f.read(4))
        
        # Verify the magic number
        if magic != PSF1_MAGIC:
            raise ValueError(f"Not a valid PSF1 file: {file_path}")
        
        # Determine number of glyphs (256 or 512)
        num_glyphs = 512 if mode & PSF1_MODE512 else 256
        
        # Read each glyph bitmap (char_size bytes per glyph)
        glyphs = []
        for _ in range(num_glyphs):
            glyph_data = f.read(char_size)
            glyphs.append(glyph_data)

        return {
            'glyphs': glyphs,
            'num_glyphs': num_glyphs,
            'char_size': char_size,
            'height': char_size,
            'width': 8  # PSF1 fonts are typically 8 pixels wide
        }

def read_psf2(file_path):
    """Reads a PSF2 font file and returns glyph bitmaps and metadata."""
    with open(file_path, 'rb') as f:
        # Read the header (32 bytes for PSF2)
        header = f.read(32)
        magic, version, header_size, flags, num_glyphs, glyph_size, height, width = struct.unpack('Iiiiiiii', header[:32])
        
        # Verify the magic number
        if magic != 0x864ab572:
            raise ValueError(f"Not a valid PSF2 file: {file_path}")
        
        # Read each glyph bitmap
        glyphs = []
        for _ in range(num_glyphs):
            glyph_data = f.read(glyph_size)
            glyphs.append(glyph_data)

        # Check for Unicode table if present
        unicode_table = {}
        if flags & 0x01:  # PSF2 has a Unicode table
            while True:
                byte = f.read(1)
                if not byte or byte == b'\xFF':  # End of the Unicode table
                    break
                glyph_index = struct.unpack('B', byte)[0]
                unicodes = []
                while True:
                    codepoint_data = f.read(2)
                    if len(codepoint_data) < 2:
                        break  # End of table or invalid code point
                    codepoint = struct.unpack('H', codepoint_data)[0]  # Unicode code point
                    if codepoint == 0xFFFF:  # End of list for this glyph
                        break
                    unicodes.append(codepoint)
                unicode_table[glyph_index] = unicodes

        return {
            'glyphs': glyphs,
            'num_glyphs': num_glyphs,
            'glyph_size': glyph_size,
            'height': height,
            'width': width,
            'unicode_table': unicode_table
        }

def decompress_gz(gz_file_path, decompressed_file_path):
    """Decompresses a .gz file and returns the decompressed file path."""
    with gzip.open(gz_file_path, 'rb') as gz_file:
        with open(decompressed_file_path, 'wb') as out_file:
            out_file.write(gz_file.read())
    return decompressed_file_path

def extract_bitmasks(psf_data):
    """Extract bitmasks for characters 0-255 from the PSF font data."""
    bitmasks = []
    for char_code in range(256):  # ASCII codes 0-255
        # Determine glyph index for PSF2 (with Unicode table) or use the character code directly for PSF1
        if 'unicode_table' in psf_data and psf_data['unicode_table']:
            glyph_index = None
            for index, codes in psf_data['unicode_table'].items():
                if char_code in codes:
                    glyph_index = index
                    break
            if glyph_index is None:
                glyph_index = 0  # Fallback to first glyph if not found
        else:
            # For PSF1 or if no Unicode table is present
            glyph_index = char_code if char_code < psf_data['num_glyphs'] else 0

        # Append the glyph's bitmask (bitmask data for the glyph)
        bitmasks.append(psf_data['glyphs'][glyph_index])
    
    return bitmasks

def process_psf_files(src_dir):
    # Step 1: Find and process all .psf.gz files (first pass)
    font_base_names = [file.replace('.psf.gz', '') for file in os.listdir(src_dir) if file.endswith('.psf.gz')]
    font_base_names.sort()

    # Process each .psf.gz file and extract its contents
    for font_base_name in font_base_names:
        print(f"Processing font archive: {font_base_name}")
        
        # Construct file paths
        psf_gz_file = os.path.join(src_dir, f"{font_base_name}.psf.gz")
        output_dir = os.path.join(src_dir, font_base_name)
        decompressed_psf_file = os.path.join(output_dir, f"{font_base_name}.psf")

        # Create the output directory if it doesn't exist
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        # Decompress the .psf.gz file into the output directory
        decompress_gz(psf_gz_file, decompressed_psf_file)

        # Delete the original .psf.gz file after decompression
        os.remove(psf_gz_file)
        print(f"Decompressed {font_base_name}.psf.gz and removed the original archive.")

    # Step 2: Process each decompressed .psf file in the subdirectories (second pass)
    for subdir in os.listdir(src_dir):
        subdir_path = os.path.join(src_dir, subdir)

        # Check if the subdir is actually a directory
        if os.path.isdir(subdir_path):
            # Look for the .psf file inside this subdirectory
            psf_files = [file for file in os.listdir(subdir_path) if file.endswith('.psf')]

            if psf_files:
                # Assuming only one .psf file per directory
                psf_file = psf_files[0]
                psf_file_path = os.path.join(subdir_path, psf_file)
                print(f"Processing font: {psf_file_path}")

                output_font_binary = os.path.join(subdir_path, f"{subdir}.data")
                output_image = os.path.join(src_dir, f"{subdir}.png")  # Master image in the root directory

                # Read the PSF font file (automatically detects PSF1 or PSF2)
                psf_data = read_psf(psf_file_path)

                # For each character (0-255), create an individual PNG file
                for char_code in range(256):
                    glyph_data = psf_data['glyphs'][char_code]
                    char_png_file = os.path.join(subdir_path, f"{char_code:03d}.png")
                    
                    # Create a monochrome image for the glyph
                    glyph_img = Image.new('1', (psf_data['width'], psf_data['height']), color=1)  # White background
                    for y in range(psf_data['height']):
                        byte = glyph_data[y] if y < len(glyph_data) else 0
                        for bit in range(psf_data['width']):
                            if byte & (0x80 >> bit):  # Set black pixel where bit is set
                                glyph_img.putpixel((bit, y), 0)  # Black pixel
                    
                    # Save the individual glyph as a .png
                    glyph_img.save(char_png_file)

                # Find the maximum width and height of the character images
                max_width, max_height = find_max_font_dimensions(subdir_path)
                print(f"Max width: {max_width}, max height: {max_height}")

                # Set target width and height for characters
                target_width, target_height = max_width, max_height
                
                # Build the master image
                master_img = build_master_image(subdir_path, target_width, target_height)

                # Save the master image in the root directory
                master_img.save(output_image)
                print(f"Master image saved to {output_image}")

                # Convert the master image to binary and save it to a file
                save_binary_from_image(master_img, output_font_binary, target_width, target_height)

                # Convert the binary file back to an image (for validation)
                img_width = 16 * target_width
                img_height = 16 * target_height
                binary_to_image(output_font_binary, output_image, img_width, img_height)

    # Step 3: Clean up individual character .png files
    for subdir in os.listdir(src_dir):
        subdir_path = os.path.join(src_dir, subdir)

        # Check if it's actually a directory
        if os.path.isdir(subdir_path):
            # Find all .png files with filenames consisting only of numbers
            for file_name in os.listdir(subdir_path):
                if file_name.endswith('.png') and file_name[:-4].isdigit():
                    file_path = os.path.join(subdir_path, file_name)
                    try:
                        os.remove(file_path)
                    except Exception as e:
                        print(f"Failed to delete {file_path}: {e}")

if __name__ == "__main__":
    # Define the base source directory
    src_dir = "src/assets/cfonts"
    
    # Process all PSF files and generate outputs
    process_psf_files(src_dir)