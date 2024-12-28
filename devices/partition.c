#include "partition.h"
#include "fs/fat/file_system.h"
#include "kernel/kprintf.h"
#include "kernel/trace.h"
#include <stdint.h>

typedef struct partition_table_entry_t {
  uint8_t indicator;
  uint8_t start_chs[3];
  uint8_t type;
  uint8_t end_chs[3];
  uint32_t sector;
  uint32_t size;
} __attribute__((packed)) partition_table_entry_t;

typedef struct mbr_t {
  uint8_t code_area[446];
  partition_table_entry_t partitions[4];
  uint16_t signature;
} mbr_t;

static const char *type_names[256] = {
    [0x00] = "Empty",
    [0x01] = "FAT12",
    [0x02] = "XENIX root",
    [0x03] = "XENIX usr",
    [0x04] = "FAT16 <32M",
    [0x05] = "Extended",
    [0x06] = "FAT16",
    [0x07] = "HPFS/NTFS",
    [0x08] = "AIX",
    [0x09] = "AIX bootable",
    [0x0a] = "OS/2 Boot Manager",
    [0x0b] = "W95 FAT32",
    [0x0c] = "W95 FAT32 (LBA)",
    [0x0e] = "W95 FAT16 (LBA)",
    [0x0f] = "W95 Ext'd (LBA)",
    [0x10] = "OPUS",
    [0x11] = "Hidden FAT12",
    [0x12] = "Compaq diagnostics",
    [0x14] = "Hidden FAT16 <32M",
    [0x16] = "Hidden FAT16",
    [0x17] = "Hidden HPFS/NTFS",
    [0x18] = "AST SmartSleep",
    [0x1b] = "Hidden W95 FAT32",
    [0x1c] = "Hidden W95 FAT32 (LBA)",
    [0x1e] = "Hidden W95 FAT16 (LBA)",
    [0x24] = "NEC DOS",
    [0x39] = "Plan 9",
    [0x3c] = "PartitionMagic recovery",
    [0x40] = "Venix 80286",
    [0x41] = "PPC PReP Boot",
    [0x42] = "SFS",
    [0x4d] = "QNX4.x",
    [0x4e] = "QNX4.x 2nd part",
    [0x4f] = "QNX4.x 3rd part",
    [0x50] = "OnTrack DM",
    [0x51] = "OnTrack DM6 Aux1",
    [0x52] = "CP/M",
    [0x53] = "OnTrack DM6 Aux3",
    [0x54] = "OnTrackDM6",
    [0x55] = "EZ-Drive",
    [0x56] = "Golden Bow",
    [0x5c] = "Priam Edisk",
    [0x61] = "SpeedStor",
    [0x63] = "GNU HURD or SysV",
    [0x64] = "Novell Netware 286",
    [0x65] = "Novell Netware 386",
    [0x70] = "DiskSecure Multi-Boot",
    [0x75] = "PC/IX",
    [0x80] = "Old Minix",
    [0x81] = "Minix / old Linux",
    [0x82] = "Linux swap / Solaris",
    [0x83] = "Linux",
    [0x84] = "OS/2 hidden C: drive",
    [0x85] = "Linux extended",
    [0x86] = "NTFS volume set",
    [0x87] = "NTFS volume set",
    [0x88] = "Linux plaintext",
    [0x8e] = "Linux LVM",
    [0x93] = "Amoeba",
    [0x94] = "Amoeba BBT",
    [0x9f] = "BSD/OS",
    [0xa0] = "IBM Thinkpad hibernation",
    [0xa5] = "FreeBSD",
    [0xa6] = "OpenBSD",
    [0xa7] = "NeXTSTEP",
    [0xa8] = "Darwin UFS",
    [0xa9] = "NetBSD",
    [0xab] = "Darwin boot",
    [0xb7] = "BSDI fs",
    [0xb8] = "BSDI swap",
    [0xbb] = "Boot Wizard hidden",
    [0xbe] = "Solaris boot",
    [0xbf] = "Solaris",
    [0xc1] = "DRDOS/sec (FAT-12)",
    [0xc4] = "DRDOS/sec (FAT-16 < 32M)",
    [0xc6] = "DRDOS/sec (FAT-16)",
    [0xc7] = "Syrinx",
    [0xda] = "Non-FS data",
    [0xdb] = "CP/M / CTOS / ...",
    [0xde] = "Dell Utility",
    [0xdf] = "BootIt",
    [0xe1] = "DOS access",
    [0xe3] = "DOS R/O",
    [0xe4] = "SpeedStor",
    [0xeb] = "BeOS fs",
    [0xee] = "EFI GPT",
    [0xef] = "EFI (FAT-12/16/32)",
    [0xf0] = "Linux/PA-RISC boot",
    [0xf1] = "SpeedStor",
    [0xf4] = "SpeedStor",
    [0xf2] = "DOS secondary",
    [0xfd] = "Linux raid autodetect",
    [0xfe] = "LANstep",
    [0xff] = "BBT",
};

void read_partition_table(block_t *block) {
  char buffer[BLOCK_SIZE_SECTOR];
  char read_buffer[BLOCK_SIZE_SECTOR];

  block_read(block, 0, buffer);
  mbr_t *mbr = (mbr_t *)buffer;

  // TODO: Check MBR signature

  for (size_t i = 0; i < 4; i++) {
    partition_table_entry_t *p = &mbr->partitions[i];
    TRACE("PARTITION", 1, "table: %d, indicator: %d, type: %s, sector: %d, size: %d", i, p->indicator,
          type_names[p->type], p->sector, p->size);

    if (p->size == 0) {
      continue;
    }

    char name[16];
    ksnprintf(name, 16, "%s%d", block->name, i);
    block_t *block_partition = block_register(block->device, name, p->sector, p->size, block->read, block->write);
    block_read(block_partition, 0, read_buffer);

    // for (size_t i = 0; i < BLOCK_SIZE_SECTOR; i++) {
    //   kprintf("%d ", read_buffer[i]);
    // }

    // for (size_t i = 0; i < BLOCK_SIZE_SECTOR; i++) {
    //   read_buffer[i] = i;
    // }

    // block_write(block_partition, 0, read_buffer);

    // TODO: Hardcoded, need to support partitionless disk
    file_system_init(block_partition, FILE_SYSTEM_FAT);
  }

  TRACE("PARTITION", 1, "signature %x", mbr->signature);
}