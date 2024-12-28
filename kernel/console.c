#include "../drivers/keyboard.h"
#include "../libc/mem.h"
#include "../libc/string.h"
#include "drivers/screen.h"
#include "kprintf.h"

#define BUFFER_LEN 256
char cmd[BUFFER_LEN];

void backspace() {
  int offset = get_cursor() - 2;
  if (offset < 0) {
    offset = 0;
  }
  unsigned char row = offset / MAX_COLS;
  unsigned char col = offset % MAX_COLS;
  set_cursor(row, col);
  kprintf(" ");
}

static void parse_cmd() {
  if (strcmp(cmd, "")) {
    // do nothing
  } else if (strcmp(cmd, "HELLO")) {
    kprintf("WORLD\n");
  } else {
    kprintf("Command not found\n");
  }
}

void prompt(unsigned char scancode, char ascii) {
  if (scancode == BACKSPACE) {
    backspace();
  } else if (scancode == ENTER) {
    kprintf("\n");
    parse_cmd();
    memory_set((unsigned char *)cmd, 0, BUFFER_LEN);
    kprintf("> ");
  } else {
    char buf[2] = {ascii, 0};
    kprintf(buf);
    strcat(cmd, buf, BUFFER_LEN);
  }
}