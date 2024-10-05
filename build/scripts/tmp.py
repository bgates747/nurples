import zipfile
from fontTools.ttLib import TTFont
import os

def unzip_files_in_downloads():
    """
    Unzips all .zip files in /home/smith/Downloads into a new folder 
    with the same name as the base zip file name. Then, renames the .ttf file 
    found inside the folder to match the base zip file name.
    """
    downloads_dir = '/home/smith/Downloads'

    # Iterate over all files in the downloads directory
    for filename in os.listdir(downloads_dir):
        if filename.endswith('.zip'):
            # Define the full path to the zip file
            zip_filepath = os.path.join(downloads_dir, filename)

            # Define the target extraction folder based on the base filename
            base_name = os.path.splitext(filename)[0]
            extract_folder = os.path.join(downloads_dir, base_name)

            # Create the directory if it doesn't exist
            os.makedirs(extract_folder, exist_ok=True)

            # Unzip the file into the new folder
            with zipfile.ZipFile(zip_filepath, 'r') as zip_ref:
                zip_ref.extractall(extract_folder)
                print(f'Unzipped {filename} to {extract_folder}')

            # Find the .ttf file in the extracted folder
            ttf_file = None
            for root, dirs, files in os.walk(extract_folder):
                for file in files:
                    if file.endswith('.ttf'):
                        ttf_file = os.path.join(root, file)
                        break
                if ttf_file:
                    break

            # If a .ttf file is found, rename it to match the base .zip filename
            if ttf_file:
                ttf_new_name = os.path.join(extract_folder, f"{base_name}.ttf")
                os.rename(ttf_file, ttf_new_name)
                print(f'Renamed {ttf_file} to {ttf_new_name}')

def extract_ttf_metadata(ttf_path):
    """
    Extracts metadata from a .ttf file and dumps it into a metadata file in the same directory.
    
    :param ttf_path: Path to the .ttf file.
    """
    # Load the TTF file
    font = TTFont(ttf_path)
    
    # Define the metadata file path
    metadata_filepath = os.path.splitext(ttf_path)[0] + '_metadata.txt'
    
    # Extract metadata
    metadata = {}
    
    # Get font name table
    name_records = font['name'].names
    for record in name_records:
        try:
            # Some records are in bytes, so decode them if needed
            name = record.toUnicode()
            metadata[record.nameID] = name
        except UnicodeDecodeError:
            pass

    # Dump the metadata to a file with UTF-8 encoding
    with open(metadata_filepath, 'w', encoding='utf-8') as f:
        f.write("TTF Metadata:\n")
        f.write("================\n")
        for nameID, name in metadata.items():
            f.write(f"Name ID {nameID}: {name}\n")

    print(f"Metadata saved as {metadata_filepath}")

if __name__ == "__main__":
    # Run the function
    # unzip_files_in_downloads()

    font_name = 'amiga_forever'
    ttf_path = f'src/assets/ttf/{font_name}/{font_name}.ttf'
    extract_ttf_metadata(ttf_path)