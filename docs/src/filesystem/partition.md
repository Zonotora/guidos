# Partition

A *partition table* is a table that contains data of the partitions on a disk.

<div style="text-align: center;"><img src="partition_table.svg"></div>

## MBR partition table

The *Master Boot Record (MBR)* is always located on the first sector of a hard disk. It contains the partition table for the disk. The partition table comprises 64 bytes in total of the 512-byte sector.


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

## GPT partition table