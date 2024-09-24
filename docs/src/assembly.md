# Assembly

There are a couple of file extensions used for assembly files: [`.a`, `.s`, `.S`, `.asm`]

In x86, there are two separate versions of assembly syntax:
- AT&T (used by Unix compilers like `gcc`)
- Intel/NASM (with a couple dialects, like MASM vs. NASM itself).

Intel syntax is dominant in the DOS and Windows world, and AT&T syntax is dominant in the Unix world.

The `.S` file extension is appropriate for assembly files with GNU syntax using `as`, while `.asm` more often is associated with Intel syntax NASM/YASM, or MASM, source code.


||AT&T|Intel|
|-|-|-|
|Parameter order|`movl $5, %eax` <br> Source before the destination|`mov eax, 5` <br> Destination before source|