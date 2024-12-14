# Boot sequence

When an x86-based computer starts, the computer tries to run a BIOS initialization program. The BIOS reads and transfers the first 512 bytes (sector 0) of a bootable device, e.g., hard disk. This sector is called the **Master Boot Record (MBR)**. The MBR usually contains two components:

1. A tiny bootstrapping program, i.e., a programming that starts the operating system.
2. A partition table for the disk device.

However, the BIOS have no idea about any of this, but simply loads the first 512 bytes into memory at `0x7C00`. If the last two bytes are `0x55` and `0xAA` out of the loaded 512 bytes, the BIOS considers it to be valid and jumps to that location to start executing whatever code that is now located there.
Because the MBR may hold a parition table there are two variants to consider:
1. If the MBR is partitionless, i.e., does not have a partition table, the code located should load the next step of the boot process which can be the kernel itself or the next stage in the bootloader sequence.
2. If the MBR holds a partition table, the MBR takes a look at the partition table and finds the only partition marked as active and loads the **boot sector** for that partition, and starts to execute that code. It then follows the same flow as (1). The boot sector is the first sector for a partition as opposed to the first sector for the whole disk.

The bootloader is often divided into different stages as it is often impossible to fit everything you want to do in 512 bytes. It is possible depending on use case, but does not leave much room for special-case handling or useful error messages. So the MBR itself contains the first stage (stage 1) of the bootloader. Due to the tiny size of the MBR, there is not much it can do, but it manages just enough to load another sector from disk that contains additional bootstrap code, e.g., the boot sector of a partition or a hard coded sector you have provided. With the code loaded so far, the bootloader is able to enter stage 2. It proceeds loads all code required to boot the kernel. There are many [variants](https://wiki.osdev.org/Boot_Sequence#Some_methods) here, but for example if the kernel is placed in a file system at the boot partition, stage 2 must know enough about this file system to proceed.

Remember that you can only acccess BIOS and only have access to 640 KiB of RAM in real mode. This implies that if your kernel is bigger than that, you can't load the whole kernel in only real mode! This is unfortunate. The solution is unreal mode! This is not a official processor mode, but rather a technique involving switching between real mode and protected mode in order to access more than 1 MiB while still using the BIOS.

- TODO: memory map

The bootloader described here assumes a couple of things:
1. The kernel is a flat binary, i.e., a binary that dose not retain any structure nor segments. There are no special headers or descriptors that describe where the code and data goes. An alternative would be to use, e.g., the ELF format and have code that can decode that format, jump to, and execture the correct code.
2. The size of the binary in terms of sectors is calculated during the build process so that stage 1 of the bootloader knows how many sectors it should load.
3. There are two stages of the bootloader. Stage 1 is located within the MBR. It loads stage 2. Stage 2 will make the necessary preparations, load all sectors of the kernel below the 1 MiB mark and jump to the main function of the kernel.
4. The kernel is loaded at address `0x100000` which means that unreal mode must be used.

- TODO: A20 line
- TODO: Memory map
- TODO: GDT (temporary?)
- TODO: Paging


## Partition
A **partition table** is a table that contains data of the partitions on a disk.

<div style="text-align: center;"><img src="partition_table.svg"></div>

### MBR partition table

The **Master Boot Record (MBR)** is always located on the first sector of a hard disk. It contains the partition table for the disk. The partition table comprises 64 bytes in total of the 512-byte sector.


<table>
  <thead>
    <tr>
      <th colspan="2">Addresses (within MBR sector)</th>
      <th>Length (bytes)</th>
      <th>Description</th>
    </tr>
    <tr>
      <th>Decimal</th>
      <th>Hex</th>
      <th></th>
      <th></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>0 - 445</td>
      <td>0x000 - 0x1BD</td>
      <td>446</td>
      <td>Code area</td>
    </tr>
    <tr>
      <td>446 - 509</td>
      <td>0x1BE - 0x1FD</td>
      <td>64</td>
      <td>Master partition table</td>
    </tr>
    <tr>
      <td>510 - 511</td>
      <td>0x1FE - 0x1FF</td>
      <td>2</td>
      <td>Boot record signature</td>
    </tr>
  </tbody>
</table>

<table>
  <thead>
    <tr>
      <th colspan="2">Addresses (within MBR sector)</th>
      <th>Length (bytes)</th>
      <th>Table entry</th>
    </tr>
    <tr>
      <th>Decimal</th>
      <th>Hex</th>
      <th></th>
      <th></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>446 - 461</td>
      <td>0x1BE - 0x1CD</td>
      <td>16</td>
      <td>Primary partition 1</td>
    </tr>
    <tr>
      <td>462 - 477</td>
      <td>0x1CE - 0x1DD</td>
      <td>16</td>
      <td>Primary partition 2</td>
    </tr>
    <tr>
      <td>478 - 493</td>
      <td>0x1DE - 0x1ED</td>
      <td>16</td>
      <td>Primary partition 3</td>
    </tr>
    <tr>
      <td>494 - 509</td>
      <td>0x1EE - 0x1FD</td>
      <td>16</td>
      <td>Primary partition 4</td>
    </tr>
  </tbody>
</table>


| Addresses (within partition table) | Length (bytes) | Description             |
|-|-|-|
|0 |1|Boot indicator (80h = active)|
|1-3|3|Starting CHS values|
|4 |1|Partition-type descriptor|
|5-7|3|Ending CHS values|
|8-11|4|Starting sector|
|12-15|4|Partition size (in sectors)|

### GPT partition table

## BIOS Parameter Block (BPB)


The boot record is always placed in the **logical** sector number zero. The first sector of a hard disk is called the Master Boot Record (MBR). In case the storage media is partition, the partition's first sector holds a Volume Boot Record (VBR). The boot record contains code and data mixed together. The **BPB** is data that contains information about how the partition is formatted.

<table>
  <thead>
    <tr>
      <th colspan="2">Offset (within MBR/VBR sector)</th>
      <th>Length (in bytes)</th>
      <th>Description</th>
    </tr>
    <tr>
      <th>Decimal</th>
      <th>Hex</th>
      <th></th>
      <th></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>0</td>
      <td>0x00</td>
      <td>3</td>
      <td>
      </td>
    </tr>
    <tr>
      <td>3</td>
      <td>0x03</td>
      <td>8</td>
      <td>
      </td>
    </tr>
    <tr>
      <td>11</td>
      <td>0x0B</td>
      <td>2</td>
      <td>
      </td>
    </tr>
    <tr>
      <td>13</td>
      <td>0x0D</td>
      <td>1</td>
      <td>Number of sectors per cluster.</td>
    </tr>
    <tr>
      <td>14</td>
      <td>0x0E</td>
      <td>2</td>
      <td>
        Number of reserved sectors. The boot record sectors are included in this
        value.
      </td>
    </tr>
    <tr>
      <td>16</td>
      <td>0x10</td>
      <td>1</td>
      <td>
        Number of File Allocation Tables (FAT's) on the storage media.
      </td>
    </tr>
    <tr>
      <td>17</td>
      <td>0x11</td>
      <td>2</td>
      <td>
        Number of root directory entries.
      </td>
    </tr>
    <tr>
      <td>19</td>
      <td>0x13</td>
      <td>2</td>
      <td>
        The total sectors in the logical volume. If this value is 0, it means
        there are more than 65535 sectors in the volume, and the actual count is
        stored in the Large Sector Count entry at 0x20.
      </td>
    </tr>
    <tr>
      <td>21</td>
      <td>0x15</td>
      <td>1</td>
      <td>
        Media descriptor type. See
        <a
          rel="nofollow"
          class="external text"
          href="https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#BPB20_OFS_0Ah"
          >this</a
        >.
      </td>
    </tr>
    <tr>
      <td>22</td>
      <td>0x16</td>
      <td>2</td>
      <td>Number of sectors per FAT. FAT12/FAT16 only.</td>
    </tr>
    <tr>
      <td>24</td>
      <td>0x18</td>
      <td>2</td>
      <td>Number of sectors per track.</td>
    </tr>
    <tr>
      <td>26</td>
      <td>0x1A</td>
      <td>2</td>
      <td>Number of heads or sides on the storage media.</td>
    </tr>
    <tr>
      <td>28</td>
      <td>0x1C</td>
      <td>4</td>
      <td>
        Number of hidden sectors. (i.e. the LBA of the beginning of the
        partition.)
      </td>
    </tr>
    <tr>
      <td>32</td>
      <td>0x20</td>
      <td>4</td>
      <td>
        Large sector count. This field is set if there are more than 65535
        sectors in the volume, resulting in a value which does not fit in the
        <i>Number of Sectors</i> entry at 0x13.
      </td>
    </tr>
  </tbody>
</table>




- https://manybutfinite.com/post/how-computers-boot-up/
- https://en.wikipedia.org/wiki/Unreal_mode

- https://wiki.osdev.org/Memory_Map_(x86)

- https://wiki.osdev.org/A20_Line
- https://en.wikipedia.org/wiki/A20_line

- https://www.pixelbeat.org/docs/disk/