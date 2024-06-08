#include "../drivers/keyboard.h"
#include "../drivers/screen.h"
#include "../libc/mem.h"
#include "../libc/string.h"

#define BUFFER_LEN 256
char cmd[BUFFER_LEN];

static void parse_cmd() {
    if (strcmp(cmd, "")) {
        // do nothing
    } else if (strcmp(cmd, "HELLO")) {
        kprint("WORLD\n");
    } else {
        kprint("Command not found\n");
    }
}

void prompt(u8 scancode, char ascii) {
    if (scancode == BACKSPACE) {
        backspace();
    } else if (scancode == ENTER) {
        kprint("\n");
        parse_cmd();
        memory_set(cmd, 0, BUFFER_LEN);
        kprint("> ");
    } else {
        char buf[2] = { ascii, 0 };
        kprint(buf);
        strcat(cmd, buf, BUFFER_LEN);
    }

}