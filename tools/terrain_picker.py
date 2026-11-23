"""Prepare, review and export a terrain palette without changing the source image."""

import argparse
import csv
import hashlib
import json
import math
import os
from pathlib import Path
from tempfile import NamedTemporaryFile

from PIL import Image


def write_json(path, data):
    with NamedTemporaryFile("w", dir=path.parent, delete=False) as f:
        json.dump(data, f, separators=(",", ":"))
        f.write("\n")
    os.replace(f.name, path)


def extract(image):
    w, h = image.size
    cols, rows = (w + 15) // 16, (h + 15) // 16
    padded = Image.new("RGBA", (cols * 16, rows * 16))
    padded.paste(image.convert("RGBA"))
    ids, layout = {}, []
    for y in range(0, padded.height, 16):
        for x in range(0, padded.width, 16):
            key = padded.crop((x, y, x + 16, y + 16)).tobytes()
            layout.append(ids.setdefault(key, len(ids)))
    return list(ids), layout, cols


def classify(tiles, threshold):
    """Greedy full-resolution RMS; block means bound RMS from below."""
    import numpy as np

    if not (0 <= threshold <= 255):
        raise ValueError("RMS threshold must be between 0 and 255")
    data = np.frombuffer(b"".join(tiles), np.uint8).reshape(-1, 16, 16, 4).astype("f")
    data[:, :, :, :3] *= data[:, :, :, 3:4] / 255
    coarse = data.reshape(-1, 4, 4, 4, 4, 4).mean(axis=(2, 4)).reshape(len(data), -1)
    data = data.reshape(len(data), -1)
    pending = np.arange(len(data))
    groups, worst = [], (0.0, None)
    while pending.size:
        rep = int(pending[0])
        candidates = pending[
            ((coarse[pending] - coarse[rep]) ** 2).mean(axis=1) <= threshold**2 + 0.001
        ]
        errors = ((data[candidates] - data[rep]) ** 2).mean(axis=1)
        members = candidates[errors <= threshold**2]
        groups.append(members.tolist())
        at = errors <= threshold**2
        i = errors[at].argmax()
        rms = math.sqrt(float(errors[at][i]))
        if rms > worst[0]:
            worst = rms, [rep, int(members[i])]
        pending = np.setdiff1d(pending, members, assume_unique=True)
    return groups, worst


