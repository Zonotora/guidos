; --------------------------------------------------------------------------------
; Kernel loader
; --------------------------------------------------------------------------------

; The ORG instruction is used to provide a "hint" to the assembler and the linker.
; It allows you to specify the base address of the section of the file.
[org 0x7c00]

; This code should be stored in the first sector of a hard disk. The BIOS
; will load this code at the physical address 0x7c00-0x7e00 (512 bytes) and
; jump to the beginning of it (in real mode).

; +---------+---------+-----------+-----------------+
; | start   | end     | size      | description     |
; +---------+---------+-----------+-----------------+
; | 0x00000 | 0x003ff |   1 KiB   | Real Mode IVT   |
; +---------+---------+-----------+-----------------+
; | 0x00400 | 0x004ff |   256 B   | BDA             |
; +---------+---------+-----------+-----------------+
; | 0x00500 | 0x07bff | 29.75 KiB | Free            |
; +---------+---------+-----------+-----------------+
; | 0x07c00 | 0x07dff |   512 B   | Boot sector     |
; +---------+---------+-----------+-----------------+
; | 0x07e00 | 0x7ffff | 480.5 KiB | Free            |
; +---------+---------+-----------+-----------------+
; | 0x80000 | 0x9ffff |  128 KiB  | EBDA            |
; +---------+---------+-----------+-----------------+
; | 0xa0000 | 0xbffff |  128 KiB  | Video display   |
; +---------+---------+-----------+-----------------+
; | 0xc0000 | 0xc7fff |   32 KiB  | Video BIOS      |
; +---------+---------+-----------+-----------------+
; | 0xc8000 | 0xeffff |  160 KiB  | BIOS expansions |
; +---------+---------+-----------+-----------------+
; | 0xf0000 | 0xfffff |   64 KiB  | BIOS            |
; +---------+---------+-----------+-----------------+

; The idea
;
; We want to place stage 2 at the first address of the first free block, i.e., 0x500
; Stage 1 has routines for:
;   1. Loading from disk
;   2. Printing string/hex
; That's it.
;
; Stage 1 can be described with the following points:
;   1. Setup the stack at the last address of the big free block (480.5 KiB),
;      i.e., address 0x7ffff.
;   2. Print status message
;   3. Load 1 * 512 bytes from the boot drive into memory at 0x500
;   4. Jump to address 0x500

STAGE2_OFFSET equ 0x0500
STACK_POINTER equ 0xffff

; --------------------------------------------------------------------------------
; Using 16-bit real mode
; --------------------------------------------------------------------------------
[bits 16]
; --------------------------------------------------------------------------------
; Start
; --------------------------------------------------------------------------------
stage1_start:
    mov [BOOT_DRIVE], dl        ; Remember that the BIOS sets us the boot drive in 'dl' on boot
    mov bp, STACK_POINTER       ; Set the base pointer to 0xffff
    mov sp, bp                  ; Set the stack pointer to 0xffff
    mov bx, MSG_REAL_MODE
    call print
    mov bx, STAGE2_OFFSET       ; Read from disk and store in 0x500
    mov dh, [N_STAGE2_SECTORS]  ; Load 1 sector
    mov dl, [BOOT_DRIVE]
    call disk_load
    mov cx, [N_STAGE2_SECTORS]
    mov ax, [N_KERNEL_SECTORS]
    jmp STAGE2_OFFSET


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
    call print_hex              ; check out the code at http://stanislavs.org/helppc/int_13-1.html
    jmp disk_loop
sectors_error:
    mov bx, SECTORS_ERROR
    call print
disk_loop:
    jmp $

; --------------------------------------------------------------------------------
; Print
; --------------------------------------------------------------------------------
; while (string[i] != 0) { print string[i]; i++ }
; The comparison for string end (null byte)
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
; Print hex
; --------------------------------------------------------------------------------
; Receiving the data in 'dx'
; For the examples we'll assume that we're called with dx=0x1234
print_hex:
    pusha
    mov cx, 0                   ; Our index variable

; Strategy: get the last char of 'dx', then convert to ASCII
; Numeric ASCII values: '0' (ASCII 0x30) to '9' (0x39), so just add 0x30 to byte N.
; For alphabetic characters A-F: 'A' (ASCII 0x41) to 'F' (0x46) we'll add 0x40
; Then, move the ASCII byte to the correct position on the resulting string
hex_loop:
    cmp cx, 4                   ; Loop 4 times
    je end

; 1. Convert last char of 'dx' to ascii
    mov ax, dx                  ; We will use 'ax' as our working register
    and ax, 0x000f              ; 0x1234 -> 0x0004 by masking first three to zeros
    add al, 0x30                ; Add 0x30 to N to convert it to ASCII "N"
    cmp al, 0x39                ; If > 9, add extra 8 to represent 'A' to 'F'
    jle step2
    add al, 7                   ; 'A' is ASCII 65 instead of 58, so 65-58=7

; 2. Get the correct position of the string to place our ASCII char
; bx <- base address + string length - index of char
step2:
    mov bx, HEX_OUT + 5         ; Base + length
    sub bx, cx                  ; Our index variable
    mov [bx], al                ; Copy the ASCII char on 'al' to the position pointed by 'bx'
    ror dx, 4                   ; 0x1234 -> 0x4123 -> 0x3412 -> 0x2341 -> 0x1234
    ; increment index and loop
    add cx, 1
    jmp hex_loop

; Prepare the parameter and call the function
; Remember that print receives parameters in 'bx'
end:
    mov bx, HEX_OUT
    call print
    popa
    ret
HEX_OUT:
    db '0x0000',0               ; Reserve memory for our new string

BOOT_DRIVE db 0
MSG_REAL_MODE db "Started in 16-bit Real Mode", 0
DISK_ERROR db "Disk read error", 0
SECTORS_ERROR db "Incorrect number of sectors read", 0
times 442 - ($-$$) db 0
N_STAGE2_SECTORS dw 0xdead
N_KERNEL_SECTORS dw 0xbeef

times 510 - ($-$$) db 0
dw 0xaa55