import os
import tempfile
import unittest
from pathlib import Path

from PIL import Image
from tools.terrain_picker import Catalog, Picker, classify, extract, prepare


class TerrainTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.base = Path(tmp.name)
        self.im = Image.new("RGBA", (64, 16))
        for n, shade in enumerate([0, 10, 20, 0]):
            self.im.paste((shade, shade, shade, 255), (n * 16, 0, (n + 1) * 16, 16))
        self.im.save(self.base / "source.png")
        prepare(self.base / "source.png", self.base / "review", 10)
        self.meta = self.base / "review/groups.json"
        self.choices = self.base / "picks.json"
        self.cat = Catalog(self.meta, self.choices)

    def test_partition_export_and_reopen(self):
        tiles, layout, cols = extract(self.im)
        self.assertEqual((len(tiles), layout, cols), (3, [0, 1, 2, 0], 4))
        self.assertEqual(self.cat.groups, [[0, 1], [2]])
        self.assertEqual(classify(tiles, 0)[0], [[0], [1], [2]])
        self.cat.toggle(1)
        self.cat.toggle(2)
        reopened = Catalog(self.meta, self.choices)
        self.assertEqual(reopened.selected, {0, 1})
        dest = self.base / "export"
        reopened.export(dest)
        with Image.open(dest / "tilesheet.png") as sheet:
            self.assertEqual(sheet.size, (256, 16))
            self.assertEqual(
                sheet.crop((0, 0, 32, 16)).tobytes(),
                self.im.crop((0, 0, 32, 16)).tobytes(),
            )
        self.assertEqual(
            (dest / "index.csv").read_text().splitlines(),
            ["source_id,tilesheet_id", "0,0", "1,1"],
        )
        with self.assertRaises(FileExistsError):
            reopened.export(dest)
        for i in list(reopened.selected):
            reopened.toggle(i)
        self.assertFalse(Catalog(self.meta, self.choices).selected)

    @unittest.skipUnless(os.environ.get("DISPLAY"), "requires Tk display or Xvfb")
    def test_click_and_filter(self):
        import tkinter as tk

        root = tk.Tk()
        self.addCleanup(root.destroy)
        ui = Picker(root, self.cat)
        root.update()
        for col in [1, 0, 1]:
            ui.c.event_generate("<Button-1>", x=ui.L + col * ui.W + 12, y=22)
            root.update()
        self.assertEqual(self.cat.selected, {2})
        ui.multi.set(True)
        ui.filter()
        root.update()
        self.assertEqual(ui.rows, [[0, 1]])
        self.assertEqual(Catalog(self.meta, self.choices).selected, {2})
