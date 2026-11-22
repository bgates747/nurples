"""Make/check a disposable MOS EXEC test of the real, linked cue routines.

Run after autoexec LOAD (not RUN). Inject only RAM inputs: pending band and
PRT counter. SAVE reads the speech ID from the actual VDU command template.
This checks sequencing, not speaker output. Never use a physical SD for this.
"""
import argparse
import json
from pathlib import Path


def prepare(bundle, out):
    binary = (bundle / "nurples.bin").read_bytes()
    symbols = {s: int(v[1:], 16) for s, v in
               (line.split() for line in (bundle / "nurples.symbols").read_text().splitlines()
                if line.strip())}
    start = 0x040000
    say = symbols["vox_say"] - start
    assert binary[say] == 0x32  # ld (@id),a: resolve the real outgoing packet operand
    id_addr = int.from_bytes(binary[say+1:say+4], "little")
    i = id_addr - start
    assert binary[i-6:i+2] == bytes((23, 0, 133, 11, 4, 8, 1, 0xfb))
    symbols["sample_id"] = id_addr
    delay = symbols["cues_alert_ticks"]
    out.mkdir(exist_ok=False)
    commands = []
    observations = {}

    def address(name, size=1):
        addr = symbols[name]
        assert start <= addr < addr + size <= start + len(binary)
        return f"&{addr:x}"

    def put(name, value, size=1):
        data = (value % (1 << (8 * size))).to_bytes(size, "little")
        file = 'v' + data.hex() + '.bin'
        (out / file).write_bytes(data)
        commands.append(f"load {file} {address(name, size)}")

    def call(name):
        commands.append("jmp " + address(name))

    def expect(label, name, value):
        file = f'o{len(observations):02d}.bin'
        observations[file] = [label, value]
        commands.append(f"save {file} {address(name)} 1")

    def poll(tick):
        put("lp_irqs", tick, 3)
        call("vox_frame")

    call("vox_reset")
    call("vox_open")
    put("sample_id", 0xfe)
    call("cues_queue_launch")
    call("cues_queue_launch")
    expect("two successful spawns queued", "cues_launches", 2)
    poll(0)
    expect("both launches drained", "cues_launches", 0)
    expect("no warning on quiet frame", "cues_pre_alert", 0)

    put("vox_warning", 2)
    poll(0xfffff8)
    expect("yellow arms klaxon", "cues_pre_alert", 1)
    poll(0xfffff8 + delay - 1)
    expect("silent until release tail ends, across counter wrap", "sample_id", 0xfe)
    poll(0xfffff8 + delay)
    expect("depleted speech at deadline", "sample_id", 2)
    expect("depleted warning consumed", "vox_warning", 0)

    put("vox_warning", 2)
    poll(100)
    put("vox_warning", 3)
    poll(105)
    expect("critical upgrade did not restart whoop", "cues_alert_start", 100)
    poll(100 + delay - 1)
    expect("no critical speech early", "sample_id", 2)
    poll(100 + delay)
    expect("critical speech at original deadline", "sample_id", 3)
    expect("critical pre-alert consumed", "cues_pre_alert", 0)

    put("vox_warning", 2)
    poll(200)
    call("cues_cancel_warning")
    poll(200 + delay + 1)
    expect("new life discards old warning", "sample_id", 3)
    put("vox_warning", 3)
    poll(300)
    call("vox_game_over")
    expect("Game Over speaks immediately", "sample_id", 1)
    poll(300 + delay + 1)
    expect("no stale warning after Game Over", "sample_id", 1)
    call("vox_close")
    (out / "checks.txt").write_bytes(("\r\n".join(commands) + "\r\n").encode("ascii"))
    (out / "expected.json").write_text(json.dumps(observations, indent=2))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("prepare", "verify"))
    parser.add_argument("source", type=Path)
    parser.add_argument("directory", type=Path)
    args = parser.parse_args()
    if args.action == "prepare":
        prepare(args.source, args.directory)
    else:
        for name, (label, value) in json.loads((args.source / "expected.json").read_text()).items():
            actual = (args.directory / name).read_bytes()
            if actual != bytes([value]):
                raise SystemExit(f"{label}: expected {value}, got {actual.hex()}")
            print(f"PASS: {label}")
