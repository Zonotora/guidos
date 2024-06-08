#ifndef STRING_H
#define STRING_H

#include "../cpu/types.h"

void int_to_ascii(int n, char str[]);
void reverse(char s[]);
int strlen(char s[]);
int strcmp(char *s1, char *s2);
void strcat(char *dest, char *src, int size);

#endif