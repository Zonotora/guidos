# Partition

A *partition table* is a table that contains data of the partitions on a disk.

<div style="text-align: center;"><img src="partition_table.svg"></div>

## MBR partition table

The *Master Boot Record (MBR)* is always located on the first sector of a hard disk. It contains the partition table for the disk. The partition table comprises 64 bytes in total of the 512-byte sector.

| Addresses (within MBR sector) | Length (bytes) | Description |
|-|-|-|
|0x000-0X1BD|446|Code area|
|0x1BE-0X1FD|64|Master partition table|
|0x1FE-0X1FF|2|Boot record signature|



| Addresses (within partition table) | Length (bytes) | Description             |
|-|-|-|
|0 |1|Boot indicator (80h = active)|
|1-3|3|Starting CHS values|
|4 |1|Partition-type descriptor|
|5-7|3|Ending CHS values|
|8-11|4|Starting sector|
|12-15|4|Partition size (in sectors)|

| Addresses (within MBR sector) | Length (bytes) | Table entry             |
|---------------|----------------------------|-----------------------------------------|
| 0x1BE - 0x1CD | 16                     | Primary partition 1 |
| 0x1CE - 0x1DD | 16                     | Primary partition 2 |
| 0x1DE - 0x1ED | 16                     | Primary partition 3 |
| 0x1EE - 0x1FD | 16                     | Primary partition 4 |


## GPT partition table