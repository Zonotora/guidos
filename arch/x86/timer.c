#include "timer.h"
#include "drivers/screen.h"
#include "isr.h"
#include "libc/string.h"
#include "ports.h"

unsigned int tick = 0;

static void timer_callback(registers_t *regs) { tick++; }

void init_timer(unsigned int freq) {
  /* Install the function we just wrote */
  register_interrupt_handler(IRQ0, timer_callback);

  /* Get the PIT value: hardware clock at 1193180 Hz */
  unsigned int divisor = 1193180 / freq;
  unsigned char low = (unsigned char)(divisor & 0xFF);
  unsigned char high = (unsigned char)((divisor >> 8) & 0xFF);
  /* Send the command */
  port_byte_out(0x43, 0x36); /* Command port */
  port_byte_out(0x40, low);
  port_byte_out(0x40, high);
}
