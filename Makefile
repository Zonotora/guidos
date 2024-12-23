C_SOURCES = $(wildcard kernel/*.c devices/*.c drivers/*.c arch/**/*.c libc/*.c fs/*.c fs/**/*.c)
HEADERS = $(wildcard kernel/*.h devices/*.h drivers/*.h arch/**/*.h libc/*.h fs/*.h fs/**/*.h)
BIN = $(wildcard *.bin)
OBJ = ${C_SOURCES:.c=.o arch/x86/interrupt.o}

CLANG = clang -m32 -target -i386-none-eabi
CFLAGS = -g -DDEBUG

.PHONY: hdd floppy grub

hda: image.bin
	qemu-system-i386 -hda image.bin -hdd ramdisk.img -serial stdio

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

image.bin: arch/x86/boot/stage1.bin arch/x86/boot/stage2.bin kernel.bin
	cat $^ > image.bin

arch/x86/boot/stage1.bin: arch/x86/boot/stage1.asm kernel.bin
	nasm $< -f bin -o $@
	python3 scripts/sectors.py kernel.bin $@

kernel.bin: arch/x86/boot/kernel_entry.o ${OBJ}
	i386-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

kernel.elf: arch/x86/boot/kernel_entry.o ${OBJ}
	i386-elf-ld -o $@ -Ttext 0x1000 $^

debug: image.bin kernel.elf
	qemu-system-i386 -hda image.bin -S -s &
	gdb -ex "target remote localhost:1234" \
			-ix commands.txt \
			-ex "symbol-file kernel.elf" \
			-ex "b *0x7c00" \
			-ex "b *0x500" \

			-ex "b kernel_main" \
	# -ex "tui layout src" \

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
	# dd if=/dev/zero of=ramdisk.img count=30000
	# use fdisk
	mkfs.fat --offset 2048 ramdisk.img

clean:
	find . -name '*.o' | xargs rm -f
	find . -name '*.img' | grep -v "ramdisk.img" | xargs rm -f
	find . -name '*.bin' | xargs rm -f
	find . -name '*.elf' | xargs rm -f
	find . -name '*.iso' | xargs rm -f
