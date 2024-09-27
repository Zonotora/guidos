#include "screen.h"

#include "arch/x86/ports.h"
#include "libc/mem.h"
#include "libc/string.h"

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

void kprint_at(char *message, unsigned char row, unsigned char col) {
  char *s = message;
  while (*s != 0) {
    if (*s == '\n') {
      row += 1;
      col = 0;
      set_cursor(row, col);
      s += 1;
      continue;
    }

    unsigned short offset = get_offset(row, col);
    offset = write_vga(*s, WHITE_ON_BLACK, offset);
    s += 1;
    offset += 1;
    row = offset / MAX_COLS;
    col = offset % MAX_COLS;
    set_cursor(row, col);
  }
}

void kprint(char *message) {
  unsigned short offset = get_cursor();
  unsigned char row = offset / MAX_COLS;
  unsigned char col = offset % MAX_COLS;
  kprint_at(message, row, col);
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