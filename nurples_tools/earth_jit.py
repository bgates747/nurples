"""Build only the Earth streaming audition, from the exact preload control."""
from hashlib import sha256
from pathlib import Path
from tempfile import TemporaryDirectory

from nurples_tools.agnb import ImageRecord, build_container
from nurples_tools.agnb_verify import parse_images
from nurples_tools.loading_music import edit


def stage_alignment_reader(src, utils_root):
    """Pin AGNB's delivered dual-version reader in the private 256-byte build only."""
    reader = (utils_root / "examples/agnb/api/agnb_api.inc").read_bytes()
    if sha256(reader).hexdigest() != "c41c3db717c533a28faf1616ddf293dcd1a303cbfbbff45bc4f07068a38ec6b5":
        raise ValueError("AGNB reader differs from the tested Earth alignment handoff")
    (src / "agnb_api.inc").write_bytes(reader)


def configure_auto_pair(src):
    """Only in the unattended executable; normal play and the interactive control are untouched."""
    edit(src, "state_game_init.inc", "    call game_start_config_begin",
         "    call player_joystick_disable\n    call game_start_session")
    edit(src, "state_game_init.inc", "    call waitKeypress\n    ld a,1\n    ld (game_exit_requested),a\n    ret\n\n@agnb_load_error:",
         "    ld a,1\n    ld (game_exit_requested),a\n    ret\n\n@agnb_load_error:")
    edit(src, "state_game_init.inc", "    call waitKeypress\n    ld a,1\n    ld (game_exit_requested),a",
         "    ld a,1\n    ld (game_exit_requested),a")
    edit(src, "player_input.inc", "    bit 2,(ix+12)\n    jp nz,player_spawn_laser",
         "    jp player_spawn_laser ; held Space, ordinary timer/power/slot checks")
    edit(src, "state_game_playing.inc", "    ld hl,game_level_transition",
         "    ld a,1\n    ld (bench_complete),a\n    ld hl,game_level_transition")
    edit(src, "nurples.asm", "    call pace_sample",
         "    call pace_sample\n    ld a,(bench_complete)\n    or a\n    jp nz,@state_exit")
    edit(src, "earth_jit.inc", "    ld de,14*65536",
         "    ld (bench_offset),hl ; actual parser handoff\n    ld de,14*65536")
    edit(src, "nurples.asm", "    call lp_save", "    call bench_save")
    edit(src, "nurples.asm", '    include "vblank_wait_clock.inc"',
         '    include "vblank_wait_clock.inc"\n    include "earth_bench_report.inc"')


def configure_frame_trace(src):
    """Inject the observer into the staged diagnostic only."""
    edit(src, "frame_pace_fixture.inc", "pace_init:\n", "pace_init:\n    call trace_reset\n")
    edit(src, "frame_pace_fixture.inc", "pace_classify:\n", "pace_classify:\n    call trace_begin_frame\n")
    edit(src, "frame_pace_fixture.inc", "@elapsed:\n", "@elapsed:\n    call trace_append\n")
    # Use identical elapsed-time arithmetic for both timing paths, but only
    # the read path updates trace_sd (not the containing batch-service timer).
    jit = (src / "earth_jit.inc").read_text()
    body = jit[jit.index("jit_time_end:\n"):jit.index("jit_report:\n")]
    read_end = body.replace("jit_time_end:", "trace_read_end:", 1)
    read_end = read_end.replace("    ret m\n", "    ret m\n    call trace_valid_sd\n")
    edit(src, "earth_jit.inc", body, body + read_end)
    edit(src, "earth_batch.inc", "batch_read:\n", "batch_read:\n    call trace_mark_read\n")
    edit(src, "earth_batch.inc", "ld ix,jit_read_max\n    call jit_time_end",
         "ld ix,jit_read_max\n    call trace_read_end")
    name = "earth_bench_report.inc"
    edit(src, name, 'asciz "EARTHAB.BIN"', 'asciz "EARTHFR.BIN"')
    edit(src, name, 'db "EAB1"', 'db "EFT1"')
    edit(src, name, "bench_record_end:", """trace_truncated: db 0
trace_rows: ds 5000*4
trace_rows_end:
    include "earth_frame_trace.inc" """.rstrip())
    edit(src, name, "    ld bc,bench_record_end-bench_record",
         "    ld hl,(trace_cursor)\n    ld de,bench_record\n    or a\n    sbc hl,de\n"
         "    ld (trace_length),hl\n    push hl\n    pop bc\n"
         "    ld hl,agnb_fil\n    ld de,bench_record")
    edit(src, name, "    ld hl,bench_record_end-bench_record", "    ld hl,(trace_length)")


def pack(control: bytes, group_rows=1) -> tuple[bytes, bytes, int]:
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
        if group_rows not in (1, 4):
            raise ValueError("unsupported Earth row group")
        rows = b"".join(s.payload[y*256:(y+group_rows)*256]
                        for s in strips[:14] for y in reversed(range(0, 256, group_rows)))
        disk = build_container([record(1280, 256, 14*256, rows, "earth-flight")])
        resident = []
        for image in images:
            n = image.buffer_id
            if n == 1280:
                resident.append(record(n, 256, group_rows, bytes(256*group_rows), "row"))
            elif not 1281 <= n < 1294:
                resident.append(record(n, image.width, image.height, image.payload, str(n)))
        return build_container(resident), disk, len(resident)


def configure(src, target, output, group_rows=1, async_row=False):
    control = (target / "game.agnb").read_bytes()
    game, stream, count = pack(control, group_rows)
    if group_rows == 4 or async_row:
        if async_row:
            edit(src, "earth_batch.inc", "batch_row_mode: equ 0", "batch_row_mode: equ 1")
        edit(src, "earth_jit.inc", "    jp jit_refill", "    jp batch_open")
        edit(src, "earth_jit.inc", "jit_plot:", '    include "earth_batch.inc"\njit_control_plot:')
        edit(src, "earth_jit.inc", "    ld hl,jit_cached\n", "jit_consume_row:\n    ld hl,jit_cached\n")
        edit(src, "earth_jit.inc", "SD / row submit", "SD / batch service")
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
