build:
	nasm -f bin boot.asm -o boot.bin
	clang -m32 -target -i386-none-eabi -ffreestanding -c kernel.c -o kernel.o
	nasm kernel_entry.asm -f elf -o kernel_entry.o
	i386-elf-ld -o kernel.bin -Ttext 0x1000 kernel_entry.o kernel.o --oformat binary
	cat boot.bin kernel.bin > image.bin

debug: build
	qemu-system-i386 -drive file=boot.bin,format=raw -S -s

i386: build
	qemu-system-i386 -drive file=boot.bin,format=raw