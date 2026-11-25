"""Build only the Earth streaming audition, from the exact preload control."""
from pathlib import Path
from tempfile import TemporaryDirectory

from nurples_tools.agnb import ImageRecord, build_container
from nurples_tools.agnb_verify import parse_images
from nurples_tools.loading_music import edit


def pack(control: bytes) -> tuple[bytes, bytes, int]:
    """Keep 14/15 pinned, replace 00..13 with a single mutable row and disk stream."""
    images = parse_images(control)
    by_id = {i.buffer_id: i for i in images}
    with TemporaryDirectory() as tmp:
        def record(n, width, height, pixels, name):
            path = Path(tmp) / name
            path.write_bytes(pixels)
            return ImageRecord("jit", name, n, width, height, path)

        strips = [by_id[1280+i] for i in range(16)]
        if any((s.width, s.height) != (256, 256) for s in strips):
            raise ValueError("unexpected Earth strip dimensions")
        rows = b"".join(s.payload[y*256:(y+1)*256]
                        for s in strips[:14] for y in reversed(range(256)))
        disk = build_container([record(1280, 256, 14*256, rows, "earth-flight")])
        resident = []
        for image in images:
            n = image.buffer_id
            if n == 1280:
                resident.append(record(n, 256, 1, bytes(256), "row"))
            elif not 1281 <= n < 1294:
                resident.append(record(n, image.width, image.height, image.payload, str(n)))
        return build_container(resident), disk, len(resident)


def configure(src, target, output):
    control = (target / "game.agnb").read_bytes()
    game, stream, count = pack(control)
    (output / "game.agnb").write_bytes(game)
    (output / "earth.agnb").write_bytes(stream)
    edit(src, "images_agnb.inc", f"game_num_images: equ {len(parse_images(control))}",
         f"game_num_images: equ {count}")
    edit(src, "earth_opening.inc", "earth_bind_background:\n",
         "earth_bind_background:\n    call jit_close\n    jp nz,jit_fail\n")
    edit(src, "earth_opening.inc", "    ld hl,earth_first_buffer\n    ld (earth_next_buffer),hl\n"
         "    ld hl,earth_first_terrain_line", "    call jit_open\n    jp nz,jit_fail\n"
         "    ld hl,earth_first_buffer+14\n    ld (earth_next_buffer),hl\n    ld hl,jit_plot")
    edit(src, "nurples.asm", '    include "earth_opening.inc"',
         '    include "earth_opening.inc"\n    include "earth_jit.inc"')
    edit(src, "runtime_init.inc", "    ld (player_event),a",
         "    ld (player_event),a\n    ld (jit_error),a\n    ld (jit_owned),a\n"
         "    ld (jit_proven),a\n    ld hl,0\n    ld (jit_read_max),hl\n    ld (jit_send_max),hl")
    edit(src, "nurples.asm", "main_end:\n", "main_end:\n    call jit_close\n    call nz,jit_fail\n")
    edit(src, "nurples.asm", "    call playfield_display_report",
         "    call jit_report\n    call playfield_display_report")
