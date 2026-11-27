import struct
import unittest

from nurples_tools.earth_pose import pose_summary
from nurples_tools.pose_cost import ARMS, CAP, FIRST, SIZE, STRIDE, decode_cost
from nurples_tools.pose_compare import describe, tables


def report(arm, *, short=False, invalid=False):
    rows = bytearray()
    n = 4543 if short else 4544
    for i in range(n):
        p = 12 if i % 100 == 0 else 0
        flags = 3 if i < 895 else 0
        if invalid and i == 1:
            flags = 1  # refill occurred, but timer reading unusable
        rows.extend(struct.pack("<HBBBB", 72 if flags else 0, 1 + (i == 0), flags,
                                p, p * 3 if arm in ("direct", "native-direct") else int(bool(p))))
    header = (72, n + 1, n, 0, 1, 0, 0, 72, 0)
    return b"EPS1" + b"".join(x.to_bytes(3, "little") for x in header) + bytes([1, 0]) + rows


class EarthPoseReport(unittest.TestCase):
    def test_sparse_cost_observer(self):
        for arm in ARMS:
            base = bytearray(report(arm))
            samples = bytearray()
            for k, frame in enumerate(range(FIRST, 4545, STRIDE)):
                # Force two poses on each observed frame (other dense frames remain).
                start = 33+6*(frame-1)+4
                base[start:start+2] = bytes((2, 6 if arm == "direct" else 1))
                tag = 1+2*(k % 3)
                payload = 30 if arm == "sync" else 12
                wire = [4, 4, 7]*2 if arm == "direct" else [30 if arm == "sync" else 38]
                for t, spans in ((7, [0]), (tag, {1: [0, 0], 3: [] if arm == "direct" else [payload],
                                                  5: wire}[tag])):
                    for size in spans:
                        # Across a PRT epoch AND the 24-bit IRQ-counter wrap.
                        samples.extend(struct.pack("<BHBHHH", t, frame, 255, 65535, 4, size))
                        samples.extend(struct.pack("<BHBHHH", t+1, frame, 0, 0, 1439, 0))
            header = b"EPT1" + base[4:33] + bytes((ARMS.index(arm), 0)) + len(samples).to_bytes(3, "little")
            data = bytearray(header + samples.ljust(CAP*SIZE, b"\0") + base[33:])
            stats = decode_cost(data, arm)
            self.assertEqual(stats["empty_pair:0"]["n"], 75)
            self.assertEqual(stats["publish:0"]["median_counts"], 5)
            with self.assertRaises(ValueError):
                decode_cost(data[:-1], arm)
            wrong = data.copy()
            wrong[34] = 2
            with self.assertRaises(ValueError):
                decode_cost(wrong, arm)
            wrong = data.copy()
            wrong[38+1] += 1
            with self.assertRaisesRegex(ValueError, "reconcile"):
                decode_cost(wrong, arm)
            wrong = data.copy()
            wrong[38+SIZE+3:38+SIZE+6] = b"\xff"*3
            self.assertEqual(decode_cost(wrong, arm)["empty_pair:0"]["invalid"], 1)
            wrong = data.copy()
            # Smaller but otherwise legal lengths must not pass frame reconciliation.
            site = next(i for i in range(38, 38+len(samples), SIZE) if wrong[i] == 5)
            struct.pack_into("<H", wrong, site+8, {"direct": 7, "sync": 15, "golem": 32}[arm])
            with self.assertRaisesRegex(ValueError, "lengths"):
                decode_cost(wrong, arm)

    def test_measured_totals(self):
        for arm in ("direct", "sync", "async", "golem", "native-direct", "native"):
            with self.subTest(arm=arm):
                data = pose_summary(report(arm), arm)
                self.assertEqual((data["updates"], data["vblanks"], data["skipped"]), (4544, 4545, 1))
                self.assertEqual(data["poses"], 46 * 12)
                group_bytes = {"golem": 98, "native": 101}.get(arm, 180)
                self.assertEqual(data["pose_wire_bytes"], group_bytes * 46)
                self.assertEqual(data["pose_sends"], 46 * (36 if arm in ("direct", "native-direct") else 1))
                self.assertEqual(data["read_ms"]["mean"], 1)
                self.assertEqual(data["read_ms"]["sd"], 0)
                self.assertEqual(data["at_least_eight_poses"]["skipped"], 1)

    def test_bad_trace_is_not_a_result(self):
        for raw in (report("sync", short=True), report("direct"), report("sync")[:-1]):
            with self.assertRaises(ValueError):
                pose_summary(raw, "sync")

    def test_invalid_refill_not_silently_in_distribution(self):
        r = pose_summary(report("sync", invalid=True), "sync")
        self.assertEqual(r["refill"]["invalid_or_clipped"], 1)
        self.assertEqual(r["read_ms"]["n"], 894)

    def test_matched_work_and_rendering_envelope(self):
        direct = describe(report("direct"), "direct")
        sync = describe(report("sync"), "sync")
        self.assertEqual(direct["workload"], sync["workload"])
        self.assertEqual(sync["updates_by_sends"], {0: 4498, 1: 46})
        self.assertIn("matched on every update", tables(dict(direct=direct, sync=sync)))
        changed = bytearray(report("sync"))
        changed[33+4:33+6], changed[39+4:39+6] = changed[39+4:39+6], changed[33+4:33+6]
        alternate = describe(changed, "sync")
        self.assertEqual(alternate["poses"], sync["poses"])
        self.assertIn("differs by update", tables(dict(direct=direct, sync=alternate)))
        with self.assertRaisesRegex(ValueError, "mirror-off"):
            tables(dict(direct=direct, native=sync))
