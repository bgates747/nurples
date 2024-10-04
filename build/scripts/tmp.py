import matplotlib.pyplot as plt
from PIL import Image, ImageDraw, ImageFont

# Define font path and the character to render
font_path = '/usr/share/fonts/truetype/noto/NotoSansMono-Regular.ttf'
char_to_render = "'"

# Set the target point size (as computed earlier)
target_point_size = 52  # From previous calculations

# Load the font
font = ImageFont.truetype(font_path, target_point_size)

# Compute the exact dimensions we need based on the previous calculations
# We already know the resulting dimensions (from prior calculations)
resulting_width = 32
resulting_height = 68  # This was calculated earlier as the height needed to maintain aspect

# Create an image to render the character at the correct dimensions
temp_img = Image.new("L", (resulting_width, resulting_height), color=0)  # "L" mode for grayscale, black background
draw = ImageDraw.Draw(temp_img)

# Draw the character in white at the correct size
draw.text((0, 0), char_to_render, font=font, fill=255)

# Resize the character image to fit into the final 32x32 box (scaling whichever axis needs it)
final_img = temp_img.resize((32, 32), Image.BICUBIC)

# Convert to numpy array and plot the result
plt.imshow(final_img, cmap="gray")
plt.axis("off")
plt.show()
