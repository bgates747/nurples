#!/usr/bin/env python3
"""Stage the standalone hardware-overlay bundle in a stopped Alexe profile.

This is deployment only. It never builds the fixture, starts an emulator, or
modifies an existing profile. Run it on Oryx from an already verified bundle.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


CARD_SIZE = 32 * 1024 * 1024
GUEST_ROOT = "apps/nurples-overlay"
RUNTIME_SHA256 = {
    "flexe-agon-emulator.bin": (
        "bad83f5455ab8d03436e8b7be68960110b318ef636dc60613e5215eb2dc37dda"
    ),
    "flexe-vdp.so": (
        "d77a7305b5c180381e49a4922ffd5a8ea9f824fee29110d5a955f337a1c96f0d"
    ),
    "esp32-firmware.bin": (
        "dc43ecb8b1c324ffd1a032e6cc0c56d8c05dd33a355ce170c23adb35ffb533ab"
    ),
    "esp32-firmware.elf": (
        "576b69f3a0a4fbb676b2108b06437d1ae6d0b459fa7976c8508a4815e55f0a40"
    ),
    "mos_platform.bin": (
        "d7a51147f3381cd68d56f3fbe4605434a897dfae1b049269afeba09ec2b7eb50"
    ),
    "mos_platform.map": (
        "48dae5890af7b49fe6bcb535a20e9b6432069c8d84666e3d0e8e7e1070282be9"
    ),
    ".bespoke-vdp-profile": (
        "3016ae6e37a83c44f8f0b1ce533c4b98e23b48e1c1639ed41919b90ea05db1cb"
    ),
    ".mos-release": (
        "b48b24cb3618502342e624965658885d12f8c72b4ad3808a63f003f02abaab05"
    ),
    "flexe-agon-emulator": (
        "cc78b5c8cac56adc662dc5f1beec84a6851052387cf3b52da690d159ee686193"
    ),
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def run(*command: str, cwd: Path | None = None) -> None:
    subprocess.run(command, cwd=cwd, check=True)


def require_hash(path: Path, expected: str, label: str) -> None:
    if not path.is_file():
        raise SystemExit(f"Missing {label}: {path}")
    actual = sha256(path)
    if actual != expected:
        raise SystemExit(
            f"{label} hash mismatch for {path}: expected {expected}, got {actual}"
        )


def verify_template(template: Path) -> None:
    for name, expected in RUNTIME_SHA256.items():
        require_hash(template / name, expected, f"pinned runtime {name}")
    for manifest in (".runtime.sha256", ".mos-release.sha256"):
        if not (template / manifest).is_file():
            raise SystemExit(f"Missing pinned runtime manifest: {template / manifest}")
    run("sha256sum", "--check", "--status", ".runtime.sha256", cwd=template)
    run("sha256sum", "--check", "--status", ".mos-release.sha256", cwd=template)
    run("bash", "-n", "flexe-agon-emulator", cwd=template)


def verify_bundle(bundle: Path) -> dict[str, str]:
    manifest = bundle / "SHA256SUMS"
    if not manifest.is_file():
        raise SystemExit(f"Missing bundle manifest: {manifest}")
    expected_names = {
        "overlay.bin",
        "game_over_0.rgba2",
        "game_over_1.rgba2",
        "press_any_key_to_continue.rgba2",
        "press_any_key_to_start.rgba2",
        "autoexec.txt",
    }
    hashes: dict[str, str] = {}
    for line in manifest.read_text(encoding="utf-8").splitlines():
        expected, name = line.split("  ", 1)
        if name in hashes or name not in expected_names:
            raise SystemExit(f"Unexpected or duplicate bundle entry: {name}")
        require_hash(bundle / name, expected, f"bundle input {name}")
        hashes[name] = expected
    if set(hashes) != expected_names:
        raise SystemExit(
            f"Bundle file set mismatch: saw {sorted(hashes)}, "
            f"expected {sorted(expected_names)}"
        )
    return hashes


def make_card_directory(card: Path, relative: str) -> None:
    current: list[str] = []
    for part in Path(relative).parts:
        current.append(part)
        destination = "::/" + "/".join(current)
        probe = subprocess.run(
            ["mdir", "-i", str(card), destination],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False,
        )
        if probe.returncode != 0:
            run("mmd", "-i", str(card), destination)


def copy_runtime(template: Path, profile: Path) -> None:
    for name in RUNTIME_SHA256:
        shutil.copy2(template / name, profile / name, follow_symlinks=True)
    shutil.copy2(template / ".runtime.sha256", profile / ".runtime.sha256")
    shutil.copy2(template / ".mos-release.sha256", profile / ".mos-release.sha256")
    (profile / "sdcard").mkdir()


def build_card(bundle: Path, profile: Path) -> None:
    card = profile / "card.img"
    with card.open("xb") as stream:
        stream.truncate(CARD_SIZE)
    run("mkfs.fat", "-F", "16", str(card))
    make_card_directory(card, GUEST_ROOT)
    for name in (
        "overlay.bin",
        "game_over_0.rgba2",
        "game_over_1.rgba2",
        "press_any_key_to_continue.rgba2",
        "press_any_key_to_start.rgba2",
    ):
        run(
            "mcopy",
            "-i",
            str(card),
            str(bundle / name),
            f"::/{GUEST_ROOT}/{name}",
        )
    run("mcopy", "-i", str(card), str(bundle / "autoexec.txt"), "::/autoexec.txt")
    (profile / "sdcard.img").symlink_to("card.img")


def write_identity(
    profile: Path,
    template: Path,
    bundle: Path,
    bundle_hashes: dict[str, str],
    fixture: str,
) -> None:
    identity = {
        "schema": 1,
        "project": "nurples-dev",
        "fixture": fixture,
        "template": str(template),
        "bundle": str(bundle),
        "guest_root": f"/{GUEST_ROOT}",
        "autoexec_runs_fixture": False,
        "bundle_sha256": bundle_hashes,
        "card_sha256": sha256(profile / "card.img"),
        "stopped": True,
    }
    (profile / "identity.json").write_text(
        json.dumps(identity, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    run("sha256sum", "--check", "--status", ".runtime.sha256", cwd=profile)
    run("sha256sum", "--check", "--status", ".mos-release.sha256", cwd=profile)
    run("bash", "-n", "flexe-agon-emulator", cwd=profile)
    run("mdir", "-i", str(profile / "card.img"), f"::/{GUEST_ROOT}")


def stage(bundle: Path, template: Path, profile: Path, fixture: str) -> Path:
    bundle = bundle.expanduser().resolve(strict=True)
    template = template.expanduser().resolve(strict=True)
    profile = profile.expanduser().resolve(strict=False)
    if profile.exists() or profile.is_symlink():
        raise SystemExit(f"Refusing to replace existing profile: {profile}")
    verify_template(template)
    bundle_hashes = verify_bundle(bundle)
    profile.parent.mkdir(parents=True, exist_ok=True)
    staging = Path(tempfile.mkdtemp(prefix=f".{profile.name}-", dir=profile.parent))
    try:
        copy_runtime(template, staging)
        build_card(bundle, staging)
        write_identity(staging, template, bundle, bundle_hashes, fixture)
        os.replace(staging, profile)
    except BaseException:
        shutil.rmtree(staging, ignore_errors=True)
        raise
    return profile


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bundle", type=Path, required=True)
    parser.add_argument("--template", type=Path, required=True)
    parser.add_argument("--profile", type=Path, required=True)
    parser.add_argument("--fixture", required=True)
    args = parser.parse_args()
    profile = stage(args.bundle, args.template, args.profile, args.fixture)
    print(f"Stopped hardware-overlay profile: {profile}")
    print(f"Author launch: cd {profile} && ./flexe-agon-emulator")


if __name__ == "__main__":
    main()
