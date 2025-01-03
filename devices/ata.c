#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/x86/isr.h"
#include "arch/x86/ports.h"
#include "arch/x86/timer.h"
#include "block.h"
#include "drivers/screen.h"
#include "kernel/kprintf.h"
#include "kernel/trace.h"
#include "partition.h"

#define PORT_DATA(CHANNEL) ((CHANNEL)->port_base + 0)
#define PORT_ERROR(CHANNEL) ((CHANNEL)->port_base + 1)
#define PORT_SECTORCOUNT(CHANNEL) ((CHANNEL)->port_base + 2)
#define PORT_LBA_LO(CHANNEL) ((CHANNEL)->port_base + 3)
#define PORT_LBA_MID(CHANNEL) ((CHANNEL)->port_base + 4)
#define PORT_LBA_HI(CHANNEL) ((CHANNEL)->port_base + 5)
#define PORT_DRIVE_SELECT(CHANNEL) ((CHANNEL)->port_base + 6)
#define PORT_STATUS(CHANNEL) ((CHANNEL)->port_base + 7)
#define PORT_COMMAND(CHANNEL) ((CHANNEL)->port_base + 7)

#define PORT_CONTROL(CHANNEL) ((CHANNEL)->port_base + 0x206)
#define PORT_ALTERNATIVE_STATUS(CHANNEL) ((CHANNEL)->port_base + 0x206)

// NOT: The value of Alternate Status is always the same as the
// Regular Status port (0x1F7 on the Primary bus), but reading the
// Alternate Status port does not affect interrupts.

#define SELECT_MASTER 0xa0
#define SELECT_SLAVE 0xb0
#define SELECT_LBA 0x40

#define CMD_IDENTIFY 0xec
#define CMD_READ_SECTORS 0x20
#define CMD_WRITE_SECTORS 0x30

#define STATUS_BSY 0x80
#define STATUS_DRQ 0x08
#define STATUS_ERR 0x01

#define MAX_BUSY_WAIT_TIME 3000

// Support the two "legacy" ATA channels (bus) found in a standard PC.
// The first two buses are called the Primary and Secondary ATA bus.
// They are almost always controlled by the IO ports PORT_BASE_PRIMARY and
// PORT_BASE_SECONDARY unless you change it. If the next two buses exist,
// they are norammly controlled by IO ports 0x1e8-0x1ef and 0x168-0x16f,
// respectively.
#define N_CHANNELS 2
// A channel has two devices, the master and the slave.
#define N_DEVICES_PER_CHANNEL 2

#define PORT_BASE_PRIMARY 0x1f0
#define PORT_BASE_SECONDARY 0x170
// The IRQ for the Primary bus is IRQ14.
#define IRQ_PRIMARY 14
// The IRQ for the Secondary bus is IRQ15.
#define IRQ_SECONDARY 15

// According to specs, the PCI disk controller is supposed to be in
// "Legacy/Compatibility" mode when the system boots. The system "should"
// use these standardized IO port settings defined above.

typedef struct ata_device {
  char name[8];
  int id;
  struct ata_channel *channel;
  bool is_ata_disk;
} ata_device;

typedef struct ata_channel {
  uint16_t port_base;
  uint8_t irq;
  ata_device devices[N_DEVICES_PER_CHANNEL];
} ata_channel;

static ata_channel channels[N_CHANNELS];

void ata_select_device(const ata_device *device) {
  // kprintf("selecting device %d\n", device->id);
  uint16_t id = device->id == 1 ? SELECT_MASTER : SELECT_SLAVE;
  outb(PORT_DRIVE_SELECT(device->channel), id);
  inb(PORT_ALTERNATIVE_STATUS(device->channel));
  // TOOD: We should wait 400 ns here. But, we currently only have msleep.
  timer_msleep(1);
}

// The controller is idle when the BSY and DRQ bits are cleared.
void wait_until_idle(const ata_device *device) {
  for (size_t i = 0; i < 1000; i++) {
    uint16_t status = inb(PORT_ALTERNATIVE_STATUS(device->channel));
    if ((status & (STATUS_BSY | STATUS_DRQ)) == 0) {
      return;
    }
    timer_msleep(10);
  }
}

