def hex_dump_comparison(file1, file2, output_file):
    def read_file(filepath):
        with open(filepath, 'rb') as f:
            return f.read()

    data1 = read_file(file1)
    data2 = read_file(file2)
    
    max_length = max(len(data1), len(data2))
    
    with open(output_file, 'w') as out:
        # Write file header with filenames
        out.write(f"File 1: {file1}\n")
        out.write(f"File 2: {file2}\n\n")
        out.write(f"{'Address':<10} {'File 1':<49} {'File 2':<49}\n")
        out.write(f"{'':<10} {' '.join(f'{i:02X}' for i in range(16)):<49} {' '.join(f'{i:02X}' for i in range(16)):<49}\n")
        out.write('-' * 108 + '\n')
        
        # Generate hex dump rows
        for addr in range(0, max_length, 16):
            chunk1 = data1[addr:addr+16]
            chunk2 = data2[addr:addr+16]
            
            hex_chunk1 = ' '.join(f'{b:02X}' for b in chunk1).ljust(48)
            hex_chunk2 = ' '.join(f'{b:02X}' for b in chunk2).ljust(48)
            
            out.write(f"{addr:04X}:    {hex_chunk1}    {hex_chunk2}\n")

    print(f"Hex comparison saved to {output_file}")

if __name__ == "__main__":
    file1 = "programmatic.map"  # Replace with your file path
    file2 = "app.map"           # Replace with your file path
    output_file = "hex_comparison.txt"  # File to save the output
    hex_dump_comparison(file1, file2, output_file)
