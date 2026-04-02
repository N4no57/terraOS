CROSS_GCC := $(HOME)/opt/cross/bin/i686-elf-gcc
CROSS_AS  := nasm

INCLUDES := -I src/intf
CFLAGS := $(INCLUDES) -ffreestanding -Wall -nostdlib

.PHONY: bootloader image clean

bootloader:
	mkdir -vp build/bootloader
	$(CROSS_AS) -f elf32 terra-kernel/bootloader/bootloader.asm -o build/bootloader/boot.o
	ld -m elf_i386 -T linker.ld build/bootloader/boot.o -o build/boot.bin

image: bootloader
	dd if=/dev/zero of=floppy.img bs=512 count=2880
	mkfs.fat -F 12 floppy.img
	dd if=build/boot.bin of=floppy.img bs=1 seek=62 conv=notrunc