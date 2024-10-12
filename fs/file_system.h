#ifndef FS_FILE_SYSTEM_H
#define FS_FILE_SYSTEM_H

#include "devices/block.h"

typedef enum file_system_type_t {
  FILE_SYSTEM_NONE,
  FILE_SYSTEM_FAT16,
} file_system_type_t;

typedef struct file_system_t {
  block_t *block;
  file_system_type_t type;
} file_system_t;

void file_system_init(block_t *block, file_system_type_t type);

#endif