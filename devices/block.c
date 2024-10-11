#include "block.h"

block_t block_register(const void *device, const char *name, const uint32_t size, block_read_t read,
                       block_write_t write) {
  block_t block;

  block.read = read;
  block.write = write;
  block.device = device;

  return block;
}