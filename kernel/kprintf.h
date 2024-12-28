#ifndef KERNEL_KPRINTF_H
#define KERNEL_KPRINTF_H

#include <stdarg.h>
#include <stddef.h>

void kprintf(const char *format, ...);
void ksnprintf(char *s, size_t n, const char *format, ...);

#endif