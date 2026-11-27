"""Stage and analyze the matched pose experiments; never modify production inputs."""
from math import ceil
from statistics import mean, median, stdev
from struct import iter_unpack

from nurples_tools.earth_frame_trace import summarize
from nurples_tools.loading_music import edit


POSE_ARMS = ("direct", "sync", "async", "golem", "native-direct", "native")
DIRECT_ARMS = ("direct", "native-direct")


def pose_summary(data, arm):
    """Require the *actual* frame rows to reconcile exactly to the level totals."""
    if len(data) < 33 or data[:4] != b"EPS1" or (len(data) - 33) % 6:
        raise ValueError("invalid pose report")
    rows = list(iter_unpack("<HBBBB", data[33:]))
    plain = b"EFT1" + data[4:33] + b"".join(
        data[i:i+4] for i in range(33, len(data), 6))
    result = summarize(plain, 72)
    if result["updates"] != 4544:
        raise ValueError("not a complete Earth run")
    if arm not in POSE_ARMS or any(
            p > 32 or (s != p * 3 if arm in DIRECT_ARMS else not (0 < s <= p or s == p == 0))
            for _, _, _, p, s in rows):
        raise ValueError("impossible pose/send counts")
    if not any(p >= 8 for _, _, _, p, _ in rows):
        raise ValueError("dense formation never ran")
    reads = sorted(d / 72 for d, _, f, _, _ in rows if f == 3)
    if result["refill"]["updates"] != 895 or len(reads) < 2:
        raise ValueError("incomplete 895-refill workload or insufficient valid samples")
    result["read_ms"] = dict(n=len(reads), min=reads[0], mean=mean(reads), sd=stdev(reads),
                              median=median(reads),
                              **{k: reads[ceil(q*len(reads))-1] for k, q in (
                                  ("p95", .95), ("p99", .99), ("max", 1))})
    result["poses"] = sum(p for _, _, _, p, _ in rows)
    result["pose_sends"] = sum(s for _, _, _, _, s in rows)
    overhead = {"golem": 26, "native": 29}.get(arm)
    result["pose_wire_bytes"] = (6 * result["poses"] + overhead * result["pose_sends"]
                                 if overhead else 15 * result["poses"])
    dense = [t for _, t, _, p, _ in rows if p >= 8]
    result["at_least_eight_poses"] = dict(updates=len(dense), vblanks=sum(dense),
                                          skipped=sum(max(0, t-1) for t in dense))
    return result


