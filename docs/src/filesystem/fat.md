# FAT



## Creating a virtual disk

```shell
dd if=/dev/zero of=ramdisk.img count=30000
```

```shell
fdisk ramdisk.img
```
Press [n, ENTER, ENTER, ENTER, t, 6, w]


```
mkfs.fat --offset 2048 ramdisk.img
```

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



## File Allocation Table (FAT)


- If the file size is larger than the sector size, file data is spanning over multiple sectors in the cluster.
- If the file size is larger than the cluster size, file data is spanning over multiple clusters in the cluster chain.

On entries in the FAT
- Value of the entry is the cluster number of the next cluster following this corresponding cluster.

## Inode
The inode is a data structure in a Unix-style file system that describes a file-system object such as a file or a directory. Each inode stores the attributes and disk block locations of the object's data.


## References
- [https://wiki.osdev.org/FAT](https://wiki.osdev.org/FAT)
- [http://elm-chan.org/docs/fat_e.html](http://elm-chan.org/docs/fat_e.html)
