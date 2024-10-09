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

char to_upper(char c) {
  if (c < 97 || c > 122)
    return c;

  return c - 32;
}

char to_lower(char c) {
  if (c < 65 || c > 90)
    return c;

  return c + 32;
}

static inline char to_hex(uint8_t n) {
  if (n >= 16) {
    return 0;
  }
  char chars[] = {'a', 'b', 'c', 'd', 'e', 'f'};
  if (n >= 10) {
    return chars[n % 10];
  }

  return '0' + n;
}

// static void print_hex()

// Limited version of vprintf() which only supports the following specifiers:
//
// - d/i: Signed decimal integer
// NOT IMPLEMENTED - u: Unsigned decimal integer
// NOT IMPLEMENTED - o: Unsigned octal
// - x: Unsigned hexadecimal integer
// - X: Unsigned hexadecimal integer (uppercase)
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
      uint32_t n = va_arg(arg, uint32_t);
      if (n == INT_MIN) {
        kprint("-2147483648");
        break;
      }

      if (n == 0) {
        kputchar('0');
        break;
      }

      if (n < 0) {
        kputchar('-');
        n = -n;
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
    case 'x':
    case 'X': {
      int n = va_arg(arg, int);
      char buf[8];
      char *buf_p = buf;
      kprint("0x");
      if (n == 0) {
        kputchar('0');
        break;
      }

      while (n) {
        *buf_p = to_hex(n % 16);
        if (*format == 'X') {
          *buf_p = to_upper(*buf_p);
        }
        buf_p++;
        n = n / 16;
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