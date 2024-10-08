#include "screen.h"
#include "arch/x86/ports.h"
#include "libc/mem.h"
#include "libc/string.h"
#include <limits.h>
#include <stdint.h>

char *const VGA = (char *const)VIDEO_ADDRESS;

// Local functions
unsigned short get_offset(unsigned char row, unsigned char col) {
  unsigned short offset = (MAX_COLS * row) + col;
  return offset;
}

void set_cursor(unsigned char row, unsigned char col) {
  unsigned short offset = get_offset(row, col);
  port_byte_out(REG_SCREEN_CTRL, VGA_OFFSET_HIGH);
  port_byte_out(REG_SCREEN_DATA, offset >> 8);
  port_byte_out(REG_SCREEN_CTRL, VGA_OFFSET_LOW);
  port_byte_out(REG_SCREEN_DATA, offset & 0xFF);
}

unsigned short get_cursor() {
  unsigned short offset = 0;
  port_byte_out(REG_SCREEN_CTRL, VGA_OFFSET_HIGH);
  offset += port_byte_in(REG_SCREEN_DATA) << 8;
  port_byte_out(REG_SCREEN_CTRL, VGA_OFFSET_LOW);
  offset += port_byte_in(REG_SCREEN_DATA);
  return offset;
}

unsigned short scroll() {
  int n_bytes = 2 * (MAX_CHARACTERS - MAX_COLS);
  memory_copy(&VGA[2 * MAX_COLS], &VGA[0], n_bytes);
  char buf[MAX_COLS + 1];
  for (int i = 0; i < MAX_COLS; i++) {
    buf[i] = ' ';
  }
  buf[MAX_COLS] = 0;
  int row = MAX_ROWS - 1;
  int col = 0;
  unsigned short offset = get_offset(row, col);
  kprint_at((char *)&buf, row, col);
  return offset;
}

unsigned short write_vga(unsigned char c, unsigned char modifier, unsigned short offset) {
  if (offset >= MAX_CHARACTERS) {
    offset = scroll();
  }
  VGA[offset * 2] = c;
  VGA[offset * 2 + 1] = modifier;
  return offset;
}

// Global functions
void clear_screen() {
  for (int i = 0; i < MAX_CHARACTERS; i++) {
    write_vga(0, 0, i);
  }
  set_cursor(0, 0);
}

void kputchar(char c) {
  unsigned short offset = get_cursor();
  unsigned char row = offset / MAX_COLS;
  unsigned char col = offset % MAX_COLS;
  if (c == '\n') {
    row += 1;
    col = 0;
    set_cursor(row, col);
    return;
  }
  offset = write_vga(c, WHITE_ON_BLACK, offset);
  offset += 1;
  row = offset / MAX_COLS;
  col = offset % MAX_COLS;
  set_cursor(row, col);
}

void kprint_at(const char *message, unsigned char row, unsigned char col) {
  set_cursor(row, col);
  while (*message) {
    kputchar(*message);
    message++;
  }
}

void kprint(const char *message) {
  while (*message) {
    kputchar(*message);
    message++;
  }
}

// Limited version of vprintf() which only supports the following specifiers:
//
// - d/i: Signed decimal integer
// NOT IMPLEMENTED - u: Unsigned decimal integer
// NOT IMPLEMENTED - o: Unsigned octal
// NOT IMPLEMENTED - x: Unsigned hexadecimal integer
// NOT IMPLEMENTED - X: Unsigned hexadecimal integer (uppercase)
// NOT IMPLEMENTED - c: Character
// NOT IMPLEMENTED - s: String of characters
// NOT IMPLEMENTED - p: Pointer address
// NOT IMPLEMENTED - %: Literal '%'
void kvprintf(const char *format, va_list arg) {
  while (*format) {

    // Print character
    if (*format != '%') {
      kputchar(*format);
      format++;
      continue;
    }

    // Otherwise parse specifier
    // *format='%', increment to get specifier
    format++;

    if (!*format)
      return;

    switch (*format) {
    case 'd':
    case 'i': {
      int n = va_arg(arg, int);
      if (n == INT_MIN) {
        kprint("-2147483648");
        break;
      }

      if (n < 0) {
        kputchar('-');
      }

      char buf[10];
      char *buf_p = buf;

      while (n) {
        *buf_p++ = '0' + n % 10;
        n = n / 10;
      }
      while (buf_p != buf) {
        kputchar(*--buf_p);
      }

    } break;

    default:
      break;
    }

    format++;
  }
}

void kprintf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  kvprintf(format, args);
  va_end(args);
}

void backspace() {
  int offset = get_cursor() - 1;
  if (offset < 0) {
    offset = 0;
  }
  unsigned char row = offset / MAX_COLS;
  unsigned char col = offset % MAX_COLS;
  kprint_at(" ", row, col);
  set_cursor(row, col);
}