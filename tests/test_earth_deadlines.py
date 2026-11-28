import struct
import unittest

from nurples_tools.earth_deadlines import cohort, summarize_deadlines


def report():
    rows = []
    for i in range(4544):
        t = 5 if i == 0 else int(i != 1)
        poses = 8 if i in (0, 1) else 7
        refill = i < 895
        edges = (6, 8, 9, 1) if i == 0 else ((10, 0, 0, 0) if t else (0, 0, 0, 0))
        rows.append(struct.pack("<H8B", 72 if refill else 0, t, 3 if refill else 0, poses, poses*3, *edges))
    header = b"".join(i.to_bytes(3, "little") for i in (72, 4547, 4544, 1, 4, 1, 0, 72, 0)) + bytes((1, 0))
    return b"EDL1"+header+b"".join(rows), b"EPS1"+header+b"".join(r[:6] for r in rows)


class DeadlineTest(unittest.TestCase):
    def test_matched_trace_and_unresolved_tail(self):
        data, control = report()
        s = summarize_deadlines(data, control, list_dense_misses=True)
        dense = s["dense"]
        self.assertEqual((dense["updates"], dense["vblanks"], dense["skipped"], dense["zero_tick"]), (2, 5, 4, 1))
        self.assertEqual(dense["additional_edges"], dict(actor_work=1, pose_send=1, unresolved=2))
        self.assertEqual(dense["first_edge_on_frames_with_skips"], dict(background_upload=1))
        self.assertEqual(s["light"]["first_edge"], dict(outer_wait=4542))
        self.assertEqual(s["dense_misses"][0]["update"], 1)
        self.assertEqual(s["dense_misses"][0]["skipped_in"], ["actor_work", "pose_send", "unresolved", "unresolved"])
        self.assertEqual(s["uninstrumented_control"]["skipped"], 4)
        self.assertEqual(s["all"]["skips_per_elapsed_second_60hz"], 60*4/4547)

    def test_bad_or_incomplete_input_and_control_mismatch(self):
        data, control = report()
        for index, value in ((33+6, 11), (33+7, 0), (43+9, 1), (53+7, 7), (33+9, 4), (32, 1), (16, 3)):
            with self.subTest(index=index), self.assertRaises(ValueError):
                summarize_deadlines(data[:index]+bytes((value,))+data[index+1:])
        with self.assertRaises(ValueError):
            summarize_deadlines(data[:-1])
        changed = bytearray(control)
        changed[33+4:33+6] = bytes((9, 27))
        with self.assertRaisesRegex(ValueError, "workload differs"):
            summarize_deadlines(data, changed)

    def test_first_edge_without_skip_and_invalid_subsamples(self):
        # Forced fresh waits may see multiple edges and need not be pending.
        sample = cohort([(0, 1, 0, 9, 27, 9, 0, 0, 1), (0, 2, 0, 4, 12, 0, 0, 0, 2)])
        self.assertEqual(sample["first_edge"], dict(pose_send=1, unresolved=1))
        self.assertEqual(sample["additional_edges"], dict(unresolved=1))
        self.assertEqual((sample["pending_at_claim"], sample["fresh_wait_path"]), (1, 1))
