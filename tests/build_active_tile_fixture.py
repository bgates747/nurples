"""Build only in scratch: compare production bytes and emit two song-free MOS probes."""
import argparse
from hashlib import sha256
from pathlib import Path
import shutil
import subprocess
from tempfile import TemporaryDirectory

from nurples_tools.active_tiles import REGISTRY, INCLUDE, parse, registry, render
from nurples_tools.campaign_audio import prepare_audio, verify_audio
from nurples_tools.loading_music import configure

ROOT = Path(__file__).resolve().parents[1]
BASE = "ee68670d6cd82dccf93a7289dd50bc1386b03383"


def replace_one(text, old, new):
    if text.count(old) != 1:
        raise ValueError(f"fixture needs exactly one anchor: {old!r}")
    return text.replace(old, new)


def build(assembler, output):
    registry(ROOT)
    output.mkdir(exist_ok=False)
    with TemporaryDirectory() as work:
        for kind in ("baseline", "production", "control", "alias"):
            asm = Path(work) / kind
            shutil.copytree(ROOT / "src/asm", asm, ignore=shutil.ignore_patterns("*.lst", "*.symbols", "*.bin"))
            if kind in ("baseline", "production"):
                modules = prepare_audio(ROOT, asm)
                configure(asm, candidate=True, small_reads=True, whole=True,
                          all_phases=True, combat_cues=True)
                if kind == "baseline":
                    old = subprocess.check_output(["git", "show", f"{BASE}:src/asm/tiles_active.inc"], cwd=ROOT)
                    (asm / "tiles_active.inc").write_bytes(old)
            else:
                spec = (ROOT / REGISTRY).read_text()
                if kind == "alias":
                    spec = replace_one(spec, '15 = ["15", "none"]', '15 = ["15", "electrode"]')
                (asm / INCLUDE.name).write_text(render(parse(spec)))
                main = asm / "nurples.asm"
                src = replace_one(main.read_text(), "    call main\n", "    call electrode_review\n")
                inject = (f"review_alias: equ {int(kind == 'alias')}\n"
                          '    include "active_electrode_review.inc"\n')
                main.write_text(replace_one(src, '    include "tables.inc"', inject + '    include "tables.inc"'))
                shutil.copy(ROOT / "tests/asm/active_electrode_review.inc", asm)
            subprocess.run([assembler, "nurples.asm", "check.bin"], cwd=asm, check=True, timeout=30)
            data = (asm / "check.bin").read_bytes()
            if kind in ("baseline", "production"):
                verify_audio(data, modules)
                if kind == "baseline":
                    reference = data
                elif data != reference:
                    raise ValueError("registry changed production machine code")
            else:
                (output / f"{kind}.bin").write_bytes(data)
            print(kind, len(data), sha256(data).hexdigest(), flush=True)


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("assembler", type=Path)
    ap.add_argument("new_output_directory", type=Path)
    args = ap.parse_args()
    build(args.assembler.resolve(strict=True), args.new_output_directory)
