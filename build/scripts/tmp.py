import os

# Path to start the search
root_path = os.path.expanduser("~/Agon")

# Search criteria
file_name = "fixed168.inc"
search_line = "    ld hl,64*256"
replace_line = "    ld hl,32*256"

# List to store edited files
edited_files = []

def replace_in_file(file_path):
    """Replaces the target line in the specified file."""
    try:
        with open(file_path, 'r') as file:
            lines = file.readlines()

        updated_lines = []
        modified = False
        for line in lines:
            if line.strip() == search_line.strip():
                updated_lines.append(replace_line + "\n")
                modified = True
            else:
                updated_lines.append(line)

        if modified:
            with open(file_path, 'w') as file:
                file.writelines(updated_lines)
            edited_files.append(file_path)
    except Exception as e:
        print(f"Error processing {file_path}: {e}")

# Walk through the directory
for dirpath, _, filenames in os.walk(root_path):
    for fname in filenames:
        if fname == file_name:
            full_path = os.path.join(dirpath, fname)
            replace_in_file(full_path)

# Output the list of edited files
print("Edited files:")
for file in edited_files:
    print(file)

if not edited_files:
    print("No files were edited.")
