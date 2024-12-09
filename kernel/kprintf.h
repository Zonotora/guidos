#ifndef KERNEL_KPRINTF_H
#define KERNEL_KPRINTF_H

#include <stdarg.h>
#include <stddef.h>

void kprint_at(const char *message, unsigned char col, unsigned char row);
void kprint(const char *message);
void kprintf(const char *format, ...);

void vprintf(char *buf_in, const char *format, va_list arg);
int vsprintf(char *s, size_t n, const char *format, va_list arg);
void snprintf(char *s, size_t n, const char *format, ...);

void tkprintf(char *printf_buf, const char *format, ...);

#endif