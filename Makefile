CROSS_GCC := $(HOME)/opt/cross64/bin/x86_64-elf-gcc
CROSS_AS  := nasm

CFLAGS := -ffreestanding -Wall -nostdlib -Os

.PHONY: bootloader kernel image clean

bootloader:
	mkdir -vp build/bootloader
	$(CROSS_AS) -f elf64 terra-kernel/bootloader/bootloader.asm -o build/bootloader/boot.o
	ld -m elf_x86_64 -T linker/bootloader.ld build/bootloader/boot.o -o build/boot.bin

kernel:
	mkdir -vp build/kernel
	$(CROSS_GCC) $(CFLAGS) -c terra-kernel/kernel/kernel.c -o build/kernel/kernel.o
	ld -m elf_x86_64 -T linker/kernel.ld build/kernel/kernel.o -o build/kernel.bin

image: bootloader kernel
	dd if=/dev/zero of=floppy.img bs=512 count=2880
	mkfs.fat -F 12 -R 2 floppy.img
	dd if=build/boot.bin of=floppy.img bs=1 seek=62 conv=notrunc
	sudo mkdir -p /mnt/floppy
	sudo mount -o loop -t vfat floppy.img /mnt/floppy
	sudo cp build/kernel.bin /mnt/floppy/
	sudo umount /mnt/floppy
	sudo rm -rf /mnt/floppy

clean:
	rm -rfv build