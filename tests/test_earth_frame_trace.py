import unittest
from struct import pack

from nurples_tools.earth_frame_trace import summarize


def report():
    # Two missed VBlanks on one refill; zero elapsed for the third update.
    rows = [(0, 1, 0), (144, 3, 3), (0, 0, 1)]
    fields = (72, 4, 3, 1, 2, 0, 0, 144, 145)
    return (b"EFT1" + b"".join(n.to_bytes(3, "little") for n in fields) + b"\1\0"
            + b"".join(pack("<HBB", *row) for row in rows))


class FrameTraceTests(unittest.TestCase):
    def test_reconciles_and_separates_refills(self):
        result = summarize(report(), 72)
        self.assertEqual(result["refill"]["updates_missing_vblank"], 1)
        self.assertEqual(result["refill"]["skipped"], 2)
        self.assertEqual(result["refill"]["ms"]["max"], 2)
        self.assertEqual(result["refill"]["invalid_or_clipped"], 1)
        self.assertEqual(result["no_refill"]["skipped"], 0)

    def test_rejects_truncation_and_inconsistent_counters(self):
        for blob in (report()[:-1], report()[:7] + bytes(3) + report()[10:],
                     report()[:32] + b"\1" + report()[33:]):
            with self.assertRaises(ValueError):
                summarize(blob, 72)
