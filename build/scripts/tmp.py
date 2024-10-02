
# bash
# for file in src/assets/img/orig/fonts/bad/*; do basename=$(basename "$file"); rm -f "src/assets/img/orig/fonts/consolefonts/$basename"; done


# import os
# import re

# # Define the path to the parent directory
# consolefonts_dir = "src/assets/img/orig/fonts/consolefonts"

# # Regular expression to match .png files starting with three numbers
# pattern = re.compile(r'^\d{3}.*\.png$')

# # Loop through all directories in the consolefonts directory (non-recursive)
# for dir_name in os.listdir(consolefonts_dir):
#     dir_path = os.path.join(consolefonts_dir, dir_name)
    
#     if os.path.isdir(dir_path):  # Only process directories
#         print(f"Processing directory: {dir_path}")
        
#         # Loop through all files in the directory
#         for file_name in os.listdir(dir_path):
#             # Check if the file is a .png file starting with three numbers
#             if pattern.match(file_name):
#                 file_path = os.path.join(dir_path, file_name)
#                 print(f"Deleting file: {file_path}")
#                 os.remove(file_path)

# print("Cleanup complete.")


import os
import shutil

# Define the paths for the bad and consolefonts directories
bad_dir = "src/assets/img/orig/fonts/bad"
consolefonts_dir = "src/assets/img/orig/fonts/consolefonts"

# Loop through all .png files in the bad directory
for file_name in os.listdir(bad_dir):
    if file_name.endswith(".png"):
        # Get the base name of the .png file (without extension)
        base_name = os.path.splitext(file_name)[0]
        
        # Check if a corresponding directory exists in consolefonts
        corresponding_dir = os.path.join(consolefonts_dir, base_name)
        
        if os.path.isdir(corresponding_dir):  # Only proceed if it's a directory
            destination = os.path.join(bad_dir, base_name)
            
            # Move the entire directory from consolefonts to bad
            print(f"Moving {corresponding_dir} to {destination}")
            shutil.move(corresponding_dir, destination)

print("Move operation complete.")
