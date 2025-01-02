import os
import sqlite3
import shutil
import subprocess

def make_tbl_08_sfx(conn, cursor):
    cursor.execute("""
        drop table if exists tbl_08_sfx;""")
    conn.commit()
    cursor.execute("""
        create table if not exists tbl_08_sfx (
            sfx_id integer,
            size integer,
            duration integer,
            filename text,
            primary key (sfx_id)
        );""")
    conn.commit()

def make_sfx(db_path, src_dir, tgt_dir):
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    # Create the target directory
    if os.path.exists(tgt_dir):
        shutil.rmtree(tgt_dir)
    os.makedirs(tgt_dir)

    sfxs = []
    for filename in sorted(os.listdir(src_dir)):
        if filename.endswith('.wav'):
            filename = filename.replace('.wav', '.raw')
            sfxs.append((len(sfxs) + 0, filename))

    for sfx in sfxs:
        sfx_id, filename = sfx
        src_path = os.path.join(src_dir, filename.replace('.raw', '.wav'))
        tgt_path = os.path.join(tgt_dir, filename)

        # Step 1: Normalize the audio to prevent clipping
        normalized_path = os.path.join(tgt_dir, 'normalized_temp.wav')
        subprocess.run([
            'ffmpeg',
            '-i', src_path,                      # Input file
            '-af', 'loudnorm=I=-24:TP=-2:LRA=11', # Normalize loudness
            normalized_path                       # Output file
        ], check=True)

        # Step 2: Resample the audio to 16 kHz
        resampled_path = os.path.join(tgt_dir, 'resampled_temp.wav')
        subprocess.run([
            'ffmpeg',
            '-i', normalized_path,               # Input file
            '-ar', '16384',                      # Resample to 16 kHz
            resampled_path                       # Output file
        ], check=True)

        # Step 3: Convert the audio to signed 8-bit PCM
        subprocess.run([
            'ffmpeg',
            '-i', resampled_path,                # Input file
            '-ac', '1',                          # Set audio channels to mono
            '-f', 's8',                          # Format: signed 8-bit PCM
            '-acodec', 'pcm_s8',                 # Codec: PCM signed 8-bit
            tgt_path                             # Output file
        ], check=True)

        # Remove temporary files
        os.remove(normalized_path)
        os.remove(resampled_path)

        # Calculate size and duration
        size = os.path.getsize(tgt_path)
        duration = size // 16.384  # Assuming size is in bytes for 8-bit mono at 16 kHz
        cursor.execute("""
            insert into tbl_08_sfx (sfx_id, size, duration, filename)
            values (?, ?, ?, ?);""", (sfx_id, size, duration, filename))

    conn.commit()
    conn.close()

if __name__ == '__main__':
    db_path = 'build/data/build.db'
    # src_dir = 'src/assets/sfx'
    # tgt_dir = 'tgt/sfx'

    # make_sfx(db_path, src_dir, tgt_dir)

    src_dir = 'assets/sound/music'
    tgt_dir = 'tgt/music'

    make_sfx(db_path, src_dir, tgt_dir)