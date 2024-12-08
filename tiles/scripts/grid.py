from PIL import Image, ImageDraw, ImageFont

# Create a 256x256 transparent image
image_size = 256
cell_size = 16
image = Image.new("RGBA", (image_size, image_size), (0, 0, 0, 0))

# Load a font for drawing the numbers
try:
    font = ImageFont.truetype("arial.ttf", 10)
except IOError:
    # Use default font if arial.ttf is unavailable
    font = ImageFont.load_default()

draw = ImageDraw.Draw(image)

# Draw hex numbers in each 16x16 grid cell
for y in range(0, image_size, cell_size):
    for x in range(0, image_size, cell_size):
        cell_number = (y // cell_size) * (image_size // cell_size) + (x // cell_size) + 1
        hex_number = "0" if cell_number == 256 else f"{cell_number:02X}"
        draw.text((x + 1, y + 1), hex_number, fill="white", font=font)

# Display the image
image.show()
