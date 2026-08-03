"""Deterministic AGNB 0.1 image-container writer.

The layout and validation rules mirror the canonical implementation in
``agon-utils/examples/agnb``.  This module owns no image conversion; callers
provide already encoded RGBA2222 payloads.
"""

from dataclasses import dataclass
from pathlib import Path
import struct


VERSION_MAJOR = 0
VERSION_MINOR = 1
IMAGE_FORMAT_RGBA2222 = 1
INVALID_BUFFER_ID = 0xFFFF
MAX_U16 = 0xFFFF
MAX_U32 = 0xFFFFFFFF


@dataclass(frozen=True)
class ImageRecord:
    """One explicitly numbered RGBA2222 VDP bitmap."""

    family: str
    name: str
    buffer_id: int
    width: int
    height: int
    payload_path: Path

    @property
    def data_size(self) -> int:
        return self.width * self.height


def align4(size: int) -> int:
    return (size + 3) & ~3


def make_chunk(chunk_id: bytes, payload: bytes) -> bytes:
    """Return a complete four-byte-aligned RIFF chunk."""

    if len(chunk_id) != 4:
        raise ValueError(f"RIFF chunk ID must contain four bytes: {chunk_id!r}")
    if len(payload) > MAX_U32:
        raise ValueError(f"RIFF payload is too large: {len(payload)} bytes")
    padding = bytes(align4(len(payload)) - len(payload))
    return chunk_id + struct.pack("<I", len(payload)) + payload + padding


def validate_records(records: list[ImageRecord]) -> None:
    """Fail before writing if a catalog cannot be represented by AGNB 0.1."""

    if not records:
        raise ValueError("an AGNB container requires at least one image record")

    seen_ids: set[int] = set()
    seen_names: set[str] = set()
    for record in records:
        if not record.family or not record.name:
            raise ValueError("AGNB image family and name must be nonempty")
        if record.name in seen_names:
            raise ValueError(f"duplicate image name: {record.name}")
        seen_names.add(record.name)
        if not 0 <= record.buffer_id < INVALID_BUFFER_ID:
            raise ValueError(f"invalid VDP buffer ID: {record.buffer_id:#x}")
        if record.buffer_id in seen_ids:
            raise ValueError(f"duplicate VDP buffer ID: {record.buffer_id:#x}")
        seen_ids.add(record.buffer_id)
        if not 1 <= record.width <= MAX_U16:
            raise ValueError(f"invalid width for {record.name}: {record.width}")
        if not 1 <= record.height <= MAX_U16:
            raise ValueError(f"invalid height for {record.name}: {record.height}")
        if record.data_size > MAX_U32:
            raise ValueError(f"image is too large for AGNB: {record.name}")
        if not record.payload_path.is_file():
            raise FileNotFoundError(record.payload_path)
        actual_size = record.payload_path.stat().st_size
        if actual_size != record.data_size:
            raise ValueError(
                f"{record.name} contains {actual_size} bytes; "
                f"expected {record.data_size}"
            )


def make_buffer_record(record: ImageRecord) -> bytes:
    pixels = record.payload_path.read_bytes()
    if len(pixels) != record.data_size:
        raise RuntimeError(f"RGBA2222 payload changed while packing: {record.name}")

    nested = b"".join(
        (
            make_chunk(b"BHDR", struct.pack("<H", record.buffer_id)),
            make_chunk(
                b"IMAG",
                struct.pack(
                    "<HHB",
                    record.width,
                    record.height,
                    IMAGE_FORMAT_RGBA2222,
                ),
            ),
            make_chunk(b"DATA", pixels),
        )
    )
    return make_chunk(b"LIST", b"BUFR" + nested)


def build_container(records: list[ImageRecord]) -> bytes:
    """Compile a validated ordered catalog into one RIFF AGNB file."""

    validate_records(records)
    body = bytearray(b"AGNB")
    body.extend(make_chunk(b"VERS", bytes((VERSION_MAJOR, VERSION_MINOR))))
    for record in records:
        body.extend(make_buffer_record(record))
    if len(body) > MAX_U32:
        raise ValueError(f"AGNB container is too large: {len(body) + 8} bytes")
    return b"RIFF" + struct.pack("<I", len(body)) + body
