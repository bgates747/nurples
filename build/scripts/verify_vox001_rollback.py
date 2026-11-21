"""Verify the five guest files of the approved VOX-001 production checkpoint."""
import argparse
from hashlib import sha256
from pathlib import Path


PINS = {
    "nurples.bin": "a607169c4431b54b0c34b28cb6e4ba827fc9aa7198d4cb21e9af1fc1c1d0457e",
    "voice.agnb": "6f11f4f52b6194dbcdefcfa39bfdcf1d685a33d4ef3b67dafa3e3ae62c52d9b3",
    "game.agnb": "652c2b01de5dbb24100aa53d32fa2aa16f454763cad1432423bf51ae52a058ba",
    "ui.agnb": "d0024003369c8674a3e7989efa6d45c264ab2a455e6da628b16b2afd9594d41a",
    "fonts/Lat38-VGA8_8x8.font": "23321abb10437db6b8bc8f21907268b46615d409b38975f6a195eb9643065a69",
}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bundle", nargs="?", type=Path,
                        default=Path(__file__).resolve().parents[2] / "tgt")
    bundle = parser.parse_args().bundle
    for name, digest in PINS.items():
        if sha256((bundle / name).read_bytes()).hexdigest() != digest:
            raise SystemExit(f"Rollback content mismatch: {name}")
        print(f"OK  {name}")
