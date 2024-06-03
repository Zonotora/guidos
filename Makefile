build:
	nasm -f bin boot.asm -o boot.bin

debug: build
	qemu-system-i386 -drive file=boot.bin,format=raw -S -s

i386: build
	qemu-system-i386 -drive file=boot.bin,format=raw

x86_64: build
	qemu-system-x86_64 -drive file=boot.bin,format=raw