import os
from pytube import YouTube
import ffmpeg


def download_youtube_audio(url, output_dir):
    """
    Downloads a YouTube video and extracts its audio as an MP3 file.
    
    Args:
        url (str): The URL of the YouTube video.
        output_dir (str): Directory to save the MP3 file. Default is 'downloads'.
        
    Returns:
        str: Path to the saved MP3 file.
    """
    # Ensure output directory exists
    os.makedirs(output_dir, exist_ok=True)

    # Download the audio stream
    print(f"Downloading audio from: {url}")
    yt = YouTube(url)
    audio_stream = yt.streams.filter(only_audio=True).first()
    downloaded_path = audio_stream.download(output_dir)
    print(f"Downloaded to: {downloaded_path}")

    # Convert to MP3 using ffmpeg
    mp3_path = os.path.splitext(downloaded_path)[0] + ".mp3"
    print(f"Converting to MP3: {mp3_path}")
    ffmpeg.input(downloaded_path).output(mp3_path, format="mp3", acodec="libmp3lame").run(
        quiet=True, overwrite_output=True
    )

    # Remove the original file
    os.remove(downloaded_path)
    print(f"Deleted original file: {downloaded_path}")

    print(f"Audio saved as: {mp3_path}")
    return mp3_path

if __name__ == "__main__":
    # Example usage
    youtube_url = "https://youtu.be/0xGPi-Al3zQ"
    output_directory = "assets/sound/music"
    try:
        mp3_path = download_youtube_audio(youtube_url, output_directory)
        print(f"MP3 file saved at: {mp3_path}")
    except Exception as e:
        print(f"An error occurred: {e}")
