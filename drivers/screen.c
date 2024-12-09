#include "screen.h"
#include "arch/x86/ports.h"
#include "libc/mem.h"
#include "libc/string.h"
#include <limits.h>
#include <stdint.h>

#define VIDEO_ADDRESS 0xb8000
#define MAX_CHARACTERS (MAX_ROWS * MAX_COLS)
#define WHITE_ON_BLACK 0x0f
#define RED_ON_WHITE 0xf4

#define VGA_OFFSET_LOW 0x0f
#define VGA_OFFSET_HIGH 0x0e

/* Screen i/o ports */
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

char *const VGA = (char *const)VIDEO_ADDRESS;

void print_at(const char *message, unsigned char row, unsigned char col);

// Local functions
unsigned short get_offset(unsigned char row, unsigned char col) {
  unsigned short offset = (MAX_COLS * row) + col;
  return offset;
}

void set_cursor(unsigned char row, unsigned char col) {
  unsigned short offset = get_offset(row, col);
  outb(REG_SCREEN_CTRL, VGA_OFFSET_HIGH);
  outb(REG_SCREEN_DATA, offset >> 8);
  outb(REG_SCREEN_CTRL, VGA_OFFSET_LOW);
  outb(REG_SCREEN_DATA, offset & 0xFF);
}

unsigned short get_cursor() {
  unsigned short offset = 0;
  outb(REG_SCREEN_CTRL, VGA_OFFSET_HIGH);
  offset += inb(REG_SCREEN_DATA) << 8;
  outb(REG_SCREEN_CTRL, VGA_OFFSET_LOW);
  offset += inb(REG_SCREEN_DATA);
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
  print_at((char *)&buf, row, col);
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

void putchar(char c) {
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

void print_at(const char *message, unsigned char row, unsigned char col) {
  set_cursor(row, col);
  while (*message) {
    putchar(*message);
    message++;
  }
}