bool wait_while_busy(const ata_device *device) {
  // For any other value, poll the status port until bit 7 clears.
  for (size_t i = 0; i < MAX_BUSY_WAIT_TIME; i++) {
    uint8_t status = inb(PORT_ALTERNATIVE_STATUS(device->channel));
    if (!(status & STATUS_BSY)) {
      // Some ATAPI drives do not follow spec... So we need to check the
      // LBA_MID and LBA_HI ports to see if they are non-zero. If they are
      // the drive is not ATA.

      // TODO: Where to put this?
      // uint8_t lba_mid = inb(PORT_LBA_MID(device->channel));
      // uint8_t lba_hi = inb(PORT_LBA_HI(device->channel));
      // if (lba_mid > 0 || lba_hi > 0) {
      //   kprintf("not an ata device\n");
      //   return false;
      // }

      status = inb(PORT_ALTERNATIVE_STATUS(device->channel));
      if (status & STATUS_ERR) {
        // Failed to read disk.
        kprintf("failed to read disk\n");
        return false;
      }

      return (status & STATUS_DRQ) != 0;
    }
    timer_msleep(10);
  }
  kprintf("done busy waiting\n");
  return false;
}

static void sector_select(ata_device *device, uint32_t sector_index) {
  wait_until_idle(device);
  ata_select_device(device);
  wait_until_idle(device);

  outb(PORT_SECTORCOUNT(device->channel), 1);
  // The LBA is 28 bits.
  outb(PORT_LBA_LO(device->channel), sector_index);
  outb(PORT_LBA_MID(device->channel), sector_index >> 8);
  outb(PORT_LBA_HI(device->channel), (sector_index >> 16));
  uint8_t sector_data = (sector_index >> 24) & 0xf;

  uint16_t id = device->id == 1 ? SELECT_MASTER : SELECT_SLAVE;
  // SELECT_LBA must be set for LBA28 or LBA48 transfers.
  outb(PORT_DRIVE_SELECT(device->channel), id | sector_data | SELECT_LBA);
}

static void sector_in(const ata_device *device, void *buffer) {
  insl(PORT_DATA(device->channel), buffer, BLOCK_SIZE_SECTOR / 4);
}

static void sector_out(const ata_device *device, void *buffer) {
  outsl(PORT_DATA(device->channel), buffer, BLOCK_SIZE_SECTOR / 4);
}

static void read(void *device, uint32_t sector_index, void *buffer) {
  sector_select((ata_device *)device, sector_index);
  outb(PORT_COMMAND(((ata_device *)device)->channel), CMD_READ_SECTORS);
  if (!wait_while_busy((ata_device *)device)) {
    kprintf("failed to read disk\n");
    return;
  }
  sector_in((ata_device *)device, buffer);
}

static void write(void *device, uint32_t sector_index, void *buffer) {
  sector_select((ata_device *)device, sector_index);
  outb(PORT_COMMAND(((ata_device *)device)->channel), CMD_WRITE_SECTORS);
  if (!wait_while_busy((ata_device *)device)) {
    kprintf("failed to write disk\n");
    return;
  }
  // TOOD: Need delay here (between every write, a.k.a, don't use rep outsl?)
  sector_out((ata_device *)device, buffer);
}

static void reset_channel(ata_channel *channel) {
  bool present[2];

  for (size_t i = 0; i < 2; i++) {
    ata_device *device = &channel->devices[i];

    ata_select_device(device);

    outb(PORT_SECTORCOUNT(channel), 0x55);
    outb(PORT_LBA_LO(channel), 0xaa);

    outb(PORT_SECTORCOUNT(channel), 0xaa);
    outb(PORT_LBA_LO(channel), 0x55);

    outb(PORT_SECTORCOUNT(channel), 0x55);
    outb(PORT_LBA_LO(channel), 0xaa);

    uint16_t sectorcount = inb(PORT_SECTORCOUNT(channel));
    uint16_t lba_lo = inb(PORT_LBA_LO(channel));

    present[i] = (sectorcount == 0x55) && (lba_lo == 0xaa);
  }

  outb(PORT_CONTROL(channel), 0);
  timer_msleep(10);
  outb(PORT_CONTROL(channel), 0x04);
  timer_msleep(10);
  outb(PORT_CONTROL(channel), 0);

  timer_msleep(150);

  if (present[0]) {
    ata_select_device(&channel->devices[0]);
    wait_while_busy(&channel->devices[0]);
  }

  if (present[1]) {
    ata_select_device(&channel->devices[1]);
    for (size_t i = 0; i < 3000; i++) {
      uint16_t sectorcount = inb(PORT_SECTORCOUNT(channel));
      uint16_t lba_lo = inb(PORT_LBA_LO(channel));
      if (sectorcount == 1 && lba_lo == 1) {
        break;
      }
      timer_msleep(10);
    }

    bool status = wait_while_busy(&channel->devices[1]);
  }
}

