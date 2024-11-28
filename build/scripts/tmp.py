import sys

def integer_sqrt(value):
    """
    Simulate the integer square root algorithm.
    Computes the integer square root of a 24-bit input value.
    
    Parameters:
        value (int): 24-bit unsigned integer input.
    
    Returns:
        tuple: (root, remainder)
            - root: The integer square root of the input.
            - remainder: The leftover value after subtracting root^2.
    """
    remainder = 0  # This holds the shifted and updated remainder during computation
    root = 0       # This builds the square root, one bit at a time

    # There are 12 iterations for a 24-bit input because we process 2 bits of the input per iteration
    for i in range(12):
        # Step 1: Shift the remainder left by 2 bits to make room for the next 2 bits of input
        remainder = (remainder << 2) | ((value >> (22 - 2 * i)) & 0b11)
        # Bring down the next 2 bits of the input into the remainder
        
        # Step 2: Form the next trial root by shifting the current root left by 1 and adding 1
        # This represents testing the next bit of the square root
        trial = (root << 1) | 1

        # Step 3: Test if the trial root squared is less than or equal to the remainder
        if remainder >= trial:
            # If the trial is successful, subtract it from the remainder
            remainder -= trial
            # Update the root by setting the bit we just tested
            root = (root << 1) | 1
        else:
            # If the trial fails, just shift the root left without setting the new bit
            root = root << 1

    # Post-processing step: Correct overshoot if necessary
    if root * root > value:
        root -= 1
        remainder += (root << 1) + 1  # Adjust remainder to restore consistency

    # At the end of 12 iterations, 'root' holds the integer square root, and 'remainder' holds the leftover
    return root, remainder



if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python sqrt24.py <value>")
        print("<value> should be a 24-bit unsigned integer (0 to 16777215).")
        sys.exit(1)

    try:
        # Parse the input value
        value = int(sys.argv[1])
        if value < 0 or value > 0xFFFFFF:
            raise ValueError("Value out of range.")
    except ValueError as e:
        print(f"Error: {e}")
        sys.exit(1)

    # Compute the square root
    root, remainder = integer_sqrt(value)
    print(f"Input: {value} (0x{value:06X})")
    print(f"Integer Square Root: {root} (0x{root:06X})")
    print(f"Remainder: {remainder} (0x{remainder:06X})")
