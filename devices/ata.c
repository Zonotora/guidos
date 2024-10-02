#include "arch/x86/ports.h"
#include "arch/x86/timer.h"
#include "drivers/screen.h"
#include <stddef.h>
#include <stdint.h>

#define PORT_BASE_PRIMARY 0x1f0
#define PORT_BASE_SECONDARY 0x170

#define PORT_DATA(CHANNEL) ((CHANNEL)->port_base + 0)
#define PORT_ERROR(CHANNEL) ((CHANNEL)->port_base + 1)
#define PORT_SECTORCOUNT(CHANNEL) ((CHANNEL)->port_base + 2)
#define PORT_LBA_LO(CHANNEL) ((CHANNEL)->port_base + 3)
#define PORT_LBA_MID(CHANNEL) ((CHANNEL)->port_base + 4)
#define PORT_LBA_HI(CHANNEL) ((CHANNEL)->port_base + 5)
#define PORT_DRIVE_SELECT(CHANNEL) ((CHANNEL)->port_base + 6)
#define PORT_STATUS(CHANNEL) ((CHANNEL)->port_base + 7)
#define PORT_COMMAND(CHANNEL) ((CHANNEL)->port_base + 7)

#define SELECT_MASTER 0xa0
#define SELECT_SLAVE 0xb0

#define CMD_IDENTIFY 0xEC

#define STATUS_BSY 0x80
#define STATUS_DRQ 0x08
#define STATUS_ERR 0x01

#define BLOCK_SIZE_SECTOR 512

#define MAX_BUSY_WAIT_TIME 3000

// Support the two "legacy" ATA channels found in a standard PC
#define N_CHANNELS 2
// A channel has two devices, the master and the slave
#define N_DEVICES_PER_CHANNEL 2

typedef struct ata_disk {
  char name[8];
  int device;
  struct ata_channel *channel;
} ata_disk;

typedef struct ata_channel {
  char name[8];
  uint16_t port_base;
  uint8_t irq;
  ata_disk devices[N_DEVICES_PER_CHANNEL];
} ata_channel;

static ata_channel channels[N_CHANNELS];

void ata_select(ata_disk *disk);

void ata_init() {
  // Initialize channels
  kprint("setting up ata channels\n");
  for (size_t i = 0; i < N_CHANNELS; i++) {
    ata_channel *channel = &channels[i];

    // Set base port address and IRQ
    switch (i) {
    case 0:
      channel->port_base = PORT_BASE_PRIMARY;
      channel->irq = 14 + 0x20;
      break;
    case 1:
      channel->port_base = PORT_BASE_SECONDARY;
      channel->irq = 15 + 0x20;
      break;

    default:
      break;
    }

    kprint("setting up devices for channel: \n");
    // Initialize devices.
    for (size_t i = 0; i < N_DEVICES_PER_CHANNEL; i++) {
      ata_disk *device = &channel->devices[i];

      device->channel = channel;
      device->device = i;
    }

    // Read hard disk identity information.
    for (size_t i = 0; i < N_DEVICES_PER_CHANNEL; i++) {
      ata_disk *device = &channel->devices[i];
      ata_select(device);
    }

    // Register interrupt handler.

    // Reset hardware.

    // Check if device is an ATA hard disk.
  }
}

void ata_select(ata_disk *disk) {
  ata_channel *channel = disk->channel;
  uint8_t sector[BLOCK_SIZE_SECTOR];

  kprint("selecting disk\n");

  // To use the IDENTIFY command, we send SELECT_MASTER or SELECT_SLAVE
  // to the DRIVE_SELECT port. This will select as drive.
  port_byte_out(PORT_DRIVE_SELECT(channel), SELECT_MASTER);

  // Then set the SECTORCOUNT, LBA_LO, LBA_MID and LBA_HI ports to 0.
  port_byte_out(PORT_SECTORCOUNT(channel), 0);
  port_byte_out(PORT_LBA_LO(channel), 0);
  port_byte_out(PORT_LBA_MID(channel), 0);
  port_byte_out(PORT_LBA_HI(channel), 0);

  // Then send the IDENTIFY command (CMD_IDENTIFY) to the command port.
  port_byte_out(PORT_COMMAND(channel), CMD_IDENTIFY);

  // Then read the status port (this is the same as the command port).
  uint8_t status = port_byte_in(PORT_STATUS(channel));
  // If the status is 0, the drive does not exist.
  if (status == 0) {
    // Does not exist
    return;
  }
  // For any other value, poll the status port until bit 7 clears.
  for (size_t i = 0; i < MAX_BUSY_WAIT_TIME; i++) {
    status = port_byte_in(PORT_STATUS(channel));
    if (!(status & STATUS_BSY)) {
      // Some ATAPI drives do not follow spec... So we need to check the
      // LBA_MID and LBA_HI ports to see if they are non-zero. If they are
      // the drive is not ATA.
      uint8_t lba_mid = port_byte_in(PORT_LBA_MID(channel));
      uint8_t lba_hi = port_byte_in(PORT_LBA_HI(channel));
      if (lba_mid > 0 || lba_hi > 0) {
        kprint("not an ata device\n");
        break;
      }

      status = port_byte_in(PORT_STATUS(channel));
      if (status & STATUS_DRQ) {
        // Data is ready to be sent. Read 256 16-bit values from the data port
        // and store that information.
        insw(PORT_COMMAND(channel), sector, BLOCK_SIZE_SECTOR / 2);
        kprint(sector);
        kprint("data is ready to be sent\n");
        break;
      } else if (status & STATUS_ERR) {
        // Failed to read disk.
        kprint("failed to read disk\n");
        break;
      }
    }

    timer_msleep(10);
  }
}