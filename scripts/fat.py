from dataclasses import dataclass
from collections import namedtuple
import struct
import sys


SECTOR_LENGTH = 512
sectors = []

with open(sys.argv[1], mode="rb") as f:
    content = f.read()
    i = 0
    while i < len(content):
        buffer = content[i : i + SECTOR_LENGTH]
        unpacked = struct.unpack(f"{SECTOR_LENGTH}B", buffer)
        sectors.append(unpacked)
        i += SECTOR_LENGTH


def shift_uint8_tuple(tup):
    s = 0
    n = len(tup)
    for i in range(n):
        s |= tup[i] << (i * 8)
    return s


@dataclass
class Field:
    name: str
    offset: int
    length: int


class Base:
    def __init__(self, fields, buffer):
        self.fields = fields
        for field in fields:
            if field.length == 1:
                value = buffer[field.offset]
            else:
                value = shift_uint8_tuple(
                    buffer[field.offset : field.offset + field.length]
                )
            setattr(self, field.name, value)

    def __str__(self):
        fields = ", ".join(
            f"{field.name}: {getattr(self, field.name)}" for field in self.fields
        )
        return f"{self.__class__.__name__}({fields})"

    def __repr__(self):
        return str(self)


bpb_fields = [
    Field("jump_boot", 0, 3),
    Field("oem_name", 3, 8),
    Field("n_bytes_per_sector", 11, 2),
    Field("n_sectors_per_cluster", 13, 1),
    Field("n_reserved_sectors", 14, 2),
    Field("n_fat", 16, 1),
    Field("n_root_entries", 17, 2),
    Field("small_sector_count", 19, 2),
    Field("media_descriptor", 21, 1),
    Field("n_sectors_per_fat16", 22, 2),
    Field("sectors_per_track", 24, 2),
    Field("n_heads", 26, 2),
    Field("hidden_sectors", 28, 2),
    Field("large_sector_count", 32, 4),
]


class Bpb(Base):
    def __init__(self, buffer):
        super().__init__(bpb_fields, buffer)


partition_fields = [
    Field("indicator", 0, 1),
    Field("start_chs", 1, 3),
    Field("type", 4, 1),
    Field("end_chs", 5, 3),
    Field("sector", 8, 4),
    Field("size", 12, 4),
]


class Partition(Base):
    def __init__(self, buffer):
        super().__init__(partition_fields, buffer)


@dataclass
class Mbr:
    code_area: list[int]
    partitions: list[Partition]
    signature: int


def parse_mbr(buffer):
    code_area = buffer[:446]
    start = 446
    partitions = []
    for i in range(4):
        partition_buffer = buffer[start : start + 16]
        partition = Partition(partition_buffer)
        start += 16
        partitions.append(partition)

    signature = shift_uint8_tuple(buffer[start:])
    mbr = Mbr(code_area, partitions, signature)
    return mbr


mbr = parse_mbr(sectors[0])
# print(Bpb(mbr.code_area))
for p in mbr.partitions:
    print(p)

index = mbr.partitions[0].sector - 1
# print(sectors[index])
# print(mbr.code_area)

for i, s in enumerate(sectors):
    p = False
    for b in s:
        if b != 0:
            p = True
    if p:
        print(i)
        print(s)
