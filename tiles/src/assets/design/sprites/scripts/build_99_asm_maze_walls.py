
def make_assembly_file(map_type, tile_size, maze_null_tile, base_bufferId, map_filepath, asm_filepath):
    # Read the map file
    with open(f'{map_filepath}', 'r') as f:
        map_lines = f.readlines()

    num_cols = len(map_lines[0].strip().split(','))
    num_rows = len(map_lines)

    # Write the assembly file
    with open(f'{asm_filepath}', 'w') as f:
        f.write(f'; Generated by beegee747/build/scripts/build_99_asm_maze_walls.py\n\n')
        f.write(f'{map_type}:\n')
        f.write(f'{map_type}_num_cols: dl {num_cols}\n')
        f.write(f'{map_type}_num_rows: dl {num_rows}\n')
        f.write(f'{map_type}_tile_size: dl {tile_size}\n')
        f.write(f'{map_type}_null_tile: dl {maze_null_tile}\n')
        f.write(f'{map_type}_x: dl 0\n')
        f.write(f'{map_type}_y: dl 0\n')
        f.write(f'{map_type}_base_bufferId: dl {base_bufferId}\n\n')

        f.write(f'{map_type}_map:\n')
        for line in map_lines:
            line = line.strip()
            f.write(f'    db {line}\n')


if __name__ == '__main__':
    map_type =              'maze_walls'
    tile_size =             8
    maze_null_tile =        6
    base_bufferId =         'BUF_TILE_00'
    map_filepath =         f'beegee747/src/assets/design/sprites/{map_type}/{map_type}.csv'
    asm_filepath =         f'beegee747/src/asm/{map_type}.inc'

    make_assembly_file(map_type, tile_size, maze_null_tile, base_bufferId, map_filepath, asm_filepath)

    map_type =              'maze_pellets'
    tile_size =             8
    maze_null_tile =        0
    base_bufferId =         'BUF_MAZE_PELLETS_00'
    map_filepath =         f'beegee747/src/assets/design/sprites/{map_type}/{map_type}.csv'
    asm_filepath =         f'beegee747/src/asm/{map_type}.inc'

    make_assembly_file(map_type, tile_size, maze_null_tile, base_bufferId, map_filepath, asm_filepath)