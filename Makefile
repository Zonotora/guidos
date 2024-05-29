i386:
	nasm -f bin boot.asm -o boot.bin
	qemu-system-i386 -drive file=boot.bin,format=raw

x86_64:
	nasm -f bin boot.asm -o boot.bin
	qemu-system-x86_64 -drive file=boot.bin,format=raw