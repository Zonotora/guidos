import argparse
import math
import os
import struct

parser = argparse.ArgumentParser()
parser.add_argument("--stage1", required=True)
parser.add_argument("--stage2", required=True)
parser.add_argument("--kernel", required=True)
args = parser.parse_args()

stage2_stat = os.stat(args.stage2)
stage2_sectors = int(math.ceil(stage2_stat.st_size / 512))

kernel_stat = os.stat(args.kernel)
kernel_sectors = int(math.ceil(kernel_stat.st_size / 512))
print(stage2_sectors, kernel_sectors)

if stage2_sectors > (0x1000 - 0x500) // 512:
    raise Exception("Stage 2 too large")

if kernel_sectors > (0x80000 - 0x1000) // 512:
    raise Exception("Kernel too large")


with open(args.stage1, "rb") as f:
    f.seek(446 - 2 - 2)
    stage2 = struct.unpack("<H", f.read(2))
    kernel = struct.unpack("<H", f.read(2))
    assert next(iter(stage2)) == 0xDEAD, stage2
    assert next(iter(kernel)) == 0xBEEF, kernel

with open(args.stage1, "rb+") as f:
    f.seek(446 - 2 - 2)
    f.write(struct.pack("<H", stage2_sectors))
    f.write(struct.pack("<H", kernel_sectors))
