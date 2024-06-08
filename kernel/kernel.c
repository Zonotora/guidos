#include "../cpu/isr.h"
#include "../cpu/timer.h"
#include "../drivers/keyboard.h"
#include "../drivers/screen.h"

void main() {
    clear_screen();
    isr_install();

    asm volatile("sti");
    init_keyboard();
}

