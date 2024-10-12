C_SOURCES = $(wildcard kernel/*.c devices/*.c drivers/*.c arch/**/*.c libc/*.c)
HEADERS = $(wildcard kernel/*.h devices/*.h drivers/*.h arch/**/*.h libc/*.h)
BIN = $(wildcard *.bin)
OBJ = ${C_SOURCES:.c=.o arch/x86/interrupt.o}

CLANG = clang -m32 -target -i386-none-eabi
CFLAGS = -g

.PHONY: hdd floppy grub

hda: image.bin
	qemu-system-i386 -hda image.bin -hdb ramdisk.img

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
	truncate --size=26112 image.bin

kernel.bin: arch/x86/boot/kernel_entry.o ${OBJ}
	i386-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

kernel.elf: arch/x86/boot/kernel_entry.o ${OBJ}
	i386-elf-ld -o $@ -Ttext 0x1000 $^

debug: image.bin kernel.elf
	qemu-system-i386 -fda image.bin -S -s &
	gdb -ex "target remote localhost:1234" \
			-ex "symbol-file kernel.elf" \
			-ex "tui layout src" \
			-ex "b kernel_main" \

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

ramdisk:
	mkfs.fat -F 16 -C ramdisk.img 10000

clean:
	find . -name '*.o' | xargs rm -f
	find . -name '*.img' | grep -v "ramdisk.img" | xargs rm -f
	find . -name '*.bin' | xargs rm -f
	find . -name '*.elf' | xargs rm -f
	find . -name '*.iso' | xargs rm -f