def sheet(tiles, ids):
    out = Image.new("RGBA", (256, 16 * ((len(ids) + 15) // 16)))
    for n, i in enumerate(ids):
        out.paste(
            Image.frombytes("RGBA", (16, 16), tiles[i]), (n % 16 * 16, n // 16 * 16)
        )
    return out


def prepare(source, directory, threshold):
    image = Image.open(source).convert("RGBA")
    tiles, layout, cols = extract(image)
    groups, worst = classify(tiles, threshold)
    directory.mkdir(parents=True, exist_ok=False)
    sheet(tiles, range(len(tiles))).save(directory / "atlas.png")
    write_json(
        directory / "groups.json",
        dict(
            version=1,
            source=source.name,
            source_hash=hashlib.sha256(
                image.width.to_bytes(4, "big")
                + image.height.to_bytes(4, "big")
                + image.tobytes()
            ).hexdigest(),
            size=image.size,
            columns=cols,
            layout=layout,
            groups=groups,
            threshold=threshold,
            worst=worst,
        ),
    )
    # Reconstruct the default palette so the curator can see grouping losses in context.
    rep = {i: g[0] for g in groups for i in g}
    preview = Image.new("RGBA", (cols * 16, ((len(layout) + cols - 1) // cols) * 16))
    for n, i in enumerate(layout):
        preview.paste(
            Image.frombytes("RGBA", (16, 16), tiles[rep[i]]),
            (n % cols * 16, n // cols * 16),
        )
    preview.crop((0, 0, *image.size)).save(directory / "representative-preview.png")
    if worst[1]:
        pair = Image.new("RGBA", (32, 16))
        for n, i in enumerate(worst[1]):
            pair.paste(Image.frombytes("RGBA", (16, 16), tiles[i]), (16 * n, 0))
        pair.resize((192, 96), Image.Resampling.NEAREST).save(
            directory / "worst-pair.png"
        )
    print(
        f"{len(layout)} placements, {len(tiles)} exact tiles, {len(groups)} groups; "
        f"worst RMS {worst[0]:.2f} for IDs {worst[1]}"
    )


class Catalog:
    def __init__(self, path, choices):
        data = json.loads(path.read_text())
        self.groups = data["groups"]
        self.n = sum(map(len, self.groups))
        if data["version"] != 1 or sorted(i for g in self.groups for i in g) != list(
            range(self.n)
        ):
            raise ValueError("Invalid group partition")
        self.atlas = Image.open(path.with_name("atlas.png")).convert("RGBA")
        self.choices = choices
        self.key = data["source_hash"]
        if choices.exists():
            saved = json.loads(choices.read_text())
            if saved["source_hash"] != self.key:
                raise ValueError("Selection belongs to a different image")
            self.selected = set(saved["selected"])
            if not self.selected <= set(range(self.n)):
                raise ValueError("Invalid selection IDs")
        else:
            self.selected = {g[0] for g in self.groups}

    def tile(self, i):
        x, y = i % 16 * 16, i // 16 * 16
        return self.atlas.crop((x, y, x + 16, y + 16))

    def toggle(self, i):
        self.selected.symmetric_difference_update({i})
        try:
            write_json(
                self.choices, dict(source_hash=self.key, selected=sorted(self.selected))
            )
        except OSError:
            self.selected.symmetric_difference_update({i})
            raise

    def export(self, directory):
        if not self.selected:
            raise ValueError("No tiles selected")
        directory.mkdir(exist_ok=False)
        out = Image.new("RGBA", (256, 16 * ((len(self.selected) + 15) // 16)))
        with (directory / "index.csv").open("w", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(["source_id", "tilesheet_id"])
            for n, i in enumerate(sorted(self.selected)):
                out.paste(self.tile(i), (n % 16 * 16, n // 16 * 16))
                writer.writerow([i, n])
        out.save(directory / "tilesheet.png")


class Picker:
    """Draw only visible rows and columns, even for a thousand-tile group."""

    W, H, L = 64, 76, 112

    def __init__(self, root, catalog):
        import tkinter as tk
        from tkinter import ttk

        self.root, self.cat = root, catalog
        root.title("Nurples terrain picker")
        root.geometry("1000x700")
        bar = ttk.Frame(root)
        bar.pack(fill="x")
        self.multi = tk.BooleanVar(value=False)
        ttk.Checkbutton(
            bar, text="Multi-tile groups only", variable=self.multi, command=self.filter
        ).pack(side="left")
        ttk.Button(bar, text="Export selected…", command=self.export).pack(side="right")
        self.status = ttk.Label(bar)
        self.status.pack(side="right", padx=12)
        frame = ttk.Frame(root)
        frame.pack(fill="both", expand=True)
        frame.columnconfigure(0, weight=1)
        frame.rowconfigure(0, weight=1)
        c = self.c = tk.Canvas(frame, background="#242424", highlightthickness=0)
        c.grid(row=0, column=0, sticky="nsew")
        for axis, row, col, sticky in [("y", 0, 1, "ns"), ("x", 1, 0, "ew")]:
            view = getattr(c, axis + "view")
            sb = ttk.Scrollbar(
                frame, orient="vertical" if axis == "y" else "horizontal", command=view
            )
            sb.grid(row=row, column=col, sticky=sticky)

            def sync(a, b, scroll=sb):
                scroll.set(a, b)
                self.schedule()

            c.configure(**{axis + "scrollcommand": sync})
        self.job = None
        c.bind("<Configure>", lambda e: self.schedule())
        c.bind("<Button-1>", self.click)
        for event, d in [("<Button-4>", -1), ("<Button-5>", 1)]:
            c.bind(event, lambda e, delta=d: c.yview_scroll(delta, "units"))
            c.bind(
                "<Shift-" + event[1:], lambda e, delta=d: c.xview_scroll(delta, "units")
            )
        self.filter()

    def filter(self):
        self.rows = [g for g in self.cat.groups if not self.multi.get() or len(g) > 1]
        self.c.configure(
            scrollregion=(
                0,
                0,
                self.L + self.W * max(map(len, self.rows), default=0),
                self.H * len(self.rows),
            ),
            yscrollincrement=self.H,
            xscrollincrement=self.W,
        )
        self.c.yview_moveto(0)
        self.c.xview_moveto(0)
        self.schedule()

    def schedule(self):
        if self.job is None:
            self.job = self.c.after_idle(self.draw)

    def draw(self):
        from PIL import ImageTk

        self.job = None
        c = self.c
        c.delete("all")
        self.images = []
        x, y = int(c.canvasx(0)), int(c.canvasy(0))
        for r in range(
            max(0, y // self.H),
            min(len(self.rows), (y + c.winfo_height()) // self.H + 1),
        ):
            g = self.rows[r]
            top = r * self.H + 3
            c.create_text(
                6,
                top + 20,
                text=f"Rep {g[0]}\n{len(g)} tile(s)",
                anchor="w",
                fill="white",
            )
            for j in range(
                max(0, (x - self.L) // self.W),
                min(len(g), (x + c.winfo_width() - self.L) // self.W + 1),
            ):
                i = g[j]
                left = self.L + j * self.W + 4
                photo = ImageTk.PhotoImage(
                    self.cat.tile(i).resize((48, 48), Image.Resampling.NEAREST)
                )
                self.images.append(photo)
                c.create_rectangle(
                    left - 2,
                    top,
                    left + 50,
                    top + 52,
                    width=2,
                    outline="#65e585" if i in self.cat.selected else "#555555",
                )
                c.create_image(left, top + 2, image=photo, anchor="nw")
                c.create_text(left + 24, top + 63, text=str(i), fill="white")
        self.status.configure(
            text=f"{len(self.cat.selected)} selected   ·   {len(self.rows)} groups"
        )

    def click(self, event):
        from tkinter import messagebox

        r = int(self.c.canvasy(event.y)) // self.H
        j = (int(self.c.canvasx(event.x)) - self.L) // self.W
        if 0 <= r < len(self.rows) and 0 <= j < len(self.rows[r]):
            try:
                self.cat.toggle(self.rows[r][j])
            except OSError as e:
                messagebox.showerror("Could not save selection", str(e))
            self.schedule()

    def export(self):
        from tkinter import filedialog, messagebox

        path = filedialog.asksaveasfilename(
            title="Name a NEW export folder",
            initialdir=self.cat.choices.parent,
            initialfile="earth-selection",
        )
        if path:
            try:
                self.cat.export(Path(path))
            except (ValueError, OSError) as e:
                messagebox.showerror("Export", str(e))
            else:
                messagebox.showinfo(
                    "Export", f"Saved tilesheet.png and index.csv in {path}"
                )


def main():
    cli = argparse.ArgumentParser(description=__doc__)
    sub = cli.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("prepare")
    p.add_argument("source", type=Path)
    p.add_argument("directory", type=Path, help="new output directory")
    p.add_argument("--threshold", type=float, default=20)
    p = sub.add_parser("review")
    p.add_argument("metadata", type=Path)
    p.add_argument(
        "--choices", type=Path, help="reuse this selection across thresholds"
    )
    args = cli.parse_args()
    if args.cmd == "prepare":
        prepare(args.source, args.directory, args.threshold)
    else:
        import tkinter as tk

        cat = Catalog(
            args.metadata, args.choices or args.metadata.with_name("selection.json")
        )
        root = tk.Tk()
        Picker(root, cat)
        root.mainloop()


if __name__ == "__main__":
    main()
