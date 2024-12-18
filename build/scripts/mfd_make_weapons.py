from PIL import Image
import os

def replace_colors(input_folder, output_folder):
    # Ensure output folder exists
    os.makedirs(output_folder, exist_ok=True)

    # Define color mappings
    colors = [
        (170, 170, 170, 255),  # aaaaaa empty
        (0, 170, 0, 255),      # 00aa00 equipped
        (0, 255, 0, 255),      # 00ff00 active
        (170, 170, 0, 255),    # aaaa00 damaged equipped
        (255, 255, 0, 255),    # ffff00 damaged active
        (255, 0, 0, 255)       # ff0000 destroyed
    ]

    for filename in os.listdir(input_folder):
        if filename.endswith('.png'):
            filepath = os.path.join(input_folder, filename)
            original_image = Image.open(filepath).convert("RGBA")

            # Create six variations of the image
            for index, color in enumerate(colors):
                # Create a new image
                new_image = Image.new("RGBA", original_image.size)
                pixels = original_image.load()
                new_pixels = new_image.load()

                # Apply color to non-transparent pixels
                for x in range(original_image.width):
                    for y in range(original_image.height):
                        r, g, b, a = pixels[x, y]
                        if a > 0:  # Non-transparent
                            new_pixels[x, y] = color
                        else:
                            new_pixels[x, y] = (0, 0, 0, 0)  # Transparent

                # Save the modified image with the new filename
                base_name = os.path.splitext(filename)[0]
                new_filename = f"{base_name}_{index}.png"
                new_image.save(os.path.join(output_folder, new_filename))

if __name__ == "__main__":
    input_folder = "assets/img/temp"
    output_folder = "assets/img/temp/output"
    replace_colors(input_folder, output_folder)
