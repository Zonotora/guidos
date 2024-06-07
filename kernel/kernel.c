#include "../drivers/ports.h"
#include "../drivers/screen.h"
#include "utils.h"
#include "../cpu/idt.h"
#include "../cpu/isr.h"

void main() {
    clear_screen();


    isr_install();
    /* Test the interrupts */
    __asm__ __volatile__("int $2");
    __asm__ __volatile__("int $3");
}
