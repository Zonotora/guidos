[org 0x7c00]
main:
    mov bp, 0x9000 ; set the stack
    mov sp, bp

    mov bx, MSG_REAL_MODE
    call print ; Written using the BIOS

    call switch_to_pm
    jmp $ ; this will never be executed

%include "print/boot_print.asm"
%include "32bit/gdt.asm"
%include "32bit/print.asm"
%include "32bit/switch.asm"


MSG_REAL_MODE db "Started in 16-bit real mode", 0
MSG_PROT_MODE db "Loaded 32-bit protected mode", 0


; Magic number
times 510 - ($-$$) db 0
dw 0xaa55