# Extras



## Bootloader
We have defined
```assembly
KERNEL_OFFSET equ 0x1000 ;
```

and we use it to load the kernel at this address
```assembly
    mov bx, KERNEL_OFFSET       ; Read from disk and store in 0x1000
```
when loading from disk.

During linking we are telling the linker to place all executable code at memory address `0x1000`.
The linker will ensure that all code references are calculated based on this address.

```Makefile
i386-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary
```