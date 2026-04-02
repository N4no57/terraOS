CROSS_GCC := $(HOME)/opt/cross/bin/i686-elf-gcc
CROSS_AS  := nasm

CFLAGS := -ffreestanding -Wall -nostdlib

.PHONY: bootloader kernel image clean

bootloader:
	mkdir -vp build/bootloader
	$(CROSS_AS) -f elf32 terra-kernel/bootloader/bootloader.asm -o build/bootloader/boot.o
	ld -m elf_i386 -T bootloader.ld build/bootloader/boot.o -o build/boot.bin

kernel:
	mkdir -vp build/kernel
	$(CROSS_GCC) $(CFLAGS) -c terra-kernel/kernel/main.c -o build/kernel/main.o
	ld -m elf_i386 -T kernel.ld build/kernel/main.o -o build/kernel.bin

image: bootloader kernel
	dd if=/dev/zero of=floppy.img bs=512 count=2880
	mkfs.fat -F 12 floppy.img
	dd if=build/boot.bin of=floppy.img bs=1 seek=62 conv=notrunc
	sudo mkdir -p /mnt/floppy
	sudo mount -o loop -t vfat floppy.img /mnt/floppy
	sudo cp build/kernel.bin /mnt/floppy/
	sudo umount /mnt/floppy
	sudo rm -rf /mnt/floppy

clean:
	rm -rfv build