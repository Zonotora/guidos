#ifndef DEVICES_BLOCK_H
#define DEVICES_BLOCK_H

#include <stdint.h>

#define BLOCK_SIZE_SECTOR 512

typedef void (*block_read_t)(void *device, uint32_t sector_index, void *buffer);
typedef void (*block_write_t)(void *device, uint32_t sector_index, void *buffer);

// A block device is a special file that provides buffered access to a hardware device.
typedef struct block_t {
  char name[16];
  uint32_t start;
  uint32_t size;
  block_read_t read;
  block_write_t write;
  void *device;
} block_t;

block_t block_register(const void *device, const char *name, const uint32_t start, const uint32_t size,
                       block_read_t read, block_write_t write);
void block_read(block_t *block, uint32_t sector, void *buffer);
void block_write(block_t *block, uint32_t sector, void *buffer);

#endif