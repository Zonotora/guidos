#ifndef UTILS_H
#define UTILS_H

#include "../cpu/types.h"

void memory_copy(s8 *source, s8 *dest, s32 nbytes);
void int_to_ascii(s32 n, s8 str[]);

#endif