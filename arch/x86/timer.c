#include "timer.h"
#include "drivers/screen.h"
#include "isr.h"
#include "libc/string.h"
#include "ports.h"

/* Get the PIT value: hardware clock at 1193180 Hz */
#define PIT_HZ 1193180
#define TIMER_FREQ 100
#define PORT_CHANNEL0 0x40
#define PORT_CHANNEL1 0x41
#define PORT_CHANNEL2 0x42
/*
Bits         Usage
6 and 7      Select channel :
                0 0 = Channel 0
                0 1 = Channel 1
                1 0 = Channel 2
                1 1 = Read-back command (8254 only)
4 and 5      Access mode :
                0 0 = Latch count value command
                0 1 = Access mode: lobyte only
                1 0 = Access mode: hibyte only
                1 1 = Access mode: lobyte/hibyte
1 to 3       Operating mode :
                0 0 0 = Mode 0 (interrupt on terminal count)
                0 0 1 = Mode 1 (hardware re-triggerable one-shot)
                0 1 0 = Mode 2 (rate generator)
                0 1 1 = Mode 3 (square wave generator)
                1 0 0 = Mode 4 (software triggered strobe)
                1 0 1 = Mode 5 (hardware triggered strobe)
                1 1 0 = Mode 2 (rate generator, same as 010b)
                1 1 1 = Mode 3 (square wave generator, same as 011b)
0            BCD/Binary mode: 0 = 16-bit binary, 1 = four-digit BCD
*/
#define PORT_MODE 0x43

uint32_t tick = 0;

static void timer_callback(registers_t *regs) {
  if (tick > 0) {
    tick--;
  }
}

void configure_pit_channel(uint8_t channel, uint8_t mode, uint16_t frequency) {
  uint8_t low = frequency & 0xFF;
  uint8_t high = frequency >> 8;
  // 0x36 (0b0011110) -> Channel 0 | lobyte/hibyte | Mode 2
  uint8_t value = (channel << 6) | 0x30 | (mode << 1);
  port_byte_out(PORT_MODE, value);
  port_byte_out(channel, low);
  port_byte_out(channel, high);
}

void timer_init() {
  /* Install the function we just wrote */
  register_interrupt_handler(IRQ0, timer_callback);
  uint32_t count = PIT_HZ / TIMER_FREQ;
  configure_pit_channel(PORT_CHANNEL0, 2, count);
}

void timer_msleep(int32_t ms) {
  tick = ms * TIMER_FREQ / 1000;
  while (tick > 0) {
    asm volatile("hlt");
  }
}