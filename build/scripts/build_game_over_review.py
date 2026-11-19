#!/usr/bin/env python3
"""Build isolated full-game Game Over review bundles, without deploying."""

import argparse
import hashlib
from pathlib import Path
import shutil
import subprocess
from tempfile import TemporaryDirectory


ROOT = Path(__file__).resolve().parents[2]
VARIANTS = {
    "immediate-frozen": (1, 0),
    "seekers-frozen": (2, 0),
    "seekers-scrolling": (2, 1),
}
ASSETS = ("game.agnb", "ui.agnb", "fonts/Lat38-VGA8_8x8.font")


def assemble(assembler: Path, source: Path, binary: Path) -> None:
    subprocess.run(
        [str(assembler), "nurples.asm", str(binary), "-c"],
        cwd=source,
        check=True,
    )


def build(assembler: Path, output: Path) -> None:
    output.mkdir(parents=True, exist_ok=False)
    with TemporaryDirectory(prefix="nurples-game-over-") as temp:
        source = Path(temp) / "asm"
        shutil.copytree(ROOT / "src" / "asm", source)
        config = source / "game_over_review_config.inc"
        ordinary_config = config.read_bytes()
        defaults = [line for line in ordinary_config.splitlines() if not line.startswith(b";")]
        if defaults != [b"GAME_OVER_REVIEW_TRIGGER: equ 0", b"GAME_OVER_REVIEW_SCROLL: equ 0"]:
            raise RuntimeError("ordinary builds must keep both review settings disabled")
        variant_binary = Path(temp) / "review.bin"
        ordinary = Path(temp) / "ordinary.bin"
        assemble(assembler, ROOT / "src" / "asm", ordinary)
        for name, (trigger, scrolling) in VARIANTS.items():
            bundle = output / name
            bundle.mkdir()
            config.write_text(
                f"GAME_OVER_REVIEW_TRIGGER: equ {trigger}\n"
                f"GAME_OVER_REVIEW_SCROLL: equ {scrolling}\n"
            )
            assemble(assembler, source, variant_binary)
            shutil.copyfile(variant_binary, bundle / "nurples.bin")
            for relative in ASSETS:
                asset = ROOT / "tgt" / relative
                target = bundle / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(asset, target)
            pins = [
                f"{hashlib.sha256((bundle / relative).read_bytes()).hexdigest()}  {relative}"
                for relative in ("nurples.bin", *ASSETS)
            ]
            (bundle / "SHA256SUMS").write_text("\n".join(pins) + "\n")
        config.write_bytes(ordinary_config)
        restored = Path(temp) / "restored.bin"
        assemble(assembler, source, restored)
        if ordinary.read_bytes() != restored.read_bytes():
            raise RuntimeError("restored ordinary build differs from the project build")
        if config.read_bytes() != (ROOT / "src" / "asm" / config.name).read_bytes():
            raise RuntimeError("project review settings changed during the build")
        shutil.copyfile(ordinary, output / "ordinary.bin")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--assembler", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    args = parser.parse_args()
    build(args.assembler.resolve(strict=True), args.output_dir.resolve())


if __name__ == "__main__":
    main()