static uint8_t *swap_byte_order_in_string(uint8_t *s, size_t n) {
  for (size_t i = 0; i < n - 1; i += 2) {
    uint8_t tmp = s[i];
    s[i] = s[i + 1];
    s[i + 1] = tmp;
  }
  return s;
}

// All current BIOSes have standardized the use of the IDENTIFY command
// to detect the existence of ATA bus devices, e.g., PATA, PATAPI, SATAPI, SATA.
void ata_identify_device(ata_device *device) {
  ata_channel *channel = device->channel;
  uint8_t sector[BLOCK_SIZE_SECTOR];

  // To use the IDENTIFY command,
  // 1. Select the correct device.
  wait_until_idle(device);
  ata_select_device(device);
  wait_until_idle(device);

  // 2. Set the SECTORCOUNT, LBA_LO, LBA_MID and LBA_HI ports to 0.
  outb(PORT_SECTORCOUNT(channel), 0);
  outb(PORT_LBA_LO(channel), 0);
  outb(PORT_LBA_MID(channel), 0);
  outb(PORT_LBA_HI(channel), 0);

  // 3. Send the IDENTIFY command (CMD_IDENTIFY) to the command port.
  outb(PORT_COMMAND(channel), CMD_IDENTIFY);

  // 4. Read the status port (this is the same as the command port).
  uint8_t status = inb(PORT_ALTERNATIVE_STATUS(channel));
  // If the status is 0, the drive does not exist.
  if (status == 0) {
    // Does not exist
    kprintf("drive does not exist\n");
    return;
  }

  if (!wait_while_busy(device)) {
    device->is_ata_disk = false;
    return;
  }

  kprintf("reading from ");
  kprintf(device->name);
  kprintf("\n");
  // Data is ready to be sent. Read 256 16-bit values from the data port
  // and store that information.
  sector_in(device, sector);

  uint8_t *serial_number = swap_byte_order_in_string(&sector[10 * 2], 10 * 2);
  sector[20 * 2] = '\0';
  uint8_t *model_number = swap_byte_order_in_string(&sector[27 * 2], 20 * 2);
  sector[47 * 2] = '\0';
  uint32_t capacity = *(uint32_t *)&sector[60 * 2];

  block_t *block = block_register(device, device->name, 0, capacity, read, write);

  read_partition_table(block);
  TRACE("ATA", 1, "capacity: %d, serial_number: %s, model_number: %s", capacity, serial_number, model_number);
}

static void interrupt_handler(registers_t *regs) {
  for (size_t i = 0; i < 2; i++) {
    ata_channel *channel = &channels[i];
    if (regs->int_no != channel->irq) {
      continue;
    }
    inb(PORT_STATUS(channel));
    TRACE("INTERRUPT", 1, "ata", 0);
  }
}

void ata_init() {
  // Initialize channels
  for (size_t ci = 0; ci < N_CHANNELS; ci++) {
    kprintf("setting up channel %d\n", ci);
    ata_channel *channel = &channels[ci];

    // Set base port address and IRQ
    if (ci == 0) {
      channel->port_base = PORT_BASE_PRIMARY;
      channel->irq = IRQ_PRIMARY + 0x20;
    } else {
      channel->port_base = PORT_BASE_SECONDARY;
      channel->irq = IRQ_SECONDARY + 0x20;
    }

    // Initialize devices.
    for (size_t di = 0; di < N_DEVICES_PER_CHANNEL; di++) {
      ata_device *device = &channel->devices[di];

      ksnprintf(device->name, sizeof(device->name), "hd%c", 'a' + ci * 2 + (1 - di));
      device->channel = channel;
      device->id = di;
    }

    // Register interrupt handler.
    register_interrupt_handler(channel->irq, interrupt_handler);

    // Reset hardware.
    // reset_channel(channel);

    // Read hard disk identity information.
    for (size_t di = 0; di < N_DEVICES_PER_CHANNEL; di++) {
      ata_device *device = &channel->devices[di];
      ata_identify_device(device);
    }
  }
}