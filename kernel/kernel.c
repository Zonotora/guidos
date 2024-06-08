#include "../cpu/isr.h"
#include "../cpu/timer.h"
#include "../drivers/keyboard.h"
#include "../drivers/screen.h"

void main() {
    isr_install();
    asm volatile("sti");
    init_keyboard();

    clear_screen();
}

