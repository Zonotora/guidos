C_SOURCES = $(wildcard kernel/*.c drivers/*.c arch/**/*.c libc/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h arch/**/*.h libc/*.h)
BIN = $(wildcard *.bin)
OBJ = ${C_SOURCES:.c=.o arch/x86/interrupt.o}

CLANG = clang -m32 -target -i386-none-eabi
CFLAGS = -g

i386-hdd: image.bin
	qemu-system-i386 -drive file=image.bin,format=raw

i386-floppy: image.bin
	qemu-system-i386 -fda image.bin

image.bin: arch/x86/boot/loader.bin kernel.bin
	cat $^ > image.bin

kernel.bin: arch/x86/boot/kernel_entry.o ${OBJ}
	i386-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

kernel.elf: arch/x86/boot/kernel_entry.o ${OBJ}
	i386-elf-ld -o $@ -Ttext 0x1000 $^

debug: image.bin kernel.elf
	qemu-system-i386 -fda image.bin -S -s &
	gdb -ex "target remote localhost:1234" \
			-ex "symbol-file kernel.elf" \
			-ex "tui layout src" \

%.o: %.c ${HEADERS}
	${CLANG} ${CFLAGS} -I. -ffreestanding -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.bin: %.asm
	nasm $< -f bin -o $@

clean:
	find . -name '*.o' | xargs rm -f
