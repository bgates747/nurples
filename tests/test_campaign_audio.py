from hashlib import sha256
import json
from pathlib import Path
import shutil
import subprocess
from tempfile import TemporaryDirectory
import unittest

from nurples_tools.campaign_audio import prepare_audio, verify_audio


class CampaignAudioTests(unittest.TestCase):
    def setUp(self):
        temp = TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        self.stage = self.root / "stage"
        self.stage.mkdir()
        self.songs = self.root / "songs"
        self.songs.mkdir()
        pins = self.root / "docs/tasks/MUS-001/samples.json"
        pins.parent.mkdir(parents=True)
        names = ("terminator.pt3", "bugs2_2.pt3", "01_loadtune.pt3")
        # Synthetic payloads only; these tests run in a clean public checkout.
        metadata = []
        for name in names:
            data = name.encode("ascii")
            (self.songs / name).write_bytes(data)
            metadata.append(dict(filename=name, bytes=len(data), sha256=sha256(data).hexdigest()))
        pins.write_text(json.dumps(metadata))
        manifest = self.root / "maps/campaign.toml"
        manifest.parent.mkdir()
        manifest.write_text('[[levels]]\nsource="../tiles/dg/nurples_z80_main.tmx"\n'
                            '[[levels]]\nsource="../tiles/dg/dg0_03.tmx"\n')

    def test_manifest_order_and_loadtune(self):
        modules = prepare_audio(self.root, self.stage, directory=self.songs)
        self.assertEqual(modules, [b"bugs2_2.pt3", b"terminator.pt3", b"01_loadtune.pt3"])
        table = (self.stage / "mus_tracks.inc").read_text()
        self.assertIn("mus_tracks:\n    dl mus_track_0\n    dl mus_track_1\nmus_song:", table)
        self.assertIn("mus_loadtune: equ mus_track_2", table)
        verify_audio(b"".join(modules), modules)
        for invalid in [modules[:-1], modules + modules[:1]]:
            with self.assertRaises(ValueError):
                verify_audio(b"".join(invalid), modules)

    def test_rejects_missing_or_substituted_module(self):
        track = self.songs / "bugs2_2.pt3"
        track.unlink()
        with self.assertRaisesRegex(FileNotFoundError, "NURPLES_MUSIC_DIR"):
            prepare_audio(self.root, self.stage, directory=self.songs)
        track.write_bytes(b"X" * len(track.name))
        with self.assertRaisesRegex(ValueError, "bugs2_2.pt3"):
            prepare_audio(self.root, self.stage, directory=self.songs)
        self.assertFalse((self.stage / "mus_payload.inc").exists())

    @unittest.skipUnless(shutil.which("ez80asm"), "ez80asm is not on PATH")
    def test_campaign_hooks_link_without_shipping_the_soundtrack(self):
        asm = Path(__file__).resolve().parents[1] / "src/asm"
        shutil.copytree(asm, self.stage, dirs_exist_ok=True,
                        ignore=shutil.ignore_patterns("*.lst", "*.symbols"))
        modules = prepare_audio(self.root, self.stage, directory=self.songs)
        subprocess.run(["ez80asm", "nurples.asm", "synthetic.bin"], cwd=self.stage,
                       stdout=subprocess.PIPE, check=True)
        binary = (self.stage / "synthetic.bin").read_bytes()
        verify_audio(binary, modules)
        # Two ADL pointers in manifest order immediately precede the tracks.
        start = binary.index(modules[0])
        addresses = [0x040000 + binary.index(module) for module in modules[:2]]
        self.assertEqual(binary[start-6:start],
                         b"".join(a.to_bytes(3, "little") for a in addresses))
