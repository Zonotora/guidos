#ifndef KERNEL_TRACE_H
#define KERNEL_TRACE_H

#include "drivers/serial.h"
#include "kprintf.h"
#include "libc/string.h"

#ifdef DEBUG
#define TRACE(group, id, msg, ...)                                                                                     \
  ({                                                                                                                   \
    serial_printf("%s:%d %s.%d ", __FILE__, __LINE__, group, id);                                                      \
    serial_printf(msg, __VA_ARGS__);                                                                                   \
    serial_printf("\n");                                                                                               \
  })

#define TRACE_STRUCT(group, id, s)                                                                                     \
  ({                                                                                                                   \
    serial_printf("%s:%d %s.%d ", __FILE__, __LINE__, group, id);                                                      \
    __builtin_dump_struct(s, &serial_printf);                                                                          \
    serial_printf("\n");                                                                                               \
  })
#else
#define TRACE(group, id, msg, ...)
#define TRACE_STRUCT(group, id, s)
#endif

#endif