#include "../cpu/ports.h"
#include "../libc/mem.h"
#include "../libc/string.h"
#include "screen.h"

s8 *const VGA = (s8 *const) VIDEO_ADDRESS;

// Local functions
u16 get_offset(u8 row, u8 col) {
    u16 offset = (MAX_COLS * row) + col;
    return offset;
}

void set_cursor(u8 row, u8 col) {
    u16 offset = get_offset(row, col);
    port_byte_out(REG_SCREEN_CTRL, VGA_OFFSET_HIGH);
    port_byte_out(REG_SCREEN_DATA, offset >> 8);
    port_byte_out(REG_SCREEN_CTRL, VGA_OFFSET_LOW);
    port_byte_out(REG_SCREEN_DATA, offset & 0xFF);
}

u16 get_cursor() {
    u16 offset = 0;
    port_byte_out(REG_SCREEN_CTRL, VGA_OFFSET_HIGH);
    offset += port_byte_in(REG_SCREEN_DATA) << 8;
    port_byte_out(REG_SCREEN_CTRL, VGA_OFFSET_LOW);
    offset += port_byte_in(REG_SCREEN_DATA);
    return offset;
}

u16 scroll() {
    s32 n_bytes = 2 * (MAX_CHARACTERS - MAX_COLS);
    memory_copy(&VGA[2*MAX_COLS], &VGA[0], n_bytes);
    s8 buf[MAX_COLS + 1];
    for (s32 i = 0; i < MAX_COLS; i++) {
        buf[i] = ' ';
    }
    buf[MAX_COLS] = 0;
    s32 row = MAX_ROWS - 1;
    s32 col = 0;
    u16 offset = get_offset(row, col);
    kprint_at((s8 *)&buf, row, col);
    return offset;
}

u16 write_vga(u8 c, u8 modifier, u16 offset) {
    if (offset >= MAX_CHARACTERS) {
        offset = scroll();
    }
    VGA[offset*2] = c;
    VGA[offset*2+1] = modifier;
    return offset;
}

// Global functions
void clear_screen() {
    for (s32 i = 0; i < MAX_CHARACTERS; i++) {
        write_vga(0, 0, i);
    }
    set_cursor(0, 0);
}

void kprint_at(s8 *message, u8 row, u8 col) {
    s8 *s = message;
    while(*s != 0) {

        if (*s == '\n') {
            row += 1;
            col = 0;
            set_cursor(row, col);
            s += 1;
            continue;
        }

        u16 offset = get_offset(row, col);
        offset = write_vga(*s, WHITE_ON_BLACK, offset);
        s += 1;
        offset += 1;
        row = offset / MAX_COLS;
        col = offset % MAX_COLS;
        set_cursor(row, col);
    }
}

void kprint(char *message) {
    u16 offset = get_cursor();
    u8 row = offset / MAX_COLS;
    u8 col = offset % MAX_COLS;
    kprint_at(message, row, col);
}

void backspace() {
    s32 offset = get_cursor() - 1;
    if (offset < 0) {
        offset = 0;
    }
    u8 row = offset / MAX_COLS;
    u8 col = offset % MAX_COLS;
    kprint_at(" ", row, col);
    set_cursor(row, col);
}