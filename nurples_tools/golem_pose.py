"""Build the stock VDP pose expander with Golem, not with a second bytecode compiler."""
import json
import subprocess
from hashlib import sha256
from pathlib import Path


def compile_program(compiler: Path, source: Path, directory: Path):
    directory.mkdir(parents=True, exist_ok=True)
    # One VDU 0 (NOP) between frame select and move aligns both signed words.
    template = bytes([23, 27, 4, 0, 23, 27, 10, 0, 0, 23, 27, 13, 0, 0, 0, 0])
    data = directory / "initial-commands.bin"
    data.write_bytes(template * 32)
    package = directory / "package"
    subprocess.run([str(compiler), "--portable", "--target", "buffered",
                    "--reserve", "1:8191", "--reserve", "64000:65534",
                    "--array-data", "commands=" + str(data),
                    str(source), str(package)], check=True)
    return package


def compile_fixed(compiler: Path, source: Path, directory: Path):
    """Specialize Golem source, not bytecode. Keep the generated source as evidence."""
    directory.mkdir(parents=True, exist_ok=True)
    _, text = source.read_text().split("// @inputs\n")
    inputs, text = text.split("// @commands\n")
    commands, body = text.split("// @body\n")
    def slots(section, n=32):
        return "".join(section.replace("@", str(i)) for i in range(32-n, 32))
    # Structured suffix entries restrict work to the smallest power of two
    # covering this packet. CALL-with-offset draws only the actual N poses.
    full = slots(inputs) + slots(commands)
    # Jump to the end of this called buffer before compiler-owned tail fields.
    full += "uint16 endA = 23\nuint16 endB = 65440\nuint16 endC = 2047\n"
    for n in (1, 2, 4, 8, 16, 32):
        full += f"Sub expand{n}\n" + slots(body, n) + "EndSub\n"
    program = directory / "fixed.golem"
    program.write_text(full)
    package = directory / "package"
    subprocess.run([str(compiler), "--portable", "--target", "buffered",
                    "--reserve", "1:8191", "--reserve", "64000:65534",
                    str(program), str(package)], check=True)
    return package


def stage_package(root, src, destination):
    compiler = root.parent / "golem/build/golemc"
    if sha256(compiler.read_bytes()).hexdigest() != "65e25fe4a0fbe8d9ac7eb49b10d07c79bb70d03829850c5ca9b06430bd275442":
        raise ValueError("Golem compiler differs from pose qualification")
    package = compile_fixed(compiler, root/"docs/tasks/EARTH-001/pose/fixed.golem.in", destination)
    spec = json.loads((package/"interface.json").read_bytes())
    meta = json.loads((package/"backends.json").read_bytes())
    fields = {g["name"]: g["offset"] for g in spec["globals"]}
    entries = {e["name"]: meta["buffered_entries"][e["logical_id"]] for e in spec["entries"]}
    first = fields["inPair0"]
    output = fields["startSelect0"]
    for n in range(32):
        if [fields[f"{k}{n}"] for k in ("inPair","inX","inY")] != [first+n*6+i for i in (0,2,4)]:
            raise ValueError("unexpected Golem input packing")
        if [fields[f"{k}{n}"] for k in ("startSelect", "outId", "outFrame", "outX", "outY")] != [
                output+n*18+i for i in (0,4,8,14,16)]:
            raise ValueError("unexpected Golem output packing")
    if fields["endA"] != output+32*18:
        raise ValueError("Golem output has no immediate end")
    desc = [f"pose_state_id: equ {meta['shared_state']}", f"pose_input_tail: equ {first+32*6}",
            "pose_descriptors:"]
    for n in range(1, 33):
        entry = entries[f"expand{1 << (n-1).bit_length()}"]
        desc.append(f"    dl {entry},{output+(32-n)*18}")
    (src/"pose_package.inc").write_text("\n".join(desc) + "\n")
    # The portable backend emits a global affine-enable even for integer
    # programs that do not use a matrix. Omit that *one* unrelated setting,
    # not a single compiled command buffer. No firmware-global state to undo.
    buffered = (package/"buffered-install.bin").read_bytes()
    affine_enable = bytes((23, 0, 248, 1, 0, 1, 0))
    if not buffered.startswith(affine_enable):
        raise ValueError("unexpected Golem install prologue")
    (src/"pose_install.bin").write_bytes((package/"common-install.bin").read_bytes()
                                         + buffered[len(affine_enable):])
