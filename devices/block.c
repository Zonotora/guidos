#include "block.h"
#include "drivers/screen.h"
#include <stddef.h>

typedef struct block_array_t {
  size_t length;
  block_t data[10];
} block_array_t;

block_array_t blocks;

block_t *block_register(const void *device, const char *name, const uint32_t start, const uint32_t size,
                        block_read_t read, block_write_t write) {

  if (blocks.length >= 10) {
    kprint("too many block devices");
    return 0;
  }

  block_t *block = &blocks.data[blocks.length++];

  for (size_t i = 0; i < 16 - 1 && *name; i++) {
    block->name[i] = *name++;
  }
  block->name[16 - 1] = '\0';
  block->start = start;
  block->size = size;
  block->read = read;
  block->write = write;
  block->device = device;

  return block;
}

void block_read(block_t *block, uint32_t sector, void *buffer) {
  uint32_t offset = block->start + sector;
  if (block->start <= offset && offset <= block->start + block->size) {
    block->read(block->device, offset, buffer);
  } else {
    kprintf("reading from a sector outside block device: %s", block->name);
  }
}

void block_write(block_t *block, uint32_t sector, void *buffer) {
  uint32_t offset = block->start + sector;
  if (block->start <= offset && offset <= block->start + block->size) {
    block->write(block->device, offset, buffer);
  } else {
    kprintf("writing to a sector outside block device: %s", block->name);
  }
}