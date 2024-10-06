# Bootloader

## Execution mode

The modes in which x86 code can be executed in are:

- `Real mode (16-bit)`

    - Computers that **use** BIOS start up in this mode.
    - 20-bit segmented memory address space (meaning that only 1 MB of memory can be addressed— actually since 80286 a little more through HMA)
    - Direct software access to peripheral hardware
    - No concept of memory protection or multitasking at the hardware level.

- `Protected mode (16-bit and 32-bit)`

    - Expands addressable physical memory to 16 MB and addressable virtual memory to 1 GB.
    - Provides privilege levels and protected memory, which prevents programs from corrupting one another.
    - 16-bit protected mode (used during the end of the DOS era) used a complex, multi-segmented memory model.
    - 32-bit protected mode uses a simple, flat memory model.

- `Long mode (64-bit)`

    - Mostly an extension of the 32-bit (protected mode) instruction set, but unlike the 16–to–32-bit transition, many instructions were dropped in the 64-bit mode. Pioneered by AMD.

- `Virtual 8086 mode (16-bit)`

    - A special hybrid operating mode that allows real mode programs and operating systems to run while under the control of a protected mode supervisor operating system

- `System Management Mode (16-bit)`

    - Handles system-wide functions like power management, system hardware control, and proprietary OEM designed code.
    - It is intended for use only by system firmware.
    - All normal execution, including the operating system, is suspended.
    - An alternate software system (which usually resides in the computer's firmware, or a hardware-assisted debugger) is then executed with high privileges.

## BIOS vs UEFI

With a computer running legacy BIOS, the BIOS and the boot loader run in Real mode.
The 64-bit operating system kernel checks and switches the CPU into Long mode and then starts new kernel-mode threads running 64-bit code.

With a computer running UEFI, the UEFI firmware (except CSM and legacy Option ROM), the UEFI boot loader and the UEFI operating system kernel all run in Long mode.
