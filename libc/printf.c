#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

#include "printf.h"

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

void vprintf(print_char_fn print_char, const char *format, va_list arg) {
  while (*format) {

    // Print character
    if (*format != '%') {
      print_char(*format);
      format++;
      continue;
    }

    // Otherwise parse specifier
    // *format='%', increment to get specifier
    format++;

    if (!*format)
      return;

    bool is_long = *format == 'l';
    if (is_long) {
      format++;
    }
    bool is_long_long = *format == 'l';
    if (is_long_long) {
      format++;
    }

    if (!*format || ((is_long || is_long_long) && *format == ' '))
      return;

    switch (*format) {
    case 's': {
      char *s = va_arg(arg, char *);
      while (*s) {
        print_char(*s++);
      }
    } break;

    case 'u': {
      uint32_t n = va_arg(arg, uint32_t);

      char buf[10];
      char *buf_p = buf;

      if (n == 0) {
        print_char('0');
        break;
      }

      while (n) {
        *buf_p++ = '0' + n % 10;
        n = n / 10;
      }
      while (buf_p != buf) {
        print_char(*--buf_p);
      }

    } break;
    case 'd':
    case 'i': {
      int32_t n = va_arg(arg, int32_t);
      if (n == INT_MIN) {
        char *s = "-2147483648";
        while (*s) {
          print_char(*s++);
        }
        break;
      }

      if (n == 0) {
        print_char('0');
        break;
      }

      if (n < 0) {
        print_char('-');
        n = -n;
      }

      char buf[10];
      char *buf_p = buf;

      while (n) {
        *buf_p++ = '0' + n % 10;
        n = n / 10;
      }
      while (buf_p != buf) {
        print_char(*--buf_p);
      }

    } break;
    case 'x':
    case 'X': {
      uint32_t n = va_arg(arg, uint32_t);
      char buf[8];
      char *buf_p = buf;
      print_char('0');
      print_char('x');
      if (n == 0) {
        print_char('0');
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
        print_char(*--buf_p);
      }

    } break;

    default:
      break;
    }

    format++;
  }
}

int vsprintf(char *s, size_t n, const char *format, va_list arg) {
  size_t i = 0;
  while (*format && i < n) {
    if (*format != '%') {
      *s++ = *format++;
      i++;
      continue;
    }

    // Otherwise parse specifier
    // *format='%', increment to get specifier
    format++;

    if (!*format)
      return i;

    switch (*format) {
    case 'c': {
      // TODO: Type of every va_arg
      int32_t n = va_arg(arg, int32_t);
      *s++ = n;
      i++;
    } break;
    case 'd': {
      int32_t n = va_arg(arg, int32_t);

      if (n == 0 && (int32_t)(i + 1) < n) {
        *s++ = '0';
        i += 1;
        break;
      }

      if ((int32_t)(i + 11) >= n) {
        break;
      }

      if (n == INT_MIN) {
        char *int_min = "-2147483648";
        while (*int_min) {
          *s++ = *int_min++;
          i += 11;
        }
        break;
      }

      if (n < 0) {
        *s++ = '-';
        i += 1;
        n = -n;
      }

      char buf[10];
      char *buf_p = buf;

      while (n) {
        *buf_p++ = '0' + n % 10;
        n = n / 10;
      }
      while (buf_p != buf) {
        *s++ = *--buf_p;
        i += 1;
      }

    } break;

    default:
      break;
    }

    format++;
  }

  if (i == n) {
    *--s = 0;
  } else {
    *s++ = 0;
  }
  return i;
}