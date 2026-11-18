#!/usr/bin/env python3
"""Stage a built Nurples production bundle in a stopped pinned Alexe profile.

This is deployment only. It never assembles the game, starts an emulator, or
replaces an existing profile. Run it on Oryx after independently building the
project tree and selecting a hash-qualified pinned Flexe template.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import shutil
import tempfile

from stage_hardware_overlay_oryx import (
    CARD_SIZE,
    RUNTIME_SHA256,
    copy_runtime,
    make_card_directory,
    run,
    sha256,
    verify_template,
)


GUEST_ROOT = "mystuff/nurples-dev/apps/nurples"
FIXED_FILES = (
    "nurples.bin",
    "nurples.symbols",
    "game.agnb",
    "ui.agnb",
)
AUTOEXEC = (
    b"SET KEYBOARD 1\r\n"
    b"cd /mystuff/nurples-dev/apps/nurples\r\n"
    b"load nurples.bin\r\n"
)


def require_regular(path: Path, label: str) -> Path:
    if path.is_symlink() or not path.is_file():
        raise SystemExit(f"Missing or non-regular {label}: {path}")
    if path.stat().st_size == 0:
        raise SystemExit(f"Empty {label}: {path}")
    return path


def verify_project_inputs(project: Path) -> tuple[list[Path], dict[str, str]]:
    target = project / "tgt"
    files = [require_regular(target / name, f"production input {name}") for name in FIXED_FILES]
    fonts = target / "fonts"
    if fonts.is_symlink() or not fonts.is_dir():
        raise SystemExit(f"Missing regular production font directory: {fonts}")
    font_files = sorted(path for path in fonts.rglob("*") if path.is_file())
    if not font_files:
        raise SystemExit(f"Production font directory is empty: {fonts}")
    if any(path.is_symlink() for path in font_files):
        raise SystemExit(f"Production font directory contains a symlink: {fonts}")
    files.extend(font_files)
    hashes = {
        path.relative_to(target).as_posix(): sha256(path)
        for path in files
    }
    return files, hashes


def build_card(
    project: Path,
    profile: Path,
    files: list[Path],
    expected_hashes: dict[str, str],
) -> None:
    target = project / "tgt"
    card = profile / "card.img"
    with card.open("xb") as stream:
        stream.truncate(CARD_SIZE)
    run("mkfs.fat", "-F", "16", str(card))
    make_card_directory(card, GUEST_ROOT)

    for source in files:
        relative = source.relative_to(target)
        parent = (Path(GUEST_ROOT) / relative.parent).as_posix()
        make_card_directory(card, parent)
        run(
            "mcopy",
            "-i",
            str(card),
            str(source),
            f"::/{GUEST_ROOT}/{relative.as_posix()}",
        )

    autoexec = profile / "autoexec.txt"
    autoexec.write_bytes(AUTOEXEC)
    run("mcopy", "-i", str(card), str(autoexec), "::/autoexec.txt")
    (profile / "sdcard.img").symlink_to("card.img")
    verify_card(card, expected_hashes)


def verify_card(card: Path, expected_hashes: dict[str, str]) -> None:
    with tempfile.TemporaryDirectory(prefix="nurples-card-readback-") as temp_dir:
        root = Path(temp_dir)
        for relative, expected in expected_hashes.items():
            output = root / Path(relative).name
            run(
                "mcopy",
                "-i",
                str(card),
                f"::/{GUEST_ROOT}/{relative}",
                str(output),
            )
            if sha256(output) != expected:
                raise SystemExit(f"Card readback hash mismatch: {relative}")
        autoexec = root / "autoexec.txt"
        run("mcopy", "-i", str(card), "::/autoexec.txt", str(autoexec))
        if autoexec.read_bytes() != AUTOEXEC:
            raise SystemExit("Card autoexec readback mismatch")


def write_identity(
    profile: Path,
    template: Path,
    project: Path,
    source_hashes: dict[str, str],
) -> None:
    identity = {
        "schema": 1,
        "project": "nurples-dev",
        "fixture": "GAME-001JK-PRODUCTION-INTEGRATION-R01",
        "template": str(template),
        "source_project": str(project),
        "guest_root": f"/{GUEST_ROOT}",
        "autoexec_runs_game": False,
        "source_sha256": source_hashes,
        "runtime_sha256": RUNTIME_SHA256,
        "card_sha256": sha256(profile / "card.img"),
        "stopped": True,
    }
    (profile / "identity.json").write_text(
        json.dumps(identity, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    run("sha256sum", "--check", "--status", ".runtime.sha256", cwd=profile)
    run("sha256sum", "--check", "--status", ".mos-release.sha256", cwd=profile)
    run("bash", "-n", "flexe-agon-emulator", cwd=profile)
    run("mdir", "-i", str(profile / "card.img"), f"::/{GUEST_ROOT}")


def stage(project: Path, template: Path, profile: Path) -> Path:
    project = project.expanduser().resolve(strict=True)
    template = template.expanduser().resolve(strict=True)
    profile = profile.expanduser().resolve(strict=False)
    if profile.exists() or profile.is_symlink():
        raise SystemExit(f"Refusing to replace existing profile: {profile}")
    verify_template(template)
    files, hashes = verify_project_inputs(project)
    profile.parent.mkdir(parents=True, exist_ok=True)
    staging = Path(tempfile.mkdtemp(prefix=f".{profile.name}-", dir=profile.parent))
    try:
        copy_runtime(template, staging)
        build_card(project, staging, files, hashes)
        write_identity(staging, template, project, hashes)
        os.replace(staging, profile)
    except BaseException:
        shutil.rmtree(staging, ignore_errors=True)
        raise
    return profile


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project", type=Path, required=True)
    parser.add_argument("--template", type=Path, required=True)
    parser.add_argument("--profile", type=Path, required=True)
    args = parser.parse_args()
    profile = stage(args.project, args.template, args.profile)
    print(f"Stopped production profile: {profile}")
    print(f"Author launch: cd {profile} && ./flexe-agon-emulator")


if __name__ == "__main__":
    main()
