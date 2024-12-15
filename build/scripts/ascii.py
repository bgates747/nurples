def format_ascii_art_for_assembly(ascii_art):
    """
    Formats ASCII art for assembly programs by:
    - Converting each character to its ASCII value
    - Adding ASCII values for `\r` and `\n` at the end of each line
    - Outputting as `db` statements with comma-separated values
    """
    formatted_lines = []
    for line in ascii_art.splitlines():
        # Convert each character to its ASCII value
        ascii_values = [ord(char) for char in line.rstrip()]
        # Add \r (13) and \n (10) terminators
        ascii_values.extend([13, 10])
        # Format as a db statement
        formatted_line = f"    db {','.join(map(str, ascii_values))}"
        formatted_lines.append(formatted_line)
    return "\n".join(formatted_lines)

if __name__ == "__main__":
    # Example ASCII art
    ascii_art = """
        __    __                 __                                   
_____ _/  |__/  |______    ____ |  | __                               
\__  \\   __\   __\__  \ _/ ___\|  |/ /                               
 / __ \|  |  |  |  / __ \\  \___|    <                                
(____  /__|  |__| (____  /\___  >__|_ \                               
     \/ _____    __  .__/     \/     \/                               
  _____/ ____\ _/  |_|  |__   ____                                    
 /  _ \   __\  \   __\  |  \_/ __ \                                   
(  <_> )  |     |  | |   Y  \  ___/                                   
 \____/|__|     |__| |___|  /\___  >                                  
__________ ____ _______________________.____     ___________          
\______   \    |   \______   \______   \    |    \_   _____/          
 |     ___/    |   /|       _/|     ___/    |     |    __)_           
 |    |   |    |  / |    |   \|    |   |    |___  |        \          
 |____|   |______/  |____|_  /|____|   |_______ \/_______  /          
 _______   ____ _______________________.____   \/___________ _________
 \      \ |    |   \______   \______   \    |    \_   _____//   _____/
 /   |   \|    |   /|       _/|     ___/    |     |    __)_ \_____  \ 
/    |    \    |  / |    |   \|    |   |    |___  |        \/        \\
\____|__  /______/  |____|_  /|____|   |_______ \/_______  /_______  /
        \/                 \/                  \/        \/        \/ 
"""

    # Process the ASCII art
    formatted_output = format_ascii_art_for_assembly(ascii_art)

    # # Print the result
    # print(formatted_output)

    # Save the formatted output to a file
    output_file_path = 'src/asm/ascii.inc'
    with open(output_file_path, 'w') as output_file:
        output_file.write(formatted_output)