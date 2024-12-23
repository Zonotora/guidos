; --------------------------------------------------------------------------------
; Kernel loader
; --------------------------------------------------------------------------------
; This code should be stored in the first sector of a hard disk. The BIOS
; will load this code at the physical address 0x7c00-0x7e00 (512 bytes) and
; jump to the beginning of it (in real mode).
[org 0x500]

; --------------------------------------------------------------------------------
; Using 16-bit real mode
; --------------------------------------------------------------------------------
[bits 16]
; --------------------------------------------------------------------------------
; Start
; --------------------------------------------------------------------------------
loader_start:
    mov [BOOT_DRIVE], dl        ; Remember that the BIOS sets us the boot drive in 'dl' on boot
    mov [N_SECTORS], ax         ;
    call disk_load_kernel
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
disk_load_kernel:
    pusha
    mov ax, KERNEL_OFFSET       ; Read from disk and store in 0x1000
    mov es, ax
    mov dl, [BOOT_DRIVE]
    mov cx, [N_SECTORS]
    mov ebx, 2
    sub ebx, 127
disk_loop_start:
    mov ax, cx
    cmp ax, 127
    jg disk_ge
disk_leq:
    sub cx, cx
    jmp disk_main
disk_ge:
    mov ax, 127
    sub cx, 127
disk_main:
    add ebx, 127
    push bp
    mov bp, sp
    push 0                      ; LBA sector number [48:63]
    push 0                      ; LBA sector number [32:47]
    push ebx                    ; LBA sector number [00:31]
    push 0                      ; Buffer offset
    push es                     ; Buffer segement
    push ax                     ; Number of sectors
    push 0x1000                 ; Size of packet
    mov ah, 0x42                ; Extended read (LBA instead of CHS)
    mov si, sp                  ; DS:SI -> packet
    int 0x13
    jc disk_error               ; if error (stored in the carry bit)
    cmp ah, 0                  ; BIOS also sets 'al' to the ; of sectors read. Compare it.
    jne sectors_error
    mov sp, bp
    pop bp
    mov ax, cx
    cmp ax, 0
    jne disk_loop_start
    popa
    ret
disk_error:
    mov bx, DISK_ERROR
    call print
    mov dh, ah                  ; ah = error code, dl = disk drive that dropped the error
    call print_hex ; check out the code at http://stanislavs.org/helppc/int_13-1.html
    jmp disk_loop
sectors_error:
    mov bx, SECTORS_ERROR
    call print
disk_loop:
    jmp $

; --------------------------------------------------------------------------------
; Print hex
; --------------------------------------------------------------------------------
; receiving the data in 'dx'
; For the examples we'll assume that we're called with dx=0x1234
print_hex:
    pusha
    mov cx, 0 ; our index variable
; Strategy: get the last char of 'dx', then convert to ASCII
; Numeric ASCII values: '0' (ASCII 0x30) to '9' (0x39), so just add 0x30 to byte N.
; For alphabetic characters A-F: 'A' (ASCII 0x41) to 'F' (0x46) we'll add 0x40
; Then, move the ASCII byte to the correct position on the resulting string
hex_loop:
    cmp cx, 4 ; loop 4 times
    je end
    ; 1. convert last char of 'dx' to ascii
    mov ax, dx ; we will use 'ax' as our working register
    and ax, 0x000f ; 0x1234 -> 0x0004 by masking first three to zeros
    add al, 0x30 ; add 0x30 to N to convert it to ASCII "N"
    cmp al, 0x39 ; if > 9, add extra 8 to represent 'A' to 'F'
    jle step2
    add al, 7 ; 'A' is ASCII 65 instead of 58, so 65-58=7
step2:
    ; 2. get the correct position of the string to place our ASCII char
    ; bx <- base address + string length - index of char
    mov bx, HEX_OUT + 5 ; base + length
    sub bx, cx  ; our index variable
    mov [bx], al ; copy the ASCII char on 'al' to the position pointed by 'bx'
    ror dx, 4 ; 0x1234 -> 0x4123 -> 0x3412 -> 0x2341 -> 0x1234
    ; increment index and loop
    add cx, 1
    jmp hex_loop
end:
    ; prepare the parameter and call the function
    ; remember that print receives parameters in 'bx'
    mov bx, HEX_OUT
    call print
    popa
    ret
HEX_OUT:
    db '0x0000',0 ; reserve memory for our new string

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
N_SECTORS dw 0
; Strings
MSG_PROT_MODE db "Landed in 32-bit Protected Mode", 0
DISK_ERROR db "Disk read error", 0
SECTORS_ERROR db "Incorrect number of sectors read", 0

; padding
times 510 - ($-$$) db 0
dw 0xaa55