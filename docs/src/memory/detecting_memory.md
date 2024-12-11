# Detecting memory

We can request "low memory" which is the available RAM below 1MB (usually below 640KB) by two BIOS functions (`INT 0x12` and `INT 0x15`). Using `INT 0x12` we get the total number of KBs in the `ax` register.
```asm
clc        ; clear carry flag.
int 0x12   ; Request low memory size from BIOS.
jc .error  ; Jump to error routine if carry flag is set.
           ; ax = amount of continuous memory in KB
```