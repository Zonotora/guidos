; --------------------------------------------------------------------------------
; Kernel loader
; --------------------------------------------------------------------------------

; The ORG instruction is used to provide a "hint" to the assembler and the linker.
; It allows you to specify the base address of the section of the file.
[org 0x7c00]

; This code should be stored in the first sector of a hard disk. The BIOS
; will load this code at the physical address 0x7c00-0x7e00 (512 bytes) and
; jump to the beginning of it (in real mode).

; --------------------------------------------------------------------------------
; Using 16-bit real mode
; --------------------------------------------------------------------------------
[bits 16]
; --------------------------------------------------------------------------------
; Start
; --------------------------------------------------------------------------------
loader_start:
    mov [BOOT_DRIVE], dl        ; Remember that the BIOS sets us the boot drive in 'dl' on boot
    mov bp, 0x9000
    mov sp, bp
    mov bx, MSG_REAL_MODE
    call print
    mov bx, KERNEL_OFFSET       ; Read from disk and store in 0x1000
    mov dh, 50
    mov dl, [BOOT_DRIVE]
    call disk_load
switch_to_pm:
    cli                         ; 1. disable interrupts
    lgdt [gdt_descriptor]       ; 2. load the GDT descriptor
    mov eax, cr0
    or eax, 0x1                 ; 3. set 32-bit mode bit in cr0
    mov cr0, eax
    jmp CODE_SEG:pm             ; 4. far jump by using a different segment

; --------------------------------------------------------------------------------
; print
; --------------------------------------------------------------------------------
; keep this in mind:
; while (string[i] != 0) { print string[i]; i++ }
; the comparison for string end (null byte)
print:
    pusha                       ; Push all registers to the stack
start:
    mov al, [bx]                ; 'bx' is the base address for the string
    cmp al, 0
    je done                     ; Jump to done if 'al' is zero
    mov ah, 0x0e                ; The part where we print with the BIOS help
    int 0x10                    ; 'al' already contains the char
    add bx, 1                   ; increment pointer and do next loop
    jmp start                   ; Jump to start
done:
    popa                        ; Restore all register from the stack
    ret                         ; Return to the caller

; --------------------------------------------------------------------------------
; Disk loader
; --------------------------------------------------------------------------------
; The BIOS passes in the drive that the loader was read from. The boot drive is
; stored in the 'dl' register. Floppy drives are numbered 0x00, 0x01, ... and hard
; drives are numbered 0x80, 0x81, ...

; load 'dh' sectors from drive 'dl' into ES:BX
; reading from disk requires setting specific values in all registers
; so we will overwrite our input parameters from 'dx'. Let's save it
; to the stack for later use.

; 0x01 is our boot sector, 0x02 is the first 'available' sector

; dl <- drive number. Our caller sets it as a parameter and gets it from BIOS
; (0 = floppy, 1 = floppy2, 0x80 = hdd, 0x81 = hdd2)

; [es:bx] <- pointer to buffer where the data will be stored
; caller sets it up for us, and it is actually the standard location for int 13h
disk_load:
    pusha
    push dx
    mov ah, 0x02                ; ah <- int 0x13 function. 0x02 = 'read'
    mov al, dh                  ; al <- number of sectors to read (0x01 .. 0x80)
    mov cl, 0x02                ; cl <- sector (0x01 .. 0x11)
    mov ch, 0x00                ; ch <- cylinder (0x0 .. 0x3FF, upper 2 bits in 'cl')
    mov dh, 0x00                ; dh <- head number (0x0 .. 0xF)
    int 0x13                    ; BIOS interrupt
    jc disk_error               ; if error (stored in the carry bit)
    pop dx
    cmp al, dh                  ; BIOS also sets 'al' to the ; of sectors read. Compare it.
    jne sectors_error
    popa
    ret
disk_error:
    mov bx, DISK_ERROR
    call print
    mov dh, ah                  ; ah = error code, dl = disk drive that dropped the error
    ; call print_hex ; check out the code at http://stanislavs.org/helppc/int_13-1.html
    jmp disk_loop
sectors_error:
    mov bx, SECTORS_ERROR
    call print
disk_loop:
    jmp $

; --------------------------------------------------------------------------------
; GDT
; --------------------------------------------------------------------------------
; don't remove the labels, they're needed to compute sizes and jumps

; the GDT starts with a null 8-byte
gdt_start:
    dd 0x0                      ; 4 byte
    dd 0x0                      ; 4 byte
; GDT for code segment. base = 0x00000000, length = 0xfffff
; for flags, refer to os-dev.pdf document, page 36
gdt_code:
    dw 0xffff                   ; segment length, bits 0-15
    dw 0x0                      ; segment base, bits 0-15
    db 0x0                      ; segment base, bits 16-23
    db 10011010b                ; flags (8 bits)
    db 11001111b                ; flags (4 bits) + segment length, bits 16-19
    db 0x0                      ; segment base, bits 24-31
; GDT for data segment. base and length identical to code segment
; some flags changed, again, refer to os-dev.pdf
gdt_data:
    dw 0xffff                   ; segment length, bits 0-15
    dw 0x0                      ; segment base, bits 0-15
    db 0x0                      ; segment base, bits 16-23
    db 10010010b                ; flags (8 bits)
    db 11001111b                ; flags (4 bits) + segment length, bits 16-19
    db 0x0                      ; segment base, bits 24-31
gdt_end:
; GDT descriptor
gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; size (16 bit), always one less of its true size
    dd gdt_start                ; address (32 bit)

; --------------------------------------------------------------------------------
; Using 32-bit protected mode
; --------------------------------------------------------------------------------
[bits 32]
; --------------------------------------------------------------------------------
; Print string in 32-bit protected mode
; --------------------------------------------------------------------------------
print_string_pm:
    pusha
    mov edx, VIDEO_MEMORY
print_string_pm_loop:
    mov al, [ebx]               ; [ebx] is the address of our character
    mov ah, WHITE_ON_BLACK
    cmp al, 0                   ; check if end of string
    je print_string_pm_done
    mov [edx], ax               ; store character + attribute in video memory
    add ebx, 1                  ; next char
    add edx, 2                  ; next video memory position
    jmp print_string_pm_loop
print_string_pm_done:
    popa
    ret
; --------------------------------------------------------------------------------
; Load kernel
; --------------------------------------------------------------------------------
; We are now using 32-bit instructions
pm:
    mov ax, DATA_SEG            ; 5. update the segment registers
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000            ; 6. update the stack right at the top of the free space
    mov esp, ebp

    mov ebx, MSG_PROT_MODE
    call print_string_pm
    call KERNEL_OFFSET          ; Give control to the kernel
    jmp $                       ; Stay here when the kernel returns control to us (if ever)


KERNEL_OFFSET equ 0x1000 ;
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
VIDEO_MEMORY equ 0xb8000
; The color byte for each character
WHITE_ON_BLACK equ 0x0f

; It is a good idea to store it in memory because 'dl' may get overwritten
BOOT_DRIVE db 0
; Strings
MSG_REAL_MODE db "Started in 16-bit Real Mode", 0
MSG_PROT_MODE db "Landed in 32-bit Protected Mode", 0
DISK_ERROR db "Disk read error", 0
SECTORS_ERROR db "Incorrect number of sectors read", 0

; padding
times 510 - ($-$$) db 0
dw 0xaa55