#include "file_system.h"

static file_system_t file_system;

void file_system_init(block_t *block, file_system_type_t type) {
  file_system.block = block;
  file_system.type = type;
}
