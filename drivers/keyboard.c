#include "keyboard.h"
#include "arch/x86/isr.h"
#include "arch/x86/ports.h"
#include "kernel/console.h"
#include "kernel/kprintf.h"
#include "libc/string.h"

#define SC_MAX 57
const char *sc_name[] = {"ERROR",     "Esc",     "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-",      "=",
                         "Backspace", "Tab",     "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[",      "]",
                         "Enter",     "Lctrl",   "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'",      "`",
                         "LShift",    "\\",      "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "RShift", "Keypad *",
                         "LAlt",      "Spacebar"};
const char sc_ascii[] = {'?', '?', '1', '2', '3', '4', '5', '6', '7', '8', '9',  '0', '-', '=',  '?',
                         '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',  '[', ']', '?',  '?',
                         'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z',
                         'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', '?', '?',  '?', ' '};

static void keyboard_callback(registers_t *regs) {
  /* The PIC leaves us the scancode in port 0x60 */
  unsigned char scancode = inb(0x60);
  if (scancode > SC_MAX)
    return;
  prompt(scancode, sc_ascii[scancode]);
}

void init_keyboard() {
  kprintf("register keyboard callback\n");
  register_interrupt_handler(IRQ1, keyboard_callback);
}
