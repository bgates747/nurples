"""Run the actual compiled stock VDU program in Golem's independent command model."""
import argparse
import importlib.util
import json
import struct
from pathlib import Path


def check(package, model):
    spec = importlib.util.spec_from_file_location("stock_command_model", model)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    machine = module.Machine()
    manifest = json.loads((package / "interface.json").read_bytes())
    backend = json.loads((package / "backends.json").read_bytes())
    for part in ("common-install.bin", "buffered-install.bin"):
        machine.execute((package / part).read_bytes())
    print(f"{len(machine.memory)} resident buffers; "
          f"{sum(map(len, machine.memory.values()))} resident payload bytes (no allocator overhead)")
    fixed = len(backend["buffered_entries"]) != 1
    fields = {g["name"]: g["offset"] for g in manifest["globals"]}
    buffers = {item["name"]: item["buffer"] for item in manifest.get("arrays", [])}
    shared = backend["shared_state"]
    for count in (0, 1, 12, 32, 3, 1, 0):
        # Deliberately retain previous output, as the real VDP does.
        incoming = bytearray() if fixed else bytearray(struct.pack("<H", count))
        expected = bytearray()
        for i in range(count):
            sprite, frame = 12 + (i * 7) % 32, (i + count) % 6
            x, y = -3 + 31*i, 513 - 23*i
            incoming.extend(struct.pack("<BBhh", sprite, frame, x, y))
            expected.extend(bytes((0,23,27,4,sprite,0,23,27,10,frame,0,23,27,13)
                                  if fixed else (23,27,4,sprite,23,27,10,frame,0,23,27,13)))
            expected.extend(struct.pack("<hh", x, y))
        buf = shared if fixed else buffers["actors"]
        offset = fields["inPair0"] + (32-count)*6 if fixed else 0
        entry = backend["buffered_entries"][((count-1).bit_length() if count else 0) if fixed else 0]
        if incoming:
            cmd = bytes((23,0,160)) + struct.pack("<HBBHH", buf,5,0xc2,offset,len(incoming))
            machine.execute(cmd + incoming)
        machine.steps = 0
        if count or not fixed:
            machine.execute(bytes((23,0,160)) + struct.pack("<HB", entry, 1))
        if fixed:
            start = fields["startSelect0"] + (32-count)*18
            result = machine.memory[shared][start:fields["endA"]]
            assert machine.memory[shared][fields["endA"]:fields["endA"]+6] == bytes((23,0,160,255,255,7))
        else:
            result = machine.memory[buffers["commands"]][(32-count)*16:]
        if result != expected or any(machine.memory[backend["shared_state"]][12:16]):
            raise ValueError(f"wrong poses or runtime error for {count} ships")
        print(f"{count:2d} poses: output OK, {machine.steps} buffered command-model operations")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("package", type=Path)
    parser.add_argument("model", type=Path)
    args = parser.parse_args()
    check(args.package, args.model)
