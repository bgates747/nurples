"""Instrument the private direct control and decode EDL1. Never applied to normal game builds."""
import argparse
from collections import Counter
import json
from pathlib import Path
from struct import iter_unpack

from nurples_tools.earth_pose import pose_summary
from nurples_tools.loading_music import edit

BASELINE = "1e9c3aed753566a4f94fd2717266d079a1f36fa51d8b41c4b6c0216a0cb29177"
PHASES = ("", "other", "carry_in", "tile_pipeline", "scroll", "sd_refill",
          "background_upload", "background_draw", "actor_work", "pose_send", "outer_wait")


def configure_deadlines(src):
    """Patch a disposable staging tree, after configure_pose(native-direct)."""
    wrappers = {}

    def wrapper(name, phase, action, *, outer=False):
        # Main-thread only. The existing MOS or callee may change registers
        # and flags: preserve the input AND the output, rather than undo either.
        before = f"ld a,{phase}\n    call dl_set_parent" if outer else "ld a,(dl_parent)\n    call dl_observe"
        after = "ld a,1\n    call dl_set_parent" if outer else f"ld a,{phase}\n    call dl_observe"
        code = (f"{name}:\n    push af\n    {before}\n    pop af\n    {action}\n"
                f"    push af\n    {after}\n    pop af\n    ret\n")
        if wrappers.setdefault(name, code) != code:
            raise ValueError(f"conflicting probe {name}")
        return name

    def call(file, target, phase, *, outer=False, opcode="call", prefix=""):
        probe = wrapper(f"dl_{target}", phase, "call " + target, outer=outer)
        edit(src, file, prefix + f"    {opcode} {target}\n", prefix + f"    {opcode} {probe}\n")

    game = "state_game_playing.inc"
    call(game, "tiles_plot", 3, outer=True)
    for name in ("do_player", "do_enemies", "move_explosions", "move_tiles"):
        call(game, name, 8, outer=True)
    for file in ("tiles.inc", "starfield_warp.inc"):
        call(file, "vdu_scroll_down", 4)
    call("earth_batch.inc", "agnb_read_exact", 5)
    upload = wrapper("dl_row_send", 6, "rst.lil 18h")
    edit(src, "earth_batch.inc", "    rst.lil 18h\n", f"    call {upload}\n")
    # Only background bitmap select and plot. Terrain and active-tile plots
    # use the same VDU helpers and must not be charged to the background.
    for file, end, opcode in (("earth_batch.inc", "    ld bc,0\n", "call"),
                              ("tiles.inc", "    inc de\n", "call"),
                              ("earth_opening.inc", "    inc de\n", "jp"),
                              ("starfield_warp.inc", "    inc de\n", "call")):
        # tiles.inc has another select/plot for ordinary terrain.
        prefix = "    ret nz\n" if file == "tiles.inc" else ""
        call(file, "vdu_buff_select", 7, prefix=prefix)
        call(file, "vdu_plot_bmp", 7, opcode=opcode, prefix=end)
    # All ordinary sprite select, frame and movement send helpers, including
    # non-formation actors. Preparation remains in the enclosing CPU scope.
    file = "vdu_sprites.inc"
    send = wrapper("dl_pose_send", 9, "rst.lil $18")
    for name in ("select", "next_frame", "prev_frame", "select_frame", "move_abs", "move_rel",
                 "move_abs168", "move_rel168"):
        body = (src / file).read_text().split(f"\nvdu_sprite_{name}:\n", 1)[1].split("    ret\n", 1)[0]
        if body.count("rst.lil $18") != 1:
            raise ValueError(f"changed MOS send in {name}")
        anchor = f"\nvdu_sprite_{name}:\n" + body
        edit(src, file, anchor, anchor.replace("rst.lil $18", f"call {send}"))

    file = "frame_pace_fixture.inc"
    edit(src, file, "    ld (pace_count_this),a\n    ret", "    ld (pace_count_this),a\n    jp dl_begin")
    edit(src, file, "pace_sample:\n", "pace_sample:\n    call dl_end\n")
    edit(src, "nurples.asm", "    call wait_clock_frame\n",
         "    push af\n    ld a,10\n    call dl_set_parent\n    pop af\n    call wait_clock_frame\n")
    file = "earth_bench_report.inc"
    edit(src, file, 'db "EPS1"', 'db "EDL1"')
    edit(src, file, 'asciz "POSE.BIN"', 'asciz "DEADLINE.BIN"')
    edit(src, file, "trace_rows: ds 5000*6", "trace_rows: ds 5000*10")
    edit(src, file, '    include "earth_frame_trace.inc"',
         '    include "earth_frame_trace.inc"\n    include "earth_deadlines.inc"\n' + "\n".join(wrappers.values()))
    edit(src, "earth_frame_trace.inc", "    ld (trace_cursor),hl\n    pop hl",
         "    ld de,(dl_tags)\n    ld (hl),de\n    inc hl\n    inc hl\n    inc hl\n"
         "    ld a,(dl_flags)\n    ld (hl),a\n    inc hl\n    ld (trace_cursor),hl\n    pop hl")


