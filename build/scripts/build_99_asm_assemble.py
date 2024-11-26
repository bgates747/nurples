import subprocess
import sys
import os
import shutil
import argparse

def run_ez80asm():
    try:
        # Store the current working directory
        original_dir = os.getcwd()

        # Define the directory containing the .asm file
        asm_directory = "src/asm"
        asm_file = "nurples.asm"
        output_file = "../../tgt/nurples.bin"

        # Change working directory to the directory containing the .asm file
        os.chdir(asm_directory)

        # Define the command with relative paths from the new working directory
        command = [
            "ez80asm",
            "-l",              # Option to generate listing file
            asm_file,          # Input assembly file
            output_file        # Output binary file (relative path from the new directory)
        ]

        # Execute the command
        result = subprocess.run(command, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        
        # Print the output
        print(result.stdout.decode())
        
    except subprocess.CalledProcessError as e:
        # Print the error message if the command fails
        print(f"Command failed with error: {e.stderr.decode()}")
        sys.exit(1)
    except Exception as e:
        # Handle other exceptions (e.g., if directory change fails)
        print(f"An error occurred: {e}")
        sys.exit(1)
    finally:
        # Restore the original working directory
        os.chdir(original_dir)

def copy_to_sdcard():
    try:
        # Define the source and target directories
        src_directory = "tgt"
        tgt_directory = "/media/smith/AGON/mystuff/nurples/tgt"

        # Check if the target directory exists, if so, delete it recursively
        if os.path.exists(tgt_directory):
            shutil.rmtree(tgt_directory)  # Remove the target directory and its contents

        # Recreate the target directory
        os.makedirs(tgt_directory)

        # Copy everything from the source directory to the target directory
        shutil.copytree(src_directory, tgt_directory, dirs_exist_ok=True)

        print(f"Successfully copied files from {src_directory} to {tgt_directory}")

    except PermissionError:
        print("Failed to copy files: SD card is likely not mounted.")
    except Exception as e:
        print(f"An unexpected error occurred while copying files: {e}")

def run_fab_agon_emulator():
    command = ["agon-emulator"]
    process = subprocess.Popen(command, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    process.communicate()

def main():
    # Create an argument parser
    parser = argparse.ArgumentParser(description="Run assembly, copy, and emulator commands.")
    
    # Add optional switches for each function
    parser.add_argument('-a', action='store_true', help="Run ez80asm assembly")
    parser.add_argument('-e', action='store_true', help="Run fab-agon-emulator")
    parser.add_argument('-sd', action='store_true', help="Copy files to SD card")

    # Parse the arguments
    args = parser.parse_args()

    # If no options are given, run all functions
    if not args.a and not args.e and not args.sd:
        run_ez80asm()
        copy_to_sdcard()
        run_fab_agon_emulator()
    else:
        if args.a:
            run_ez80asm()
        if args.sd:
            copy_to_sdcard()
        if args.e:
            run_fab_agon_emulator()

if __name__ == "__main__":
    main()