def configure_pose(src, arm, root=None, output=None):
    if arm not in POSE_ARMS:
        raise ValueError("unknown pose transport")
    # Read the exact same physical FAT extents for each arm. Each binary
    # and its create-new report have their own working directory. No asset
    # copies, moves or verification reads against the staged control.
    for source, names in (("images.inc", ("game.agnb", "ui.agnb")),
                          ("announcer.inc", ("voice.agnb",)),
                          ("fonts_list.inc", ("fonts/Lat38-VGA8_8x8.font",)),
                          ("earth_jit.inc", ("earth.agnb",))):
        for name in names:
            edit(src, source, f'"{name}"', f'"/test/earth-ab-vl1/control/{name}"')
    # Keep the same SD pipeline and 267-byte wire packet in every arm, but
    # use ordinary MOS output. No producer owns the row after RST 18 returns.
    edit(src, "earth_batch.inc", "    ld de,batch_callback\n    ld a,025h\n    rst.lil 08h",
         "    rst.lil 18h")
    path = src / "earth_batch.inc"
    body = path.read_text()
    edit(src, path.name, body[body.index("batch_callback:"):body.index("jit_plot:")],
         "batch_join:\n    ret ; synchronous producer\n\n")
    edit(src, "sprites.inc", '    include "side_to_side.inc"',
         f'pose_direct: equ {int(arm in DIRECT_ARMS)}\npose_async: equ {int(arm == "async")}\n'
         '    include "earth_pose.inc"\n    include "side_to_side.inc"')
    if arm == "golem":
        from nurples_tools.golem_pose import stage_package
        stage_package(root, src, output/"compiler")
        edit(src, "sprites.inc", '    include "earth_pose.inc"', '    include "earth_pose_state.inc"')
        # LOADTUNE has stopped and gameplay timing has not begun. Do not
        # install during attract, or interrupt the 50 Hz player with a long
        # command stream. This unattended fixture runs exactly one session.
        edit(src, "presentation_flow_states.inc", "    call game_new_session_init",
             "    call pose_install\n    call game_new_session_init")
    if arm in ("native-direct", "native"):
        # The actual bench observer enters with VDU 2; native vdu_submit
        # refuses a live printer. Match this envelope in an independent
        # direct arm and restore the observer before the ordinary outro.
        edit(src, "presentation_flow_states.inc", "    call game_new_session_init",
             "    ld a,3\n    rst.lil 10h\n" +
             ("    call native_startup\n    jp nc,presentation_flow_fail\n" if arm == "native" else "") +
             "    call game_new_session_init")
        edit(src, "nurples.asm", "    call presentation_flow_shutdown",
             ("    call native_shutdown\n" if arm == "native" else "") +
             "    ld a,2\n    rst.lil 10h\n    call presentation_flow_shutdown")
    if arm == "native":
        import shutil
        import subprocess
        import sys

        sdk = root.parent / "agon-vdplet/tools/build_module.py"
        subprocess.run([sys.executable, str(sdk), str(root/"docs/tasks/EARTH-001/pose/native.c"),
                        "--output", str(output/"sdk"), "--name", "nurples-pose",
                        "--build-id", "0x4e505331", "--capabilities", "5"], check=True)
        shutil.copyfile(output/"sdk/nurples-pose.vdpl", src/"pose.vdpl")
        edit(src, "sprites.inc", '    include "earth_pose.inc"', '    include "earth_pose_native.inc"')
        edit(src, "earth_pose_packed.inc", "    ld hl,pose_frame_n",
             "    ld hl,native_n\n    inc (hl)\n    ld hl,pose_frame_n")
        edit(src, "earth_bench_report.inc", "bench_save:\n", "bench_save:\n    call native_diagnostic\n")
    # No change to the just-spawned pass: it publishes and shows immediately.
    old = "    ld a,(iy+sprite_id) ; get spriteId\n    call vdu_sprite_select ; select sprite "
    edit(src, "sprites.inc", "    jp nc,@next_record\n" + old,
         "    jp nc,@next_record\n    ld a,(pose_eligible)\n    or a\n    jr z,@direct_pose\n"
         "    call pose_publish\n    jr @next_record\n@direct_pose:\n" + old)
    edit(src, "sprites.inc", "    push iy ; behavior callbacks",
         "    call pose_before_behavior ; HL is validated move program\n    push iy ; behavior callbacks")
    edit(src, "sprites.inc", "; reset all just spawned flags",
         "    call pose_submit\n; reset all just spawned flags")
    edit(src, "sprites.inc", "    res 7,(iy+sprite_type) ; clear just spawned flag",
         "    call pose_barrier\n    res 7,(iy+sprite_type) ; clear just spawned flag")
    edit(src, "sprites.inc", "    djnz @reset_just_spawned\n    ret",
         "    djnz @reset_just_spawned\n    jp pose_barrier")
    for label in ("@bad_record:", "@bad_reset_record:"):
        edit(src, "sprites.inc", label + "\n    ld hl,sprite_fault_flags",
             label + "\n    call pose_barrier\n    ld hl,sprite_fault_flags")
    edit(src, "sprites.inc", "table_deactivate_sprite:\n", "table_deactivate_sprite:\n    call pose_barrier\n")
    # Earth motion is CPU-only except for these visibility/contact transitions.
    for before in ("    ld (iy+air_visibility),side_hidden", "    call update_shields", "    call update_score"):
        edit(src, "enemy_side_air.inc", before, "    call pose_barrier\n" + before)

    template = "    db 23,27,4,0,23,27,10,0,23,27,13,0,0,0,0\n"
    (src / "pose_templates.inc").write_text(template * 32)
    # Preserve all original 4-byte frame rows and counters; add two measured
    # bytes for actual eligible poses and host submissions on this update.
    edit(src, "earth_bench_report.inc", 'db "EFT1"', 'db "EPS1"')
    edit(src, "earth_bench_report.inc", 'asciz "EARTHFR.BIN"', 'asciz "POSE.BIN"')
    edit(src, "earth_bench_report.inc", "trace_rows: ds 5000*4", "trace_rows: ds 5000*6")
    edit(src, "earth_frame_trace.inc", "trace_begin_frame:\n    xor a",
         "trace_begin_frame:\n    xor a\n    ld (pose_frame_n),a\n    ld (pose_frame_sends),a")
    edit(src, "earth_frame_trace.inc", "    ld (trace_cursor),hl\n    pop hl",
         "    ld a,(pose_frame_n)\n    ld (hl),a\n    inc hl\n    ld a,(pose_frame_sends)\n"
         "    ld (hl),a\n    inc hl\n    ld (trace_cursor),hl\n    pop hl")
