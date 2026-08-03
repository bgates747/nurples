"""Independent, strict parser used to verify generated AGNB image files."""

from dataclasses import dataclass
import struct

from nurples_tools.agnb import ImageRecord


@dataclass(frozen=True)
class ParsedImage:
    buffer_id: int
    width: int
    height: int
    image_format: int
    payload: bytes


def _chunk(
    data: bytes,
    offset: int,
    boundary: int,
) -> tuple[bytes, int, int, int]:
    if offset + 8 > boundary:
        raise ValueError(f"truncated chunk header at offset {offset}")
    chunk_id = data[offset : offset + 4]
    size = struct.unpack_from("<I", data, offset + 4)[0]
    start = offset + 8
    end = start + size
    next_offset = start + ((size + 3) & ~3)
    if end > boundary or next_offset > boundary:
        raise ValueError(f"chunk {chunk_id!r} crosses its enclosing boundary")
    if any(data[end:next_offset]):
        raise ValueError(f"chunk {chunk_id!r} has nonzero alignment padding")
    return chunk_id, start, end, next_offset


def parse_images(data: bytes) -> list[ParsedImage]:
    """Parse the exact AGNB 0.1 image form without writer helper reuse."""

    if len(data) < 12 or data[:4] != b"RIFF" or data[8:12] != b"AGNB":
        raise ValueError("not a RIFF AGNB file")
    if struct.unpack_from("<I", data, 4)[0] != len(data) - 8:
        raise ValueError("RIFF size does not match the physical file")

    offset = 12
    chunk_id, start, end, offset = _chunk(data, offset, len(data))
    if chunk_id != b"VERS" or data[start:end] != b"\x00\x01":
        raise ValueError("container is not canonical AGNB image version 0.1")

    images: list[ParsedImage] = []
    seen_ids: set[int] = set()
    while offset < len(data):
        chunk_id, list_start, list_end, offset = _chunk(
            data,
            offset,
            len(data),
        )
        if chunk_id != b"LIST" or data[list_start : list_start + 4] != b"BUFR":
            raise ValueError("expected LIST BUFR image record")
        nested_offset = list_start + 4
        nested: list[tuple[bytes, int, int, int]] = []
        while nested_offset < list_end:
            item = _chunk(data, nested_offset, list_end)
            nested.append(item)
            nested_offset = item[3]
        if nested_offset != list_end:
            raise ValueError("record does not end at its LIST boundary")
        if [item[0] for item in nested] != [b"BHDR", b"IMAG", b"DATA"]:
            raise ValueError("record must contain BHDR, IMAG, DATA in order")

        _id, payload_start, payload_end, _next = nested[0]
        if payload_end - payload_start != 2:
            raise ValueError("BHDR must contain one u16 buffer ID")
        buffer_id = struct.unpack_from("<H", data, payload_start)[0]
        if buffer_id == 0xFFFF or buffer_id in seen_ids:
            raise ValueError(f"invalid or duplicate buffer ID: {buffer_id:#x}")
        seen_ids.add(buffer_id)

        _id, payload_start, payload_end, _next = nested[1]
        if payload_end - payload_start != 5:
            raise ValueError("IMAG must contain width, height, and format")
        width, height, image_format = struct.unpack_from(
            "<HHB",
            data,
            payload_start,
        )
        if not width or not height or image_format != 1:
            raise ValueError("unsupported or invalid IMAG metadata")

        _id, payload_start, payload_end, _next = nested[2]
        payload = data[payload_start:payload_end]
        if len(payload) != width * height:
            raise ValueError("DATA length disagrees with RGBA2222 dimensions")
        images.append(ParsedImage(buffer_id, width, height, image_format, payload))

    if offset != len(data) or not images:
        raise ValueError("container has no complete image records")
    return images


def verify_container(data: bytes, records: list[ImageRecord]) -> None:
    """Require parsed records and payload bytes to match an ordered catalog."""

    parsed = parse_images(data)
    if len(parsed) != len(records):
        raise ValueError(f"parsed {len(parsed)} records; expected {len(records)}")
    for index, (actual, expected) in enumerate(zip(parsed, records, strict=True)):
        identity = (actual.buffer_id, actual.width, actual.height, actual.image_format)
        wanted = (expected.buffer_id, expected.width, expected.height, 1)
        if identity != wanted:
            raise ValueError(
                f"record {index} metadata is {identity}; expected {wanted}"
            )
        if actual.payload != expected.payload_path.read_bytes():
            raise ValueError(f"record {index} payload differs: {expected.name}")
