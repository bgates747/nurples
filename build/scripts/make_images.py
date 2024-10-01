import time
import os
import shutil
from PIL import Image
import agonutils as au

def crop_images_fixed_size(img, target_width=16, target_height=16):
    """
    Crops the given image to exactly target dimensions pixels if it is larger than that.
    Images smaller than target dimensions are left untouched.
    
    Parameters:
    - img (PIL.Image): The image to be cropped.
    - target_width (int): The target width to crop to.
    - target_height (int): The target height to crop to.
    
    Returns:
    - PIL.Image: The cropped image or original image if cropping is not needed.
    """
    # Get the original image dimensions
    original_width, original_height = img.size
    
    # Check if the image is larger than the target dimensions
    if original_width >= target_width and original_height >= target_height:
        print(f"Cropping image from {original_width}x{original_height} to {target_width}x{target_height}")
        
        # Perform the crop (top-left corner is (0, 0))
        cropped_img = img.crop((0, 0, target_width, target_height))
        return cropped_img
    
    # If the image is smaller than the target size, leave it untouched
    print(f"No cropping required for image with dimensions: {original_width}x{original_height}")
    return img


def crop_images(img, target_aspect_ratio=(4, 3)):
    """
    Crops the given image to the target aspect ratio if it's wider than taller.
    
    Parameters:
    - img (PIL.Image): The image to be cropped.
    - target_aspect_ratio (tuple): The target aspect ratio as a (width, height) tuple.
    
    Returns:
    - PIL.Image: The cropped image.
    """
    # Get the original image dimensions
    original_width, original_height = img.size
    
    # Calculate the target width and height based on the target aspect ratio
    target_width_ratio, target_height_ratio = target_aspect_ratio
    target_aspect = target_width_ratio / target_height_ratio
    
    # Calculate the current aspect ratio of the image
    current_aspect = original_width / original_height

    # Debugging: Print the original aspect ratio and target aspect ratio
    print(f"Original Aspect Ratio: {current_aspect}, Target Aspect Ratio: {target_aspect}")
    
    # If the image is wider than the target aspect ratio, we need to crop
    if current_aspect > target_aspect:
        print(f"Cropping required for image with dimensions: {original_width}x{original_height}")
        # Calculate the new width based on the target aspect ratio
        new_width = int(original_height * target_aspect)
        
        # Calculate the horizontal cropping offsets (to crop from the center)
        left = (original_width - new_width) // 2
        right = left + new_width
        
        # Crop the image to the new dimensions
        cropped_img = img.crop((left, 0, right, original_height))
        return cropped_img
    
    # Debugging: Print message if no cropping is needed
    print(f"No cropping required for image with dimensions: {original_width}x{original_height}")
    
    # If the image is not wider than the target aspect ratio, return it as-is
    return img

def scale_image(image, target_width, target_height):
    return image.resize((target_width, target_height), Image.NEAREST)


