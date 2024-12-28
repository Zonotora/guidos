

#ifndef LIBC_PRINTF_H
#define LIBC_PRINTF_H

#include <stdarg.h>
#include <stddef.h>

typedef void (*print_char_fn)(char c);

void vprintf(print_char_fn print_char, const char *format, va_list arg);
int vsprintf(char *s, size_t n, const char *format, va_list arg);

#endif