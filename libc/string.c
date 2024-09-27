#include "string.h"

void int_to_ascii(int n, char str[]) {
  int i, sign;
  if ((sign = n) < 0)
    n = -n;
  i = 0;
  do {
    str[i++] = n % 10 + '0';
  } while ((n /= 10) > 0);

  if (sign < 0)
    str[i++] = '-';
  str[i] = '\0';

  reverse(str);
}

void reverse(char s[]) {
  int c, i, j;
  for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}

int strlen(char s[]) {
  int i = 0;
  while (s[i] != '\0')
    ++i;
  return i;
}

int strcmp(char *s1, char *s2) {
  int n1 = strlen(s1);
  int n2 = strlen(s2);
  if (n1 != n2)
    return 0;
  for (int i = 0; i < n1; i++) {
    if (s1[i] != s2[i])
      return 0;
  }
  return 1;
}

void strcat(char *dest, char *src, int size) {
  int i = 0;
  while (dest[i] != 0)
    i += 1;
  int n = strlen(src);
  while (*src != 0 && i < size) {
    dest[i] = *src;
    src += 1;
  }
  dest[size - 1] = 0;
}
