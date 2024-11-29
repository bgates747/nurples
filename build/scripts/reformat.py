import os

def reformat_line(line):
    """Reformats a single line while preserving comments and keeping line-ending spaces."""
    # Split the line into code and comment parts
    if ';' in line:
        code, comment = line.split(';', 1)
        comment = ';' + comment  # Keep the semicolon and comment part
    else:
        code, comment = line, ''
    
    # Replace tabs with a single space
    code = code.replace('\t', ' ')
    # Replace double spaces with a single space until no double spaces remain
    while '  ' in code:
        code = code.replace('  ', ' ')
    # Add three spaces to lines beginning with a single space
    if code.startswith(' '):
        code = '   ' + code

    # Combine reformatted code and untouched comment
    return code + comment

def reformat_file(file_path):
    """Reformats the given assembly file according to the specified rules."""
    with open(file_path, 'r') as file:
        lines = file.readlines()
    
    # Process each line
    formatted_lines = [reformat_line(line.rstrip('\n')) for line in lines]
    
    # Write the formatted lines back to the file with single newline separators
    with open(file_path, 'w', newline='\n') as file:
        file.write('\n'.join(formatted_lines) + '\n')
    print(f"Reformatted: {file_path}")

def reformat_asm_files(directory):
    """Finds and reformats all .asm and .inc files in the specified directory."""
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(('.asm', '.inc')):
                file_path = os.path.join(root, file)
                reformat_file(file_path)

if __name__ == "__main__":
    src_directory = 'src/asm'  # Replace with your directory path if different
    reformat_asm_files(src_directory)
