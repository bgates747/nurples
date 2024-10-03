import struct

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
            'height': char_size,  # In PSF1, char_size is also the height
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

# Replace with your local path to the PSF file
psf_file_path = "/home/smith/Desktop/bad/Lat2-Terminus18x10/Lat2-Terminus18x10.psf"

psf_data = read_psf(psf_file_path)

# Print the extracted information based on the PSF version
if 'char_size' in psf_data:
    print(f"Number of glyphs: {psf_data['num_glyphs']}")
    print(f"Character size (PSF1): {psf_data['char_size']}")
elif 'glyph_size' in psf_data:
    print(f"Number of glyphs: {psf_data['num_glyphs']}")
    print(f"Glyph size (PSF2): {psf_data['glyph_size']}, Width: {psf_data['width']}, Height: {psf_data['height']}")

# Print the first glyph data for inspection
print(f"First glyph data: {psf_data['glyphs'][0]}")
