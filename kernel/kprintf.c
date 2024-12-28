#include <limits.h>
#include <stdint.h>

#include "drivers/screen.h"
#include "kprintf.h"
#include "libc/mem.h"
#include "libc/printf.h"
#include "libc/string.h"

void kprintf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(putchar, format, args);
  va_end(args);
}

void ksnprintf(char *s, size_t n, const char *format, ...) {
  va_list args;
  va_start(args, format);
  vsprintf(s, n, format, args);
  va_end(args);
}