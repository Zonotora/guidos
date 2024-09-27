C_SOURCES = $(wildcard kernel/*.c drivers/*.c arch/**/*.c libc/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h arch/**/*.h libc/*.h)
BIN = $(wildcard *.bin)
OBJ = ${C_SOURCES:.c=.o arch/x86/interrupt.o}

CLANG = clang -m32 -target -i386-none-eabi
CFLAGS = -g

.PHONY: hdd floppy grub

hdd: image.bin
	qemu-system-i386 -drive file=image.bin,format=raw

floppy: image.bin
	qemu-system-i386 -fda image.bin


grub: image.iso
	qemu-system-i386 -cdrom image.iso

image.iso: arch/x86/boot/multiboot/loader.o ${OBJ}
	i386-elf-ld -T arch/x86/boot/multiboot/linker.ld -o image.bin $^
	grub-file --is-x86-multiboot image.bin
	mkdir -p isodir/boot/grub
	cp image.bin isodir/boot/image.bin
	grub-mkrescue -o image.iso isodir

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
			-ex "b *0x7c00" \

debug-iso: image.iso kernel.elf
	qemu-system-i386 -cdrom image.iso -S -s &
	gdb -ex "target remote localhost:1234" \
			-ex "symbol-file kernel.elf" \
			-ex "tui layout src" \
			-ex "b kernel_main" \

%.o: %.c ${HEADERS}
	${CLANG} ${CFLAGS} -I. -ffreestanding -Wall -Wextra -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.bin: %.asm
	nasm $< -f bin -o $@

clean:
	find . -name '*.o' | xargs rm -f
	find . -name '*.img' | grep -v "fat16_ramdisk.img" | xargs rm -f
	find . -name '*.bin' | xargs rm -f
	find . -name '*.elf' | xargs rm -f
	find . -name '*.iso' | xargs rm -f
