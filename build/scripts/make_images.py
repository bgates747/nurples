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

def rotate_image(image, degrees256):
    # Calculate the rotation angle in degrees (left-handed coordinate system)
    degrees = 360 - (degrees256 * 360 / 256) 
    
    # Get the original size of the image
    original_width, original_height = image.size
    
    # Rotate the image about its center
    rotated_image = image.rotate(degrees, resample=Image.NEAREST, expand=True)
    
    # Calculate the coordinates to crop the image back to the original size
    center_x, center_y = rotated_image.size[0] // 2, rotated_image.size[1] // 2
    left = center_x - original_width // 2
    top = center_y - original_height // 2
    right = center_x + original_width // 2
    bottom = center_y + original_height // 2
    
    # Crop the rotated image back to the original size, centered
    cropped_image = rotated_image.crop((left, top, right, bottom))
    
    return cropped_image

def make_images(buffer_id, img_width, img_height, images_type, asm_images_filepath, originals_dir, output_dir_png, output_dir_rgba, palette_name, palette_dir, palette_conv_type, transparent_rgb, del_non_png, do_crop, do_scale, do_palette):
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

    # Copy all .png files from the input directory to the output png directory
    for input_image_filename in os.listdir(originals_dir):
        if input_image_filename.endswith('.png'):
            input_image_path = os.path.join(originals_dir, input_image_filename)
            output_image_path = os.path.join(output_dir_png, input_image_filename)
            shutil.copy(input_image_path, output_image_path)

    rot_images_list = ['seeker.png','turret.png']
    for input_image_filename in rot_images_list:
        input_image_path = os.path.join(output_dir_png, input_image_filename)
        if not os.path.exists(input_image_path):
            continue
        output_image_path = os.path.join(output_dir_png, input_image_filename)
        img = Image.open(input_image_path)
        os.remove(input_image_path)
        for degrees256 in range(0, 256, 8):
            output_image_path = os.path.join(output_dir_png, f'{os.path.splitext(input_image_filename)[0]}_{degrees256:03d}.png')
            img_rot = rotate_image(img, degrees256)
            img_rot.save(output_image_path)

    # Scan the output directory for all .png files and sort them
    filenames = sorted([f for f in os.listdir(output_dir_png) if f.endswith('.png')])

    # Initialize variables
    num_images = 0
    image_type = 1  # RGBA2222

    image_list = []
    files_list = []
    buffer_ids = []

    # Process the images
    for input_image_filename in filenames:
        input_image_path = os.path.join(output_dir_png, input_image_filename)

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

        image_filepath = f'{output_dir_png}/{input_image_filename}'
        file_name, ext = os.path.splitext(input_image_filename)

        with Image.open(image_filepath) as img:
            start_time = time.perf_counter()

            if do_crop:
                    img = crop_images_fixed_size(img, img_width, img_height)
                    img = img
                    img.save(image_filepath)

            if do_scale:
                    img = crop_images(img)
                    img = scale_image(img, img_width, img_height)
                    img.save(image_filepath)

            if do_palette:
                au.convert_to_palette(image_filepath, image_filepath, palette_filepath, palette_conv_type, transparent_rgb)

            if image_type == 1:
                rgba_filepath = f'{output_dir_rgba}/{file_name}.rgba2'
                au.img_to_rgba2(image_filepath, rgba_filepath)
            else:
                rgba_filepath = f'{output_dir_rgba}/{file_name}.rgba8'
                au.img_to_rgba8(image_filepath, rgba_filepath)

            end_time = time.perf_counter()
            print(f'{file_name} conversion took {end_time - start_time} seconds')

            buffer_ids.append(f'BUF_{file_name.upper()}: equ {buffer_id}\n')

            image_width, image_height = img.width, img.height
            image_filesize = os.path.getsize(rgba_filepath)

            image_list.append(f'\tdl {image_type}, {image_width}, {image_height}, {image_filesize}, fn_{file_name}, {buffer_id}\n')

            files_list.append(f'fn_{file_name}: db "{images_type}/{file_name}.rgba2",0 \n') 

            buffer_id += 1
            num_images += 1

    # Open assembly file for writing
    with open(f'{asm_images_filepath}', 'w') as f:
        f.write(f'; Generated by make_images.py\n\n')

        f.write(f'{images_type}_num_images: equ {num_images}\n\n')

        f.write(f'; buffer_ids:\n')
        f.write(''.join(buffer_ids))
        f.write(f'\n') 

        f.write(f'{images_type}_image_list: ; type; width; height; filename; bufferId:\n')
        f.write(''.join(image_list))
        f.write(f'\n') 

        f.write(f'; files_list: ; filename:\n')
        f.write(''.join(files_list))

if __name__ == '__main__':
    img_width =             16
    img_height =            16
    buffer_id =             256
    images_type =           'sprites'
    asm_images_filepath =  f'src/asm/images_{images_type}.inc'
    originals_dir =        f'src/assets/img/orig/{images_type}'
    output_dir_png =       f'src/assets/img/proc/{images_type}'
    output_dir_rgba =      f'tgt/{images_type}'
    palette_name =          'Agon64.gpl'
    palette_dir =           'build/palettes'
    palette_conv_type =     'floyd'
    transparent_rgb =       (0, 0, 0, 0)
    del_non_png =           False
    do_crop =               False
    do_scale =              False
    do_palette =            False
    make_images(buffer_id, img_width, img_height, images_type, asm_images_filepath, originals_dir, output_dir_png, output_dir_rgba, palette_name, palette_dir, palette_conv_type, transparent_rgb, del_non_png, do_crop, do_scale, do_palette)

    img_width =             16
    img_height =            16
    buffer_id =             0
    images_type =           'ui'
    asm_images_filepath =  f'src/asm/images_{images_type}.inc'
    originals_dir =        f'src/assets/img/orig/{images_type}'
    output_dir_png =       f'src/assets/img/proc/{images_type}'
    output_dir_rgba =      f'tgt/{images_type}'
    palette_name =          'Agon64.gpl'
    palette_dir =           'build/palettes'
    palette_conv_type =     'floyd'
    transparent_rgb =       (0, 0, 0, 0)
    del_non_png =           False
    do_crop =               False
    do_scale =              False
    do_palette =            False
    make_images(buffer_id, img_width, img_height, images_type, asm_images_filepath, originals_dir, output_dir_png, output_dir_rgba, palette_name, palette_dir, palette_conv_type, transparent_rgb, del_non_png, do_crop, do_scale, do_palette)