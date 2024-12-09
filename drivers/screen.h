#ifndef SCREEN_H
#define SCREEN_H

#define MAX_ROWS 25
#define MAX_COLS 80

/* Public kernel API */
void clear_screen();
void putchar(char c);
unsigned short get_cursor();
void set_cursor(unsigned char row, unsigned char col);
void print_at(const char *message, unsigned char row, unsigned char col);

#endif