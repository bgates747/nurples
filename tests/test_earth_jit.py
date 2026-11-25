"""Check the complete audition map against the independently parsed preload control."""
from pathlib import Path
import unittest

from nurples_tools.agnb_verify import parse_images
from nurples_tools.earth_jit import pack


class EarthJitTests(unittest.TestCase):
    def test_reconstructs_control_without_changing_other_assets(self):
        control = (Path(__file__).resolve().parents[1] / "tgt/game.agnb").read_bytes()
        original = {r.buffer_id: r for r in parse_images(control)}
        bank, disk, count = pack(control)
        resident = {r.buffer_id: r for r in parse_images(bank)}
        stream, = parse_images(disk)
        self.assertEqual(len(resident), count)
        self.assertEqual((stream.buffer_id, stream.width, stream.height), (1280, 256, 3584))
        self.assertEqual((resident[1280].width, resident[1280].height), (256, 1))
        self.assertEqual(set(original) - set(resident), set(range(1281, 1294)))
        for n in resident.keys() - {1280}:
            self.assertEqual(resident[n], original[n])
        for line in range(4096):
            strip, offset = divmod(line, 256)
            start = (255-offset)*256
            wanted = original[1280+strip].payload[start:start+256]
            if line < 3584:
                actual = stream.payload[line*256:(line+1)*256]
            else:
                actual = resident[1280+strip].payload[start:start+256]
            self.assertEqual(actual, wanted, f"flight row {line}")
