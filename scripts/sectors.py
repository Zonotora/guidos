import math
import os
import struct
import sys

filename = sys.argv[1]
bin_filename = sys.argv[2]
stat = os.stat(filename)
sectors = int(math.ceil(stat.st_size / 512))
if sectors > (0x80000 - 0x1000) // 512:
    raise Exception("Too large")

with open(bin_filename, "rb") as f:
    f.seek(446 - 2)
    bytes = struct.unpack("<H", f.read(2))
    assert next(iter(bytes)) == 0xABCD, bytes

with open(bin_filename, "rb+") as f:
    f.seek(446 - 2)
    f.write(struct.pack("<H", sectors))
