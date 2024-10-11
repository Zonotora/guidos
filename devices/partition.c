#include "partition.h"
#include "block.h"
#include <stdint.h>

typedef struct partition_table_entry_t {
  uint16_t indicator : 1;
  uint16_t start_chs : 3;
  uint16_t type : 1;
  uint16_t end_chs : 3;
  uint16_t sector : 4;
  uint16_t size : 4;
} partition_table_entry_t __attribute__((packed));

typedef struct mbr_t {
  uint8_t code_area[446];
  partition_table_entry_t partitions[4];
  uint16_t signature;
} mbr_t;

void read_partition_table(block_t *block) {
  //

  char buffer[512];
  block->read(block->device, 0, buffer);
}