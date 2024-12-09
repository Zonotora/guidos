#ifndef KERNEL_TRACE_H
#define KERNEL_TRACE_H

#include "drivers/serial.h"
#include "kprintf.h"
#include "libc/string.h"

#ifdef DEBUG
#define TRACE(group, id, msg, ...)                                                                                     \
  ({                                                                                                                   \
    char varargs_buf[512];                                                                                             \
    char printf_buf[1024];                                                                                             \
    tkprintf(varargs_buf, msg, __VA_ARGS__);                                                                           \
    tkprintf(printf_buf, "%s:%d %s.%d %s\n", __FILE__, __LINE__, group, id, varargs_buf);                              \
    serial_write(printf_buf);                                                                                          \
  })
#else
#define TRACE(group, id, msg, ...)
#endif

#endif