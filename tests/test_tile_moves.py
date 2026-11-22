"""Exercise Git staging and both build-time translations on a disposable checkout."""

import os
import shutil
import subprocess
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path

from PIL import Image

from nurples_tools.legacy_level import render_tmx
from nurples_tools.active_tiles import REGISTRY, INCLUDE
from nurples_tools.legacy_tiles import check_legacy_tiles
from nurples_tools.maps import compile_map_manifest
from nurples_tools.tile_bindings import bindings
from nurples_tools.tile_moves import CAT, CSV, PNG, TSX, git, tracked

ROOT = Path(__file__).resolve().parents[1]
MANIFESTS = ("maps/campaign.toml", "tests/maps/dg/suite.toml")


class TileMoveTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.root = Path(tmp.name)
        for p in (*tracked(ROOT, "*.tmx", "tiles/dg/legacy_z80/*.png"),
                  TSX, PNG, CAT, CSV, REGISTRY, INCLUDE, *map(Path, MANIFESTS)):
            (self.root / p).parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / p, self.root / p)
        git(self.root, "init", "-q", "-b", "dev")
        git(self.root, "add", ".")
        self.commit()

    def commit(self):
        git(self.root, "-c", "user.name=Test", "-c", "user.email=test@invalid",
            "-c", "commit.gpgSign=false", "commit", "-qm", "checkpoint")

    def cli(self, *args, ok=True):
        p = subprocess.run([sys.executable, "-m", "nurples_tools.tile_moves", *map(str, args)],
                           env={**os.environ, "PYTHONPATH": str(ROOT)}, cwd=self.root,
                           capture_output=True, text=True)
        if ok:
            self.assertEqual(p.returncode, 0, p.stderr)
            return p.stdout
        self.assertNotEqual(p.returncode, 0)
        return p.stderr

    def runtime_ids(self):
        return bindings(ET.parse(self.root / TSX).getroot())

    def slot(self, runtime):
        return self.runtime_ids().index(runtime) - 1

    def art(self, slot):
        x, y = (slot % 16) * 16, (slot // 16) * 16
        with Image.open(self.root / PNG) as im:
            return im.crop((x, y, x + 16, y + 16)).tobytes()

    def compiled(self):
        return [compile_map_manifest(self.root / p).text for p in MANIFESTS]

    def cells(self):
        ids = self.runtime_ids()
        return {p: bytes(ids[int(i)] for i in ET.parse(self.root / p).findtext("layer/data").split(","))
                for p in tracked(self.root, "*.tmx")}

    def test_move_then_swap_on_full_map_set(self):
        before = self.compiled(), self.cells()
        a, b, c = (self.slot(runtime) for runtime in (1, 25, 17))
        original = [self.art(slot) for slot in (a, b, c)]
        for op, src, dst in [("move", a, b), ("swap", b, c)]:
            token = self.cli(op, src, dst).split()[-1]
            self.assertEqual(git(self.root, "status", "--porcelain"), b"")
            self.assertIn("--reviewed-active required", self.cli(op, src, dst, "--apply", token, ok=False))
            self.cli(op, src, dst, "--reviewed-active", "--apply", token)
            self.assertTrue(git(self.root, "diff", "--cached", "--stat"))
            self.assertEqual((self.compiled(), self.cells()), before)
            self.commit()
        self.assertEqual([self.art(i) for i in (a, b, c)], [original[1], original[2], original[0]])
        self.assertEqual((self.slot(1), self.slot(17)), (c, b))
        legacy = compile_map_manifest(self.root / MANIFESTS[0]).levels[1]
        self.assertEqual(render_tmx(legacy.runtime_rows, self.runtime_ids()),
                         legacy.source_path.read_text())
        # No runtime tile files were copied here. Missing files are expected;
        # an atlas regenerated at the wrong slots would be reported as changed.
        self.assertNotIn(f"changed {PNG}", check_legacy_tiles(self.root))

    def test_fail_closed(self):
        a, b, c = (self.slot(runtime) for runtime in (1, 25, 5))
        for op, src, dst in [("move", a, c), ("swap", a, b),
                             ("move", b, a), ("move", a, a), ("move", -1, b), ("swap", a, 255)]:
            self.cli(op, src, dst, ok=False)
        token = self.cli("move", a, b).split()[-1]
        self.assertIn("stale", self.cli("swap", a, c, "--apply", token, ok=False))
        self.assertEqual(git(self.root, "status", "--porcelain"), b"")
        p = self.root / "tiles/dg/dg0_00.tmx"
        p.write_text(p.read_text().replace('encoding="csv"', 'encoding="base64"'))
        self.assertIn("clean", self.cli("move", a, b, "--apply", token, ok=False))
        git(self.root, "add", "--", p)
        self.commit()
        self.assertIn("CSV", self.cli("move", a, b, ok=False))

    def test_reserved_empty_destination_requires_manual_review(self):
        source, dest = self.slot(17), self.slot(15)
        report = self.cli("move", source, dest)
        self.assertNotIn("active-range source", report)
        self.assertIn("active-range destination", report)
        self.assertIn("activate_null_tile, actor none", report)
        token = report.split()[-1]
        self.assertIn("--reviewed-active required", self.cli("move", source, dest, "--apply", token, ok=False))
        self.assertEqual(git(self.root, "status", "--porcelain"), b"")
        self.assertIn("occupied", self.cli("move", source, self.slot(11), ok=False))

    def test_registry_drift_refuses_preview(self):
        p = self.root / REGISTRY
        p.write_text(p.read_text().replace('15 = ["15", "none"]', '15 = ["15", "electrode"]'))
        git(self.root, "add", "--", REGISTRY)
        self.commit()
        self.assertIn("stale", self.cli("move", self.slot(17), self.slot(15), ok=False))
