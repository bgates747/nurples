import subprocess
import os
import shutil

def do_all_the_things(db_path, map_dim_x, map_dim_y, tgt_dir, floor_nums):
    # build_00_delete_tgt_dir.py
    if do_00_delete_tgt_dir:
        print(f"Deleting target directory: {tgt_dir}")
        # Check and delete the target directory if necessary
        if os.path.exists(tgt_dir):
            shutil.rmtree(tgt_dir)
        os.makedirs(tgt_dir)

    # build_02_fetch_tiles.py
    src_tiles_path = 'src/mapmaker/tiles.txt'
    mapmaker_tiles_dir = 'src/mapmaker'
    uvs_tgt_dir = 'build/panels/uv'
    thumbs_tgt_dir = 'build/panels/thumbs'
    if do_02_fetch_tiles:
        print(f"build_02_fetch_tiles: Fetching tiles")
        from build_02_fetch_tiles import fetch_tiles
        fetch_tiles(db_path, src_tiles_path, mapmaker_tiles_dir, uvs_tgt_dir, thumbs_tgt_dir)

# build_06b_map_import_mapmaker
    map_src_dir = f'src/mapmaker'
    if do_06_import_mapmaker_files:
        print(f"build_06_map_import_mapmaker: Importing mapmaker files")
        from build_06_map_import_mapmaker import import_mapmaker
        for floor_num in floor_nums:
            import_mapmaker(db_path, floor_num, map_src_dir, map_dim_x, map_dim_y)

# build_08_make_sfx.py
    sfx_src_dir = 'src/assets/sfx'
    sfx_tgt_dir = 'tgt/sfx'
    if do_08_make_sfx:
        print(f"build_08_make_sfx: Making sound effects")
        from build_08_make_sfx import make_sfx
        make_sfx(db_path, sfx_src_dir, sfx_tgt_dir)
        
        
# build_91_asm_img_load.py
    if do_91_asm_img_load:
        print(f"build_91_asm_img_load: Making image load assembler file")
        from build_91_asm_img_load import make_asm_images_inc
        panels_inc_path = f"src/asm/images.asm"
        next_buffer_id_counter = 256
        make_asm_images_inc(db_path, panels_inc_path, next_buffer_id_counter)

# build_91a_asm_font.py
    if do_91a_asm_font:
        next_buffer_id = 0x1000
        print(f"build_91a_asm_font: Making font assembler file")
        from build_91a_asm_font import maken_zee_fonts
        maken_zee_fonts(next_buffer_id)

# build_98_asm_sfx.py
    sfx_inc_path = 'src/asm/sfx.asm'
    sfx_tgt_dir = 'sfx'
    if do_98_asm_sfx:
        print(f"build_98_asm_sfx: Making sfx assembler file")
        from build_98_asm_sfx import make_asm_sfx
        # next_buffer_id = 0x3000
        next_buffer_id = 64256 
        make_asm_sfx(db_path, sfx_inc_path, sfx_tgt_dir, next_buffer_id)

# build_99_asm_assemble.py
    if do_99_asm_assemble:
        print(f"build_99_asm_assemble: Assembling application")
        from build_99_asm_assemble import do_assembly
        src_file = 'src/asm/wolf3d.asm'
        do_assembly(src_file, tgt_dir)


if __name__ == "__main__":
# Set build parameters
    # Set paths
    db_path = 'build/data/build.db' # Literally everything the app needs to build goes through this database
    tgt_dir = 'tgt' # This is where all the build artifacts go

# By default don't run any scripts
    do_00_delete_tgt_dir = False
    do_02_fetch_tiles = False
    do_06_import_mapmaker_files = False
    do_08_make_sfx = False
    do_91_asm_img_load = False
    do_91a_asm_font = False
    do_98_asm_sfx = False
    do_99_asm_assemble = False

# I find it easier to simply comment out the scripts I don't want to run
    do_00_delete_tgt_dir = True
    do_02_fetch_tiles = True
    do_06_import_mapmaker_files = True
    do_08_make_sfx = True
    do_91_asm_img_load = True
    do_91a_asm_font = True
    do_98_asm_sfx = True
    do_99_asm_assemble = True

    map_dim_x, map_dim_y = 16, 16 # Don't mess with this

    # Set which maps to build
    floor_nums = list(range(1))

    do_all_the_things(db_path, map_dim_x, map_dim_y, tgt_dir, floor_nums)