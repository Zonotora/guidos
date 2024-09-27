#include "arch/x86/isr.h"
#include "arch/x86/timer.h"
#include "drivers/keyboard.h"
#include "drivers/screen.h"

void kernel_main() {
  isr_install();
  asm volatile("sti");
  init_keyboard();

  clear_screen();
}