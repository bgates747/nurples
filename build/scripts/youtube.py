import os
import subprocess

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
        "yt-dlp",
        "-x",  # Extract audio
        "--audio-format", "mp3",  # Convert to MP3
        "--output", output_template,  # Define output file template
        url,
    ]

    print(f"Downloading and converting: {url}")
    try:
        subprocess.run(command, check=True)
        print(f"Download complete. Audio saved to: {output_dir}")
    except subprocess.CalledProcessError as e:
        print(f"An error occurred: {e}")
        raise

if __name__ == "__main__":
    # Example usage
    # youtube_url = "https://youtu.be/0xGPi-Al3zQ" # Rhiannon by Fleetwood Mac
    # youtube_url = "https://youtu.be/Epj84QVw2rc" # Come Undone by Duran Duran
    # youtube_url = "https://youtu.be/1lyu1KKwC74"  # Bitter Sweet Symphony by The Verve
    youtube_url = "https://youtu.be/Kb7lAMjFuA0" # Africa by Toto
    # youtube_url = "https://youtu.be/6cucosmPj-A"  # Every Breath You Take by The Police
    output_directory = "assets/sound/music"
    try:
        mp3_path = download_youtube_audio(youtube_url, output_directory)
        print(f"MP3 file saved at: {mp3_path}")
    except Exception as e:
        print(f"An error occurred: {e}")
