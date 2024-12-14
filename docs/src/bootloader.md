# Bootloader

The bootloader is the program that brings your operating system to life.
So it needs to bring the kernel into memory somehow. Before transfering control to the kernel, it would be great to setup an environment that the kernel likes, e.g., [protected mode]().

The bootloader runs in [real mode](). Because of this, it has easy access to the BIOS.







Checklist:

1. Setup 16-bit segment registers and stack
2. Print startup message
3. Check presence of PCI, CPUID, MSRs
4. Enable and confirm enabled A20 line
5. Load GDTR
6. Inform BIOS of target processor mode
7. Get memory map from BIOS
8. Locate kernel in filesystem
9. Allocate memory to load kernel image
10. Load kernel image into buffer
11. Enable graphics mode
12. Check kernel image ELF headers
13. Enable long mode, if 64-bit
14. Allocate and map memory for kernel segments
15. Setup stack
16. Setup COM serial output port
17. Setup IDT
18. Disable PIC
19. Check presence of CPU features (NX, SMEP, x87, PCID, global pages, TCE, WP, MMX, SSE, SYSCALL), and enable them
20. Assign a PAT to write combining
21. Setup FS/GS base
22. Load IDTR
23. Enable APIC and setup using information in ACPI tables
24. Setup GDT and TSS



## BIOS vs UEFI

With a computer running legacy BIOS, the BIOS and the boot loader run in Real mode.
The 64-bit operating system kernel checks and switches the CPU into Long mode and then starts new kernel-mode threads running 64-bit code.

With a computer running UEFI, the UEFI firmware (except CSM and legacy Option ROM), the UEFI boot loader and the UEFI operating system kernel all run in Long mode.