def _phases(ticks, tags, flags):
    if flags & ~3 or any(tag >= len(PHASES) for tag in tags) or (flags & 1 and not ticks):
        raise ValueError("invalid phase/claim flags")
    n = min(3, ticks)
    if not flags & 2 and (not all(tags[:n]) or any(tags[n:])):
        raise ValueError("edge tags do not match epoch delta")
    return (["unresolved"] * ticks if flags & 2 else
            [PHASES[tag] for tag in tags[:n]] + ["unresolved"] * (ticks-n))


def cohort(rows):
    first, missed, first_on_late = Counter(), Counter(), Counter()
    pending = skips = vblanks = late = zero = 0
    for _, ticks, _, _, _, a, b, c, flags in rows:
        phases = _phases(ticks, (a, b, c), flags)
        pending += flags & 1
        vblanks += ticks
        zero += ticks == 0
        if phases:
            first[phases[0]] += 1
        if ticks > 1:
            late += 1
            skips += ticks-1
            first_on_late[phases[0]] += 1
            missed.update(phases[1:])
    return dict(updates=len(rows), vblanks=vblanks, skipped=skips, zero_tick=zero,
                frames_with_skips=late, pending_at_claim=pending, fresh_wait_path=len(rows)-pending,
                skips_per_update=skips/len(rows) if rows else None,
                skipped_pct_of_vblanks=100*skips/vblanks if vblanks else None,
                skips_per_elapsed_second_60hz=60*skips/vblanks if vblanks else None,
                first_edge=dict(first), first_edge_on_frames_with_skips=dict(first_on_late),
                additional_edges=dict(missed))


def summarize_deadlines(data, control=None, *, list_dense_misses=False):
    if len(data) < 33 or data[:4] != b"EDL1" or (len(data)-33) % 10:
        raise ValueError("not an EDL1 deadline report")
    rows = list(iter_unpack("<H8B", data[33:]))
    eps = b"EPS1" + data[4:33] + b"".join(data[i:i+6] for i in range(33, len(data), 10))
    workload = pose_summary(eps, "native-direct")
    result = dict(all=cohort(rows), dense=cohort([r for r in rows if r[3] >= 8]),
                  light=cohort([r for r in rows if r[3] < 8]), eligible_poses=workload["poses"],
                  eligible_pose_sends=workload["pose_sends"], sd_read_ms=workload["read_ms"])
    if list_dense_misses:
        result["dense_misses"] = [
            dict(update=i, poses=p, sends=s, refilled=bool(f & 1), sd_ms=sd/72 if f == 3 else None,
                 due=edges[0], skipped_in=edges[1:], pending_at_claim=bool(flags & 1))
            for i, (sd, t, f, p, s, a, b, c, flags) in enumerate(rows, 1) if p >= 8 and t > 1
            for edges in [_phases(t, (a, b, c), flags)]]
    if control is not None:
        baseline = pose_summary(control, "native-direct")
        plain = list(iter_unpack("<H4B", control[33:]))
        if [(r[2] & 1, r[3], r[4]) for r in plain] != [(r[2] & 1, r[3], r[4]) for r in rows]:
            raise ValueError("control/observer per-update refill or pose workload differs")
        result["uninstrumented_control"] = {k: baseline[k] for k in ("skipped", "vblanks", "updates",
                                                                       "at_least_eight_poses")}
    return result


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Decode direct Earth VBlank attribution (dense = 8+ eligible poses)")
    parser.add_argument("report", type=Path)
    parser.add_argument("--control", type=Path, help="optional uninstrumented direct EPS1 report")
    parser.add_argument("--list-dense-misses", action="store_true", help="include individual late dense updates")
    args = parser.parse_args()
    print(json.dumps(summarize_deadlines(args.report.read_bytes(),
                                         args.control.read_bytes() if args.control else None,
                                         list_dense_misses=args.list_dense_misses), indent=2))
