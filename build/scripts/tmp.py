

def adjust_dimensions(screen_width, screen_height, char_cols, char_rows):
    """
    Adjusts the character dimensions to the nearest values such that the screen width and height
    are evenly divisible by the target character width and height.

    :param screen_width: The width of the screen in pixels.
    :param screen_height: The height of the screen in pixels.
    :param char_cols: The number of columns (characters) across the screen.
    :param char_rows: The number of rows (characters) down the screen.
    :return: Adjusted target character width and height.
    """

    # Ideal width and height per character
    ideal_width = screen_width / char_cols
    ideal_height = screen_height / char_rows

    # Round to the nearest integer such that screen resolution is divisible by character dimensions
    target_width = round(ideal_width)
    target_height = round(ideal_height)

    # Adjust width and height to ensure exact divisibility
    while screen_width % target_width != 0:
        target_width += 1 if ideal_width < target_width else -1

    while screen_height % target_height != 0:
        target_height += 1 if ideal_height < target_height else -1

    return target_width, target_height

def get_unique_screen_resolutions():
    """
    Extracts and returns a unique list of screen resolutions from the given data.
    Outputs them in an ordered format ready to be pasted into source code.
    """
    screen_modes = [
        (320, 240), (512, 384), (640, 240), (640, 480), (800, 600), (1024, 768)
    ]

    # Remove duplicates and sort by width, then height
    unique_resolutions = sorted(list(set(screen_modes)), key=lambda x: (x[0], x[1]))

    # Output in source code-friendly format
    print("Screen resolutions:")
    print(", ".join([f"({w}, {h})" for w, h in unique_resolutions]))

    return unique_resolutions


def get_unique_text_resolutions():
    """
    Returns a unique list of text resolutions to be used for computing character resolutions.
    """
    text_resolutions = [
        (40, 24),
        (80, 24)
    ]

    # Return text resolutions directly
    return text_resolutions


def calculate_character_resolutions():
    """
    Combines screen resolutions with text resolutions, adjusts character dimensions, 
    and outputs a unique, ordered list of character resolutions.
    """
    # Get screen resolutions
    screen_resolutions = get_unique_screen_resolutions()

    # Get text resolutions
    text_resolutions = get_unique_text_resolutions()

    # Initialize a set to hold unique character resolutions
    unique_character_resolutions = set()

    # Loop through all screen and text resolution combinations
    for screen_width, screen_height in screen_resolutions:
        for char_cols, char_rows in text_resolutions:
            # Adjust character dimensions for the screen resolution and text resolution
            target_width, target_height = adjust_dimensions(screen_width, screen_height, char_cols, char_rows)
            
            # Add the resulting character resolution to the set
            unique_character_resolutions.add((target_width, target_height))

    # Sort the character resolutions by width, then height
    sorted_character_resolutions = sorted(list(unique_character_resolutions), key=lambda x: (x[0], x[1]))

    # Output the unique character resolutions in source code-friendly format
    print("\nCharacter resolutions:")
    print(", ".join([f"({w}, {h})" for w, h in sorted_character_resolutions]))

    return sorted_character_resolutions


if __name__ == '__main__':
    # Call the function to generate and print unique character resolutions
    calculate_character_resolutions()
