#ifndef SCREEN_H
#define SCREEN_H

#include "../cpu/types.h"

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define MAX_CHARACTERS (MAX_ROWS * MAX_COLS)
#define WHITE_ON_BLACK 0x0f
#define RED_ON_WHITE 0xf4

#define VGA_OFFSET_LOW 0x0f
#define VGA_OFFSET_HIGH 0x0e

/* Screen i/o ports */
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

/* Public kernel API */
void clear_screen();
void kprint_at(s8 *message, u8 col, u8 row);
void kprint(s8 *message);
void backspace();

#endif