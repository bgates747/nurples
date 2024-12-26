import random
from PIL import Image

# Load the images
img0 = Image.open('assets/img/design/fireball_0.png').convert('RGBA')
img1 = Image.open('assets/img/design/fireball_1.png').convert('RGBA')
img2 = Image.open('assets/img/design/fireball_2.png').convert('RGBA')
img2 = Image.open('assets/img/design/fireball_3.png').convert('RGBA')

# Ensure all images are the same size
assert img0.size == img1.size == img2.size, "All images must be the same size"

width, height = img0.size

for i in range(4):
    # Create a new image for the output
    output_img = Image.new('RGBA', (width, height))

    # Get pixel data
    pixels0 = img0.load()
    pixels1 = img1.load()
    pixels2 = img2.load()
    output_pixels = output_img.load()

    # Randomly sample pixels from the three images
    for x in range(width):
        for y in range(height):
            choice = random.choice([0, 1, 2])
            if choice == 0:
                output_pixels[x, y] = pixels0[x, y]
            elif choice == 1:
                output_pixels[x, y] = pixels1[x, y]
            else:
                output_pixels[x, y] = pixels2[x, y]

    # Save the output image
    output_img.save(f'assets/img/orig/sprites/fireball_{i}.png')