import argparse
import os
import subprocess
import sys

def download_youtube_audio(url, output_dir="downloads"):
    """
    Downloads a YouTube video's audio as an MP3 file using yt-dlp.
    
    Args:
        url (str): The URL of the YouTube video.
        output_dir (str): Directory to save the MP3 file. Default is 'downloads'.
        
    Returns:
        str: Path to the saved MP3 file.
    """
    # Ensure output directory exists
    os.makedirs(output_dir, exist_ok=True)

    # Define the output path for the audio
    output_template = os.path.join(output_dir, "%(title)s.%(ext)s")

    # yt-dlp command to download and convert to MP3
    command = [
        sys.executable,
        "-m",
        "yt_dlp",
        "-x",  # Extract audio
        "--audio-format", "mp3",  # Convert to MP3
        "--print", "after_move:filepath",
        "--output", output_template,  # Define output file template
        url,
    ]

    print(f"Downloading and converting: {url}")
    try:
        result = subprocess.run(
            command,
            check=True,
            capture_output=True,
            text=True,
        )
        output_paths = [line for line in result.stdout.splitlines() if line]
        if not output_paths:
            raise RuntimeError("yt-dlp completed without reporting an output path")
        output_path = output_paths[-1]
        print(f"Download complete. Audio saved to: {output_path}")
        return output_path
    except subprocess.CalledProcessError as e:
        print(f"An error occurred: {e}")
        raise

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Download audio with yt-dlp")
    parser.add_argument("url", help="Video URL to download")
    parser.add_argument(
        "--output-dir",
        default="assets/sound/music",
        help="Directory for the converted MP3",
    )
    args = parser.parse_args()
    print(download_youtube_audio(args.url, args.output_dir))
