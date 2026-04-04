CROSS_GCC := $(HOME)/opt/cross64/bin/x86_64-elf-gcc
CROSS_AS  := nasm

INCLUDES := -I terra-kernel/kernel
CFLAGS := -ffreestanding -Wall -nostdlib -mcmodel=kernel -O3

kernel_c_files := $(shell find terra-kernel/kernel -name '*.c')
kernel_asm_files := $(shell find terra-kernel/kernel -name '*.asm')
kernel_c_object_files := $(patsubst terra-kernel/kernel/%.c, build/kernel/%.o, $(kernel_c_files))
kernel_asm_object_files :=$(patsubst terra-kernel/kernel/%.asm, build/kernel/%.o, $(kernel_asm_files))

$(kernel_c_object_files): build/kernel/%.o : terra-kernel/kernel/%.c
	mkdir -vp $(dir $@)
	$(CROSS_GCC) -c $(CFLAGS) -g $< -o $@

$(kernel_asm_object_files): build/kernel/%.o : terra-kernel/kernel/%.asm
	mkdir -vp $(dir $@)
	nasm -f elf64 -g -F dwarf $< -o $@

build-objs: $(kernel_c_object_files) $(kernel_asm_object_files)

.PHONY: bootloader kernel image clean

bootloader:
	mkdir -vp build/bootloader
	$(CROSS_AS) -f elf64 terra-kernel/bootloader/bootloader.asm -o build/bootloader/boot.o
	ld -m elf_x86_64 -T linker/bootloader.ld build/bootloader/boot.o -o build/boot.bin

kernel: build-objs
	ld -m elf_x86_64 -T linker/kernel.ld -o build/kernel.bin $(kernel_c_object_files) $(kernel_asm_object_files)

image: clean bootloader kernel
	dd if=/dev/zero of=floppy.img bs=512 count=2880
	mkfs.fat -F 12 -R 2 floppy.img
	dd if=build/boot.bin of=floppy.img bs=1 seek=62 conv=notrunc
	sudo mkdir -p /mnt/floppy
	sudo mount -o loop -t vfat floppy.img /mnt/floppy
	sudo cp build/kernel.bin /mnt/floppy/
	sudo umount /mnt/floppy
	sudo rm -rf /mnt/floppy

debug: image
	mkdir -vp build/debug
	ld -m elf_x86_64 -T linker/kernel_debug.ld -o build/debug/kernel.elf $(kernel_c_object_files) $(kernel_asm_object_files)

clean:
	rm -rfv build