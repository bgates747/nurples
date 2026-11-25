from hashlib import sha256
import json
from pathlib import Path
import shutil
from struct import pack
import subprocess
from tempfile import TemporaryDirectory
import unittest

from nurples_tools.campaign_audio import prepare_audio, verify_audio
from nurples_tools.loading_music import configure


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
        manifest.write_text('[[levels]]\nsource="../level-design/levels/recovered-z80/level.tmx"\n'
                            '[[levels]]\nsource="../level-design/levels/modern-dg/level.tmx"\n')

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

    @unittest.skipUnless(shutil.which("ez80asm"), "ez80asm is not on PATH")
    def test_ordinary_clock_in_linked_game(self):
        asm = Path(__file__).resolve().parents[1] / "src/asm"
        shutil.copytree(asm, self.stage, dirs_exist_ok=True)
        prepare_audio(self.root, self.stage, directory=self.songs)
        configure(self.stage, True, small_reads=True, whole=True, all_phases=True, combat_cues=True)
        subprocess.run(["ez80asm", "-s", "nurples.asm", "clock.bin"], cwd=self.stage,
                       stdout=subprocess.PIPE, check=True)
        game = (self.stage / "clock.bin").read_bytes()
        symbols = {name: int(value[1:], 16) for name, value in
                   (line.split() for line in (self.stage / "nurples.symbols").read_text()
                    .splitlines() if line.strip())}

        def section(start, end):
            return game[symbols[start]-0x40000:symbols[end]-0x40000]

        def call(name):
            return b"\xcd" + symbols[name].to_bytes(3, "little")

        # In the shipped path, preparation is the ONLY decoder call, and a
        # frame can only poll. There is no compiled free-running lp_frame.
        self.assertEqual(game.count(call("pt3_play")), 1)
        self.assertIn(call("pt3_play") + call("ay_update"), section("lp_prepare", "lp_send"))
        self.assertNotIn("lp_frame", symbols)
        self.assertEqual(section("mus_tick", "mus_effects"), call("lp_pump"))
        self.assertNotIn(call("lp_stop_timer"), section("lp_finish", "lp_poll"))
        self.assertIn(call("lp_stop_timer"), section("main_end", "mus_in_warp"))
        self.assertEqual(game.count(call("lp_poll")), 3)
        self.assertEqual(symbols["lp_record_end"] - symbols["lp_record"], 57)
        # The rocket still matches the soundboard's key 5.
        self.assertEqual(sha256(section("cues_rocket", "cues_rocket_end")).hexdigest(),
                         "2f0a9935d462d1e148f14a8c6fa880ee3d996052928af0a78566694b526ba81a")
        # Decode the intended audible shape independently of its assembly
        # layout: four pairs at the old speed, not a longer silent wait.
        header = bytes((23, 0, 133, 14))
        phases = []
        for i in range(8):
            body = 75 if i % 2 == 0 else 64
            phases.extend([(127, 4), (body, 146 if i < 7 else 156)])
            if i < 7:
                phases.append((0, 10))
        gate = sum(ms for _, ms in phases)
        tail = 176
        expected = (
            header + bytes((4, 1)) +
            header + bytes((6, 2, len(phases))) + b"".join(pack("<BH", *p) for p in phases) +
            bytes((0, 1)) + pack("<BH", 0, tail) +
            header + bytes((7, 1, 2, 5)) + pack("<HhHhH", 10, 32, 8, -32, 8) +
            header + bytes((0, 80)) + pack("<HH", 470, gate)
        )
        self.assertEqual(section("cues_klaxon", "cues_klaxon_end"), expected)
        self.assertEqual(gate + tail, 4 * (320 + 44))
        self.assertEqual(symbols["cues_alert_ticks"], ((gate + tail) * 120 + 999) // 1000 + 2)
        toggle = section("lp_sample_m", "lp_note_tick")
        # MOS KEY(102) is one-based: M = IX+12 bit 5, comma is bit 6.
        self.assertIn(bytes.fromhex("dd cb 0c 6e"), toggle)
        self.assertIn(bytes.fromhex("dd 7e 0c e6 df"), game[
            symbols["presentation_input_poll"]-0x40000:][:128])
        self.assertIn(bytes.fromhex("af ed 39 83 ed 38 83"), toggle)  # stop + acknowledge PRT1
        self.assertIn(bytes.fromhex("3e 5f ed 39 83"), toggle)       # restart from reload
        self.assertIn(call("input_gate_update"), toggle)
        self.assertIn(call("lp_prime_track"), toggle)
        self.assertIn(b"\xc0", section("lp_prime_track", "lp_rebase").split(call("pt3_init"))[0])
        off = b"\x3a" + symbols["lp_music_off"].to_bytes(3, "little")
        self.assertIn(off, section("lp_poll", "lp_prime_track"))
        self.assertTrue(game[symbols["lp_pump"]-0x40000:].startswith(off + b"\xb7\xc0"))
        spawn = section("enemy_spawn_fireball", "fireball_to_screen_vy")
        self.assertIn(call("spawn_sprite_from_tile") + b"\xd0", spawn)  # RET NC
        self.assertTrue(spawn.endswith(call("cues_queue_launch") + b"\x37\xc9"))  # SCF; RET: fired
        self.assertEqual(game.count(call("cues_queue_launch")), 1)
        self.assertIn(call("cues_cancel_warning"), section("player_init_respawn", "player_init"))
        self.assertIn(call("cues_cancel_warning"), section("vox_game_over", "vox_say"))
        for ch in (12, 13, 14):
            header = bytes((23, 0, 0x85, ch))
            self.assertIn(header + b"\x08", section("vox_open", "vox_close"))
            self.assertIn(header + b"\x09", section("vox_close", "vox_game_over"))
