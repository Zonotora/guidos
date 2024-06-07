C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h)
BIN = $(wildcard *.bin)
OBJ = ${C_SOURCES:.c=.o cpu/interrupt.o}

CLANG = clang -m32 -target -i386-none-eabi
CFLAGS = -g

image.bin: boot/boot.bin kernel.bin
	cat $^ > image.bin

kernel.bin: boot/kernel_entry.o ${OBJ}
	i386-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

kernel.elf: boot/kernel_entry.o ${OBJ}
	i386-elf-ld -o $@ -Ttext 0x1000 $^

debug: image.bin kernel.elf
	qemu-system-i386 -fda image.bin -S -s &
	gdb -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

i386-hdd: image.bin
	qemu-system-i386 -drive file=image.bin,format=raw

i386-floppy: image.bin
	qemu-system-i386 -fda image.bin

%.o: %.c ${HEADERS}
	${CLANG} ${CFLAGS} -ffreestanding -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.bin: %.asm
	nasm $< -f bin -o $@

clean:
	rm -rf ${OBJ}
	rm -rf ${BIN}