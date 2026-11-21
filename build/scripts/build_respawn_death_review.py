"""Build the isolated full-game D-to-die FAE bundle. Never deploy or launch."""

import argparse
import hashlib
from pathlib import Path
import shutil
import subprocess
from tempfile import TemporaryDirectory

from build_game_over_review import ASSETS, ROOT


def redirect(source: Path, filename: str, old: str, new: str) -> None:
    path = source / filename
    text = path.read_text()
    label = f"\n{old}:\n"
    if text.count(label) != 1:
        raise ValueError(f"Expected one {old} entry in {filename}")
    path.write_text(text.replace(label, f"\n{new}:\n"))


def build(assembler: Path, output: Path) -> None:
    with TemporaryDirectory(prefix="npl-dkey-") as temporary:
        source = Path(temporary) / "asm"
        shutil.copytree(ROOT / "src/asm", source)

        def assemble() -> bytes:
            subprocess.run([str(assembler), "-s", "nurples.asm", "n.bin"],
                           cwd=source, check=True)
            return (source / "n.bin").read_bytes()

        # The D-key fixture is intentionally independent of private campaign music.
        control = assemble()
        redirect(source, "player_shields.inc", "update_shields", "ordinary_update_shields")
        redirect(source, "game_pause.inc", "game_pause_poll", "ordinary_pause_poll")
        with (source / "game_pause.inc").open("a") as stream:
            stream.write('\n    include "respawn_death_review.inc"\n')
        shutil.copyfile(ROOT / "tests/asm/respawn_death_review.inc", source / "respawn_death_review.inc")
        fixture = assemble()
        if fixture == control:
            raise RuntimeError("D-key hooks did not change the ordinary song-free assembly")
        output.mkdir(parents=True, exist_ok=False)
        (output / "nurples.bin").write_bytes(fixture)
        shutil.copyfile(source / "nurples.symbols", output / "nurples.symbols")
        for name in ASSETS:
            dest = output / name
            dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / "tgt" / name, dest)
        (output / "SHA256SUMS").write_text("".join(
            f"{hashlib.sha256((output / name).read_bytes()).hexdigest()}  {name}\n"
            for name in ("nurples.bin", "nurples.symbols", *ASSETS)
        ))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--assembler", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    args = parser.parse_args()
    build(args.assembler.resolve(strict=True), args.output_dir.resolve())
