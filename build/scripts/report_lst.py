#!/usr/bin/env python3
"""
scan asm listing (nurples.lst) for included files and produce a report of their address ranges, size and so forth.
the input file lines have the format:
<address>   <line_num>   [possibly some whitespace] <mnemonic or directive> ...

we need to identify lines that match something like:
04068E             0031       include "maths.inc"

key points:
- we must find lines that have 'include "' after the line number with no intervening semicolons.
- if there's a semicolon before 'include "', that line is commented out, ignore it.
- addresses start at 040000 hex (this is the start of header).
- first include occurs some lines later. The difference between first include's address and the start address is the header size.
- after collecting includes, we also identify when main program starts (at line with `; --- MAIN PROGRAM FILE ---`)
- from that line to EOF is main app section.
- last line of the file gives final address for the entire program.
- output a simple report to console.
"""

def parse_line(line):
    # example line:
    # 04068E             0031       include "maths.inc"
    # fields might be split by whitespace in complicated ways, let's try a safe parse:
    # address: first 6 chars hex
    # some blanks
    # then line number?
    # rest could have code, possibly a comment starting with ';'
    # We'll assume the address is always at the start of line, followed by spaces, then line number.
    # If line isn't well-formed, return None for address or line_num
    parts = line.strip('\n')
    # Extract address: first 6 chars should be hex address
    if len(parts) < 6:
        return None, None, line
    addr_part = parts[0:6]
    # check address is hex
    try:
        address = int(addr_part, 16)
    except ValueError:
        return None, None, line
    # remainder after address
    remainder = line[6:].strip()
    # line number might be separated by multiple spaces, find first sequence of digits
    # We'll split by whitespace and try to parse second token as line number:
    # The line might have variable spacing, best approach: remove leading/trailing, split on whitespace
    tokens = remainder.split()
    if not tokens:
        # no tokens after address line
        return address, None, line
    # first token might or might not be line number, sometimes blank or something else
    # We'll guess that the first numeric token after the address is the line number
    line_num = None
    # find first purely numeric token
    numeric_found = False
    numeric_index = None
    for i, t in enumerate(tokens):
        if t.isdigit():
            line_num = int(t)
            numeric_found = True
            numeric_index = i
            break
    # We'll reconstruct the code portion from tokens after line_num
    if numeric_found:
        code_tokens = tokens[numeric_index+1:]
    else:
        # no line number found
        code_tokens = tokens

    code_line = ' '.join(code_tokens)

    return address, line_num, code_line


def main():
    filename = "src/asm/nurples.lst"
    start_address = 0x40000
    first_include_address = None
    includes = []  # list of (file, start_address, end_address)
    last_include_end = None
    main_program_start_address = None
    final_address = None
    current_include_file = None
    current_include_start = None

    # We identify includes by scanning lines until we find lines that have 'include "'
    # with no intervening semicolons between line number and include directive.
    # We'll store them, and when next include or main program comment line found, we close previous include range.

    # The line that indicates main program start:
    main_start_marker = "; --- MAIN PROGRAM FILE ---"

    # We'll also assume that the final line of file sets final_address from address field.

    with open(filename, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    for i, line in enumerate(lines):
        address, line_num, code_line = parse_line(line)
        if address is not None:
            final_address = address  # always keep track of last address seen
        # Check for main program start marker
        # If found, record main_program_start_address = address of that line
        if main_start_marker in line and main_program_start_address is None:
            main_program_start_address = address
            # if we are currently tracking an include, close it
            if current_include_file is not None:
                includes.append((current_include_file, current_include_start, address - 1))
                current_include_file = None
                current_include_start = None
            continue

        # If line_num is not None, and 'include "' appears in code_line,
        # and there's no semicolon before it:
        # We must ensure no ';' appears before 'include "' in code_line.
        if line_num is not None and 'include "' in code_line:
            # Check position of 'include "' and ';'
            inc_pos = code_line.index('include "')
            sem_pos = code_line.find(';')
            if sem_pos == -1 or sem_pos > inc_pos:
                # This is a real include line
                # close previous include if any
                if current_include_file is not None:
                    # close out previous include range
                    includes.append((current_include_file, current_include_start, address - 1))
                    current_include_file = None
                    current_include_start = None

                # extract filename inside quotes
                # code_line could look like: include "maths.inc"
                start_quote = code_line.find('"')
                end_quote = code_line.find('"', start_quote+1)
                if start_quote != -1 and end_quote != -1:
                    inc_file = code_line[start_quote+1:end_quote]
                else:
                    inc_file = "UNKNOWN"

                current_include_file = inc_file
                current_include_start = address
                if first_include_address is None:
                    first_include_address = address
            # else it's commented out, ignore

    # if we ended file and still have an open include
    if current_include_file is not None:
        includes.append((current_include_file, current_include_start, final_address))

    # now we have includes and main_program_start_address
    # everything from start_address(=0x40000) to first_include_address-1 is header
    # everything from main_program_start_address to final_address is main app

    # If there's a main program start address, that means the line was found
    # If not found, main program might be absent or scenario different
    # We'll still do a report.

    header_start = start_address
    header_end = (first_include_address - 1) if first_include_address else (start_address - 1)

    # print report
    print("PROGRAM REPORT")
    print("==============")
    print(f"Header: {header_start:06X} - {header_end:06X}  size: {header_end - header_start + 1 if first_include_address else 0} bytes")
    print()
    print("INCLUDES:")
    for inc_file, start_addr, end_addr in includes:
        size = end_addr - start_addr + 1
        print(f" {inc_file}: {start_addr:06X} - {end_addr:06X}  size: {size} bytes")
    print()
    if main_program_start_address is not None:
        main_size = final_address - main_program_start_address + 1
        print(f"MAIN PROGRAM: {main_program_start_address:06X} - {final_address:06X} size: {main_size} bytes")
    else:
        # no main program marker found, main might start after last include
        if includes:
            mp_start = includes[-1][2] + 1
        else:
            mp_start = start_address
        main_size = final_address - mp_start + 1
        print(f"MAIN PROGRAM: {mp_start:06X} - {final_address:06X} size: {main_size} bytes")

    print(f"TOTAL PROGRAM SIZE: {final_address - start_address + 1} bytes")

if __name__ == "__main__":
    main()