def make_images():
    # Load the palette
    palette_filepath = f'{palette_dir}/{palette_name}'
    
    # Delete rgba output directory and recreate it
    if os.path.exists(output_dir_rgba):
        shutil.rmtree(output_dir_rgba)
    os.makedirs(output_dir_rgba)

    # Delete processed output directory and recreate it
    if os.path.exists(output_dir_png):
        shutil.rmtree(output_dir_png)
    os.makedirs(output_dir_png)

    # Convert .jpeg, .jpg, and .gif files to .png
    for input_image_filename in os.listdir(originals_dir):
        input_image_path = os.path.join(originals_dir, input_image_filename)
        if input_image_filename.endswith(('.jpeg', '.jpg', '.gif')):
            # Load the image
            img = Image.open(input_image_path)

            # Convert to .png format and save with the same name but .png extension
            png_filename = os.path.splitext(input_image_filename)[0] + '.png'
            png_filepath = os.path.join(originals_dir, png_filename)
            img.save(png_filepath, 'PNG')

            if del_non_png:
                # Optionally, delete the original .jpeg, .jpg, or .gif file after conversion
                os.remove(input_image_path)

    # Now scan the directory again for all .png files and sort them
    filenames = sorted([f for f in os.listdir(originals_dir) if f.endswith('.png')])

    # Initialize variables
    buffer_id = 256
    num_images = 0
    image_type = 1  # RGBA2222

    image_list = []
    files_list = []
    buffer_ids = []

    # Process the images
    for input_image_filename in filenames:
        input_image_path = os.path.join(originals_dir, input_image_filename)

        # Continue only if it's a .png file
        if input_image_filename.endswith('.png'):
            # Open the image
            img = Image.open(input_image_path)

            # Remove ICC profile if present to avoid the warning
            if "icc_profile" in img.info:
                img.info.pop("icc_profile")
                # Re-save the image to remove the incorrect ICC profile
                img.save(input_image_path, 'PNG')

        else:
            continue

        input_image_filepath = f'{originals_dir}/{input_image_filename}'
        file_name, ext = os.path.splitext(input_image_filename)
        output_image_filepath_png = f'{output_dir_png}/{input_image_filename}'

        start_time = time.perf_counter()

        if do_crop:
            with Image.open(input_image_filepath) as img:
                # Crop the image to the target aspect ratio if needed
                img = crop_images(img)
                img = scale_image(img, img_width, img_height)
                img.save(output_image_filepath_png)

        if do_scale:
                # Alternative: Scale to fixed size and save to original
                img = crop_images_fixed_size(img, img_width, img_height)
                img = img
                img.save(input_image_filepath)

        if do_palette:
            au.convert_to_palette(output_image_filepath_png, output_image_filepath_png, palette_filepath, palette_conv_type, transparent_rgb)

        if image_type == 1:
            rgba_filepath = f'{output_dir_rgba}/{file_name}.rgba2'
            au.img_to_rgba2(output_image_filepath_png, rgba_filepath)
        else:
            rgba_filepath = f'{output_dir_rgba}/{file_name}.rgba8'
            au.img_to_rgba8(output_image_filepath_png, rgba_filepath)

        end_time = time.perf_counter()
        print(f'{file_name} conversion took {end_time - start_time} seconds')

        buffer_ids.append(f'buf_{file_name}: equ {buffer_id}\n')

        image_width, image_height = img.width, img.height
        image_filesize = os.path.getsize(rgba_filepath)

        image_list.append(f'\tdl {image_type}, {image_width}, {image_height}, {image_filesize}, fn_{file_name}\n')

        files_list.append(f'fn_{file_name}: db "images/{file_name}.rgba2",0 \n') 

        buffer_id += 1
        num_images += 1

    # Open assembly file for writing
    with open(f'{asm_images_filepath}', 'w') as f:
        f.write(f'; Generated by make_images.py\n\n')
        f.write(f'image_type: equ 0\n')
        f.write(f'image_width: equ image_type+3\n')
        f.write(f'image_height: equ image_width+3\n')
        f.write(f'image_filesize: equ image_height+3\n')
        f.write(f'image_filename: equ image_filesize+3\n')
        f.write(f'image_record_size: equ image_filename+3\n\n')
        f.write(f'num_images: equ {num_images}\n\n')

        f.write(f'; buffer_ids:\n')
        f.write(''.join(buffer_ids))
        f.write(f'\n') 

        f.write(f'image_list: ; type; width; height; filename:\n')
        f.write(''.join(image_list))
        f.write(f'\n') 

        f.write(f'; files_list: ; filename:\n')
        f.write(''.join(files_list))

if __name__ == '__main__':
    img_width =             16
    img_height =            16
    asm_images_filepath =   'src/asm/images2.inc'
    originals_dir =         'src/assets/img/orig'
    output_dir_png =        'src/assets/img/proc'
    output_dir_rgba =       'tgt/img'
    palette_name =          'Agon64.gpl'
    palette_dir =           'build/palettes'
    palette_conv_type =     'floyd'
    transparent_rgb =       (0, 0, 0, 0)
    del_non_png =           False
    do_crop =               False
    do_scale =              False
    do_palette =            